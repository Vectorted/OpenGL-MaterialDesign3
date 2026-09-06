/**
 * @file StarrySkyNotification.cpp
 * @brief Implementation of cross-platform global and in-app Starry Sky notification queue renderer with text truncation and built-in icons.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "StarrySkyNotification.hpp"
#include "../shader/stb_image.h"
#include "../shader/stb_truetype.h"
#include <cmath>
#include <algorithm>
#include <iostream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

extern void requestUIWakeup(double seconds);

/**
 * @brief Utility function to truncate text and append ellipsis if it exceeds maximum pixel width.
 * 
 * @param renderer Reference to MaterialShader font rasterizer.
 * @param text Original source string.
 * @param fontSize Font size in pixels.
 * @param maxWidth Maximum permissible horizontal layout width in pixels.
 * @return std::string Truncated string with trailing ellipsis, or original text if fitting.
 */
static std::string truncateStarryText(MaterialShader& renderer, const std::string& text, float fontSize, float maxWidth) {
    if (text.empty() || maxWidth <= 0.0f) return "";

    float currentWidth = renderer.getTextWidth(text, fontSize);
    if (currentWidth <= maxWidth) {
        return text;
    }

    const std::string ellipsis = "...";
    float ellipsisWidth = renderer.getTextWidth(ellipsis, fontSize);
    if (ellipsisWidth >= maxWidth) {
        return ellipsis;
    }

    float availableWidth = maxWidth - ellipsisWidth;
    std::string result = "";

    for (size_t i = 0; i < text.size(); ) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        size_t charLen = 1;
        if (c >= 0xF0) charLen = 4;
        else if (c >= 0xE0) charLen = 3;
        else if (c >= 0xC0) charLen = 2;

        if (i + charLen > text.size()) break;

        std::string nextStr = result + text.substr(i, charLen);
        if (renderer.getTextWidth(nextStr, fontSize) > availableWidth) {
            break;
        }

        result = nextStr;
        i += charLen;
    }

    return result + ellipsis;
}

StarrySkyNotification::StarrySkyNotification(int maxCount, float duration)
    : m_maxCount(maxCount), m_defaultDuration(duration) {
    layout_width = MATCH_PARENT;
    layout_height = MATCH_PARENT;
}

StarrySkyNotification::~StarrySkyNotification() {
    closeOSOverlayWindow();
}

void StarrySkyNotification::setIcon(const std::string& iconPath) {
    m_customIconPath = iconPath;
}

void StarrySkyNotification::setFont(const std::string& fontPath) {
    m_customFontPath = fontPath;
    if (m_osRenderer) {
        applyWindowFont();
    }
}

void StarrySkyNotification::applyWindowIcon() {}

void StarrySkyNotification::applyWindowFont() {
    if (!m_osRenderer || m_customFontPath.empty()) return;
    m_osRenderer->loadFont(m_customFontPath);
}

void StarrySkyNotification::spawnOSOverlayWindow() {
    if (m_osWin) return;

    GLFWwindow* mainWin = glfwGetCurrentContext();
    if (!mainWin) return;

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    int workX = 0, workY = 0, workW = 1920, workH = 1080;
    if (primary) {
        glfwGetMonitorWorkarea(primary, &workX, &workY, &workW, &workH);
    }

    int osWinW = (int)dp(420.0f);
    int osWinH = workH - 24;
    int posX = workX + workW - osWinW;
    int posY = workY + 12;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);

    m_osWin = glfwCreateWindow(osWinW, osWinH, "StarrySkyDesktopOverlay", NULL, mainWin);

    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);

    if (!m_osWin) return;

#ifdef _WIN32
    HWND hwnd = glfwGetWin32Window(m_osWin);
    HWND mainHwnd = mainWin ? glfwGetWin32Window(mainWin) : NULL;
    if (hwnd) {
        if (mainHwnd) {
            SetWindowLongPtr(hwnd, GWLP_HWNDPARENT, (LONG_PTR)mainHwnd);
        }
        LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        exStyle &= ~WS_EX_APPWINDOW;
        exStyle |= WS_EX_TOOLWINDOW;
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
#elif defined(__linux__)
    Display* x11Display = glfwGetX11Display();
    Window x11Win = glfwGetX11Window(m_osWin);
    Window x11Main = mainWin ? glfwGetX11Window(mainWin) : 0;

    if (x11Display && x11Win) {
        if (x11Main) {
            XSetTransientForHint(x11Display, x11Win, x11Main);
        }
        XSetWindowAttributes attrs;
        attrs.override_redirect = True;
        XChangeWindowAttributes(x11Display, x11Win, CWOverrideRedirect, &attrs);

        Atom netWmWindowType = XInternAtom(x11Display, "_NET_WM_WINDOW_TYPE", False);
        Atom netWmTypeNotif  = XInternAtom(x11Display, "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
        Atom netWmTypeUtil   = XInternAtom(x11Display, "_NET_WM_WINDOW_TYPE_UTILITY", False);
        Atom types[] = { netWmTypeNotif, netWmTypeUtil };
        XChangeProperty(x11Display, x11Win, netWmWindowType, XA_ATOM, 32, PropModeReplace,
                        (unsigned char*)types, 2);

        Atom netWmState       = XInternAtom(x11Display, "_NET_WM_STATE", False);
        Atom netWmSkipTaskbar = XInternAtom(x11Display, "_NET_WM_STATE_SKIP_TASKBAR", False);
        Atom netWmSkipPager   = XInternAtom(x11Display, "_NET_WM_STATE_SKIP_PAGER", False);
        Atom netWmAbove       = XInternAtom(x11Display, "_NET_WM_STATE_ABOVE", False);

        Atom states[] = { netWmSkipTaskbar, netWmSkipPager, netWmAbove };
        XChangeProperty(x11Display, x11Win, netWmState, XA_ATOM, 32, PropModeReplace,
                        (unsigned char*)states, 3);

        XFlush(x11Display);
    }
#endif

    glfwSetWindowPos(m_osWin, posX, posY);

    glfwMakeContextCurrent(m_osWin);
    m_osRenderer = new MaterialShader();
    m_osRenderer->init();

    applyWindowFont();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(m_osWin);

    glfwMakeContextCurrent(mainWin);
}

void StarrySkyNotification::closeOSOverlayWindow() {
    if (m_osWin) {
        GLFWwindow* mainWin = glfwGetCurrentContext();
        glfwMakeContextCurrent(m_osWin);
        if (m_osRenderer) {
            delete m_osRenderer;
            m_osRenderer = nullptr;
        }
        glfwDestroyWindow(m_osWin);
        m_osWin = nullptr;
        if (mainWin) glfwMakeContextCurrent(mainWin);
    }
}

void StarrySkyNotification::show(const std::string& typeStr, const std::string& title, const std::string& desc) {
    StarryNotifType t = StarryNotifType::Green;
    if (typeStr == "red" || typeStr == "error" || typeStr == "alert") t = StarryNotifType::Red;
    else if (typeStr == "blue") t = StarryNotifType::Blue;
    else if (typeStr == "pink") t = StarryNotifType::Pink;
    else if (typeStr == "white") t = StarryNotifType::White;
    else if (typeStr == "primary" || typeStr == "theme") t = StarryNotifType::Primary;
    else if (typeStr == "secondary") t = StarryNotifType::Secondary;
    else if (typeStr == "tertiary") t = StarryNotifType::Tertiary;
    
    show(t, title, desc);
}

void StarrySkyNotification::show(StarryNotifType type, const std::string& title, const std::string& desc) {
    if (m_maxCount > 0 && m_items.size() >= (size_t)m_maxCount) {
        m_items[0].dismissing = true;
    }

    StarryNotifItem item;
    item.id = std::to_string(rand());
    item.type = type;
    item.title = title;
    item.desc = desc;
    item.duration = m_defaultDuration;
    item.timer = 0.0f;
    item.animSlideX = 1.0f;
    item.enterProgress = 0.0f;
    item.exitProgress = 0.0f;

    item.animSlot = 0.0f;
    item.startSlot = 0.0f;
    item.targetSlot = 0.0f;
    item.slotProgress = 1.0f;
    item.dismissing = false;

    m_items.push_back(item);

    if (!m_isWindowMode) {
        spawnOSOverlayWindow();
    }
}

void StarrySkyNotification::update(float dt) {
    View::update(dt);
    float dtSafe = (std::min)(dt, 0.033f);

    float notifH = dp(64.0f);
    float gap = dp(10.0f);
    float osMarginB = dp(24.0f);
    size_t maxVisible = 10;
    
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (primary) {
        int wx, wy, ww, wh;
        glfwGetMonitorWorkarea(primary, &wx, &wy, &ww, &wh);
        maxVisible = (std::max)((size_t)1, (size_t)((wh - osMarginB * 2.0f) / (notifH + gap)));
    }

    if (m_items.size() > maxVisible) {
        size_t overflowCount = m_items.size() - maxVisible;
        for (size_t k = 0; k < overflowCount; ++k) {
            m_items[k].dismissing = true;
        }
    }

    // 1. Vertical slot repositioning via cubic decelerate easing
    for (size_t i = 0; i < m_items.size(); ++i) {
        auto& item = m_items[i];
        size_t revIndex = m_items.size() - 1 - i;
        float desiredSlot = static_cast<float>(revIndex);

        if (std::abs(desiredSlot - item.targetSlot) > 0.0001f) {
            item.startSlot = item.animSlot;
            item.targetSlot = desiredSlot;
            item.slotProgress = 0.0f;
        }

        if (item.slotProgress < 1.0f) {
            item.slotProgress += dtSafe / 0.28f;
            if (item.slotProgress >= 1.0f) {
                item.slotProgress = 1.0f;
                item.animSlot = item.targetSlot;
            } else {
                float p = item.slotProgress;
                float easeOut = 1.0f - std::pow(1.0f - p, 3.0f);
                item.animSlot = item.startSlot + (item.targetSlot - item.startSlot) * easeOut;
            }
        } else {
            item.animSlot = item.targetSlot;
        }
    }

    // 2. Horizontal entrance and exit kinetics
    for (auto it = m_items.begin(); it != m_items.end(); ) {
        it->timer += dtSafe;
        if (it->timer >= it->duration) {
            it->dismissing = true;
        }

        if (!it->dismissing) {
            it->enterProgress += dtSafe / 0.28f;
            if (it->enterProgress >= 1.0f) {
                it->enterProgress = 1.0f;
                it->animSlideX = 0.0f;
            } else {
                float p = it->enterProgress;
                float easeOut = 1.0f - std::pow(1.0f - p, 3.0f);
                it->animSlideX = 1.0f - easeOut;
            }
        } else {
            it->exitProgress += dtSafe / 0.22f;
            float p = (std::clamp)(it->exitProgress, 0.0f, 1.0f);
            float easeIn = std::pow(p, 2.5f);
            it->animSlideX = easeIn * 1.25f;
        }

        it->outerAngle += 60.0f * dtSafe;
        it->greekAngle -= 45.0f * dtSafe;

        if (it->dismissing && it->exitProgress >= 1.0f) {
            it = m_items.erase(it);
        } else {
            ++it;
        }
    }

    if (!m_items.empty()) {
        requestUIWakeup(0.016);
    }
}

void StarrySkyNotification::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (m_items.empty()) {
        if (!m_isWindowMode && m_osWin && glfwGetWindowAttrib(m_osWin, GLFW_VISIBLE) == GLFW_TRUE) {
            glfwHideWindow(m_osWin);
        }
        return;
    }

    float notifW = std::round(dp(320.0f));
    float notifH = std::round(dp(64.0f));
    float gap = std::round(dp(10.0f));

    // Mode 1: Standalone Desktop OS Overlay Window
    if (!m_isWindowMode) {
        spawnOSOverlayWindow();
        if (!m_osWin || !m_osRenderer) return;

        GLFWwindow* mainWin = glfwGetCurrentContext();

        double wx, wy;
        glfwGetCursorPos(m_osWin, &wx, &wy);

        bool isHoveringCard = false;
        for (const auto& hb : m_hitBoxes) {
            if (wx >= hb.x && wx <= hb.x + hb.w && wy >= hb.y && wy <= hb.y + hb.h) {
                isHoveringCard = true;
                break;
            }
        }
        glfwSetWindowAttrib(m_osWin, GLFW_MOUSE_PASSTHROUGH, isHoveringCard ? GLFW_FALSE : GLFW_TRUE);

        bool isLeftPress = glfwGetMouseButton(m_osWin, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        if (isLeftPress && !m_wasOSClickPressed) {
            m_wasOSClickPressed = true;
            for (const auto& hb : m_hitBoxes) {
                if (wx >= hb.closeX && wx <= hb.closeX + hb.closeW && wy >= hb.closeY && wy <= hb.closeY + hb.closeH) {
                    if (hb.index < m_items.size()) {
                        m_items[hb.index].dismissing = true;
                    }
                }
            }
        } else if (!isLeftPress) {
            m_wasOSClickPressed = false;
        }

        renderer.end();
        glfwMakeContextCurrent(m_osWin);

        int fbW, fbH;
        glfwGetFramebufferSize(m_osWin, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);

        m_osRenderer->begin(fbW, fbH);
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        m_hitBoxes.clear();
        M3Color bgDark = { 28.0f / 255.0f, 28.0f / 255.0f, 33.0f / 255.0f, 0.97f };

        float osMarginR = std::round(dp(24.0f));
        float osMarginB = std::round(dp(24.0f));

        for (size_t i = 0; i < m_items.size(); ++i) {
            auto& item = m_items[i];

            float rawY = (float)fbH - osMarginB - notifH - item.animSlot * (notifH + gap);
            float rawX = (float)fbW - osMarginR - notifW + item.animSlideX * (notifW + dp(40.0f));
            
            float targetY = std::round(rawY);
            float curX = std::round(rawX);

            if (targetY < -notifH || targetY + notifH > (float)fbH + dp(10.0f)) continue;

            M3Color titleColor, arrayColor, progressColor;
            M3Color descColor = { 138.0f / 255.0f, 143.0f / 255.0f, 171.0f / 255.0f, 1.0f };

            switch (item.type) {
                case StarryNotifType::Red:
                case StarryNotifType::Error:
                    titleColor = { 255.0f / 255.0f, 82.0f / 255.0f, 82.0f / 255.0f, 1.0f };
                    arrayColor = { 255.0f / 255.0f, 128.0f / 255.0f, 171.0f / 255.0f, 1.0f };
                    progressColor = { 255.0f / 255.0f, 64.0f / 255.0f, 129.0f / 255.0f, 1.0f };
                    break;
                case StarryNotifType::Green:
                    titleColor = { 0.0f, 230.0f / 255.0f, 118.0f / 255.0f, 1.0f };
                    arrayColor = { 130.0f / 255.0f, 177.0f / 255.0f, 255.0f / 255.0f, 1.0f };
                    progressColor = titleColor;
                    break;
                case StarryNotifType::Blue:
                    titleColor = { 41.0f / 255.0f, 182.0f / 255.0f, 246.0f / 255.0f, 1.0f };
                    arrayColor = { 128.0f / 255.0f, 216.0f / 255.0f, 255.0f / 255.0f, 1.0f };
                    progressColor = titleColor;
                    break;
                default:
                    titleColor = theme.primary;
                    arrayColor = theme.primary;
                    progressColor = theme.primary;
                    break;
            }

            // 1. Background card
            m_osRenderer->drawM3UI(curX, targetY, notifW, notifH, dp(8.0f), dp(8.0f), dp(8.0f), dp(8.0f), bgDark);

            // 2. Progress bar
            float progress = (std::max)(0.0f, (std::min)(1.0f, item.timer / item.duration));
            float padX = std::round(dp(8.0f));
            float maxBarW = notifW - padX * 2.0f;
            float barW = std::round(maxBarW * progress);
            float barH = std::round(dp(2.5f));
            float barX = curX + padX;
            float barY = targetY + notifH - barH - std::round(dp(3.0f));

            if (barW > 1.0f) {
                m_osRenderer->drawM3UI(barX, barY, barW, barH, dp(1.5f), dp(1.5f), dp(1.5f), dp(1.5f), progressColor);
            }

            // 3. Magic array formation
            float arrayCx = std::round(curX + dp(30.0f));
            float arrayCy = std::round(targetY + notifH * 0.5f);
            StarrySkyFormation::drawMagicArray(*m_osRenderer, arrayCx, arrayCy, dp(16.0f), FormationLevel::Active, item.outerAngle, item.greekAngle, 0.0f, arrayColor);

            // 4. Close button geometry calculation
            float closeBoxSize = std::round(dp(26.0f));
            float closeX = std::round(curX + notifW - closeBoxSize - dp(8.0f));
            float closeY = std::round(targetY + (notifH - closeBoxSize) * 0.5f);

            // 5. Text labels with strict bounds and ellipsis truncation
            float txtStartX = std::round(curX + dp(54.0f));
            float maxTxtW = closeX - txtStartX - dp(6.0f); // Reserve clearance from close button

            std::string displayTitle = truncateStarryText(*m_osRenderer, item.title, dp(12.0f), maxTxtW);
            m_osRenderer->drawText(displayTitle, txtStartX, std::round(targetY + dp(14.0f)), dp(12.0f), titleColor);

            if (!item.desc.empty()) {
                std::string displayDesc = truncateStarryText(*m_osRenderer, item.desc, dp(11.0f), maxTxtW);
                m_osRenderer->drawText(displayDesc, txtStartX, std::round(targetY + dp(34.0f)), dp(11.0f), descColor);
            }

            // 6. Close button drawing via built-in M3 icon pipeline (calibrated to 8.5dp for compact refinement)
            bool isCloseHovered = (wx >= closeX && wx <= closeX + closeBoxSize &&
                                   wy >= closeY && wy <= closeY + closeBoxSize);

            if (isCloseHovered) {
                M3Color closeHoverBg = { 1.0f, 1.0f, 1.0f, 0.15f };
                float closeRadius = closeBoxSize * 0.5f;
                m_osRenderer->drawM3UI(closeX, closeY, closeBoxSize, closeBoxSize, closeRadius, closeRadius, closeRadius, closeRadius, closeHoverBg);
            }

            M3Color closeIconColor = isCloseHovered ? M3Color{ 1.0f, 1.0f, 1.0f, 1.0f } : M3Color{ 1.0f, 1.0f, 1.0f, 0.55f };
            float iconDim = std::round(dp(8.5f));
            float iconDrawX = std::round(closeX + (closeBoxSize - iconDim) * 0.5f);
            float iconDrawY = std::round(closeY + (closeBoxSize - iconDim) * 0.5f);

            m_osRenderer->drawIcon(iconDrawX, iconDrawY, iconDim, IconM3(M3Icon::Close), closeIconColor);

            HitBox hb;
            hb.x = curX; hb.y = targetY; hb.w = notifW; hb.h = notifH;
            hb.closeX = closeX; hb.closeY = closeY; hb.closeW = closeBoxSize; hb.closeH = closeBoxSize;
            hb.index = i;
            m_hitBoxes.push_back(hb);
        }

        m_osRenderer->end();
        glfwSwapBuffers(m_osWin);

        if (glfwGetWindowAttrib(m_osWin, GLFW_VISIBLE) == GLFW_FALSE) {
            glfwShowWindow(m_osWin);
        }

        glfwMakeContextCurrent(mainWin);
        int mwW, mwH;
        glfwGetFramebufferSize(mainWin, &mwW, &mwH);
        renderer.begin(mwW, mwH);
        return;
    }

    // Mode 2: In-App Window Overlay Mode
    m_hitBoxes.clear();

    float windowMarginR = std::round(dp(24.0f));
    float windowMarginB = std::round(dp(24.0f));

    float baseOriginX = x + width;
    float baseOriginY = y + height;

    M3Color bgDark = { 28.0f / 255.0f, 28.0f / 255.0f, 33.0f / 255.0f, 0.97f };

    for (size_t i = 0; i < m_items.size(); ++i) {
        auto& item = m_items[i];

        float rawY = baseOriginY - windowMarginB - notifH - item.animSlot * (notifH + gap);
        float rawX = baseOriginX - windowMarginR - notifW + item.animSlideX * (notifW + dp(40.0f));

        float targetY = std::round(rawY);
        float curX = std::round(rawX);

        M3Color titleColor, arrayColor, progressColor;
        M3Color descColor = { 138.0f / 255.0f, 143.0f / 255.0f, 171.0f / 255.0f, 1.0f };

        switch (item.type) {
            case StarryNotifType::Red:
            case StarryNotifType::Error:
                titleColor = { 255.0f / 255.0f, 82.0f / 255.0f, 82.0f / 255.0f, 1.0f };
                arrayColor = { 255.0f / 255.0f, 128.0f / 255.0f, 171.0f / 255.0f, 1.0f };
                progressColor = { 255.0f / 255.0f, 64.0f / 255.0f, 129.0f / 255.0f, 1.0f };
                break;
            case StarryNotifType::Green:
                titleColor = { 0.0f, 230.0f / 255.0f, 118.0f / 255.0f, 1.0f };
                arrayColor = { 130.0f / 255.0f, 177.0f / 255.0f, 255.0f / 255.0f, 1.0f };
                progressColor = titleColor;
                break;
            case StarryNotifType::Blue:
                titleColor = { 41.0f / 255.0f, 182.0f / 255.0f, 246.0f / 255.0f, 1.0f };
                arrayColor = { 128.0f / 255.0f, 216.0f / 255.0f, 255.0f / 255.0f, 1.0f };
                progressColor = titleColor;
                break;
            default:
                titleColor = theme.primary;
                arrayColor = theme.primary;
                progressColor = theme.primary;
                break;
        }

        // 1. Background card
        renderer.drawM3UI(curX, targetY, notifW, notifH, dp(8.0f), dp(8.0f), dp(8.0f), dp(8.0f), bgDark);

        // 2. Progress bar
        float progress = (std::max)(0.0f, (std::min)(1.0f, item.timer / item.duration));
        float padX = std::round(dp(8.0f));
        float maxBarW = notifW - padX * 2.0f;
        float barW = std::round(maxBarW * progress);
        float barH = std::round(dp(2.5f));
        float barX = curX + padX;
        float barY = targetY + notifH - barH - std::round(dp(3.0f));
        
        if (barW > 1.0f) {
            renderer.drawM3UI(barX, barY, barW, barH, dp(1.5f), dp(1.5f), dp(1.5f), dp(1.5f), progressColor);
        }

        // 3. Magic array formation
        float arrayCx = std::round(curX + dp(30.0f));
        float arrayCy = std::round(targetY + notifH * 0.5f);
        StarrySkyFormation::drawMagicArray(renderer, arrayCx, arrayCy, dp(16.0f), FormationLevel::Active, item.outerAngle, item.greekAngle, 0.0f, arrayColor);

        // 4. Close button geometry calculation
        float closeBoxSize = std::round(dp(26.0f));
        float closeX = std::round(curX + notifW - closeBoxSize - dp(8.0f));
        float closeY = std::round(targetY + (notifH - closeBoxSize) * 0.5f);

        // 5. Text labels with strict bounds and ellipsis truncation
        float txtStartX = std::round(curX + dp(54.0f));
        float maxTxtW = closeX - txtStartX - dp(6.0f); // Reserve clearance from close button

        std::string displayTitle = truncateStarryText(renderer, item.title, dp(12.0f), maxTxtW);
        renderer.drawText(displayTitle, txtStartX, std::round(targetY + dp(14.0f)), dp(12.0f), titleColor);

        if (!item.desc.empty()) {
            std::string displayDesc = truncateStarryText(renderer, item.desc, dp(11.0f), maxTxtW);
            renderer.drawText(displayDesc, txtStartX, std::round(targetY + dp(34.0f)), dp(11.0f), descColor);
        }

        // 6. Close button drawing via built-in M3 icon pipeline (calibrated to 8.5dp for compact refinement)
        bool isCloseHovered = (m_mouseX >= closeX && m_mouseX <= closeX + closeBoxSize &&
                               m_mouseY >= closeY && m_mouseY <= closeY + closeBoxSize);

        if (isCloseHovered) {
            M3Color closeHoverBg = { 1.0f, 1.0f, 1.0f, 0.15f };
            float closeRadius = closeBoxSize * 0.5f;
            renderer.drawM3UI(closeX, closeY, closeBoxSize, closeBoxSize, closeRadius, closeRadius, closeRadius, closeRadius, closeHoverBg);
        }

        M3Color closeIconColor = isCloseHovered ? M3Color{ 1.0f, 1.0f, 1.0f, 1.0f } : M3Color{ 1.0f, 1.0f, 1.0f, 0.55f };
        float iconDim = std::round(dp(8.5f));
        float iconDrawX = std::round(closeX + (closeBoxSize - iconDim) * 0.5f);
        float iconDrawY = std::round(closeY + (closeBoxSize - iconDim) * 0.5f);

        renderer.drawIcon(iconDrawX, iconDrawY, iconDim, IconM3(M3Icon::Close), closeIconColor);

        HitBox hb;
        hb.x = curX; hb.y = targetY; hb.w = notifW; hb.h = notifH;
        hb.closeX = closeX; hb.closeY = closeY; hb.closeW = closeBoxSize; hb.closeH = closeBoxSize;
        hb.index = i;
        m_hitBoxes.push_back(hb);
    }
}

bool StarrySkyNotification::handleMouseMove(float mx, float my) {
    m_mouseX = mx;
    m_mouseY = my;

    if (!m_isWindowMode) return false;
    for (const auto& hb : m_hitBoxes) {
        if (mx >= hb.x && mx <= hb.x + hb.w && my >= hb.y && my <= hb.y + hb.h) {
            requestUIWakeup(0.016);
            return true;
        }
    }
    return false;
}

bool StarrySkyNotification::handleMouseButton(int button, int action, float mx, float my) {
    if (!m_isWindowMode || m_items.empty()) return false;

    for (const auto& hb : m_hitBoxes) {
        if (mx >= hb.x && mx <= hb.x + hb.w && my >= hb.y && my <= hb.y + hb.h) {
            if (button == 0 && action == 1) {
                if (mx >= hb.closeX && mx <= hb.closeX + hb.closeW && my >= hb.closeY && my <= hb.closeY + hb.closeH) {
                    if (hb.index < m_items.size()) {
                        m_items[hb.index].dismissing = true;
                    }
                }
            }
            return true;
        }
    }
    return false;
}