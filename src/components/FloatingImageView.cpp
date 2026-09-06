/**
 * @file FloatingImageView.cpp
 * @brief Complex rasterized texture calculations dynamically integrating hardware-accelerated filter matrices.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "FloatingImageView.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>

#include "../shader/stb_image.h"

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
#endif

FloatingImageView::FloatingImageView() {
    this->floatingX = 1000.0f;
    this->floatingY = 80.0f;
    this->layout_width = 1.0f;
    this->layout_height = 1.0f;
}

FloatingImageView::FloatingImageView(const std::string& path, float width, float height, float radius) {
    this->floatingX = 1000.0f;
    this->floatingY = 80.0f;
    this->layout_width = 1.0f;
    this->layout_height = 1.0f;
    this->targetWidth = width;
    this->targetHeight = height;
    this->cornerRadius = radius;
    loadHighResImage(path);
}

FloatingImageView::~FloatingImageView() {
    closeWidget();
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}

void FloatingImageView::loadHighResImage(const std::string& path) {
    m_imagePath = path;
    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

    if (!data) {
        std::cerr << "[FloatingImageView] Not Found: " << path << std::endl;
        return;
    }

    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float maxAniso = 0.0f;
    glGetFloatv(0x84FF, &maxAniso);
    if (maxAniso > 0.0f) {
        glTexParameterf(GL_TEXTURE_2D, 0x84FE, maxAniso);
    }

    if (width > 0 && height > 0 && targetHeight <= 0.0f) {
        float aspect = (float)width / (float)height;
        targetHeight = targetWidth / aspect;
    }

    stbi_image_free(data);
}

void FloatingImageView::setSize(float width, float height) {
    this->targetWidth = width;
    this->targetHeight = height;
}

void FloatingImageView::setCornerRadius(float radius) {
    this->cornerRadius = radius;
}

void FloatingImageView::setPosition(float px, float py) {
    this->floatingX = px;
    this->floatingY = py;
}

void FloatingImageView::applyOSWindowHiding(GLFWwindow* subWin, GLFWwindow* mainWin) {
    if (!subWin) return;

#ifdef _WIN32
    HWND hwnd = glfwGetWin32Window(subWin);
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
    Window x11Win = glfwGetX11Window(subWin);
    Window x11Main = mainWin ? glfwGetX11Window(mainWin) : 0;

    if (x11Display && x11Win) {
        if (x11Main) {
            XSetTransientForHint(x11Display, x11Win, x11Main);
        }
        Atom netWmType     = XInternAtom(x11Display, "_NET_WM_WINDOW_TYPE", False);
        Atom netWmTypeUtil = XInternAtom(x11Display, "_NET_WM_WINDOW_TYPE_UTILITY", False);
        XChangeProperty(x11Display, x11Win, netWmType, XA_ATOM, 32, PropModeReplace,
                        (unsigned char*)&netWmTypeUtil, 1);

        Atom netWmState       = XInternAtom(x11Display, "_NET_WM_STATE", False);
        Atom netWmSkipTaskbar = XInternAtom(x11Display, "_NET_WM_STATE_SKIP_TASKBAR", False);
        Atom netWmSkipPager   = XInternAtom(x11Display, "_NET_WM_STATE_SKIP_PAGER", False);
        Atom states[] = { netWmSkipTaskbar, netWmSkipPager };
        XChangeProperty(x11Display, x11Win, netWmState, XA_ATOM, 32, PropModeReplace,
                        (unsigned char*)states, 2);
        XFlush(x11Display);
    }
#endif
}

void FloatingImageView::detachToDesktop() {
    if (widgetWin) return;
    if (!m_mainWin) {
        m_mainWin = glfwGetCurrentContext();
    }
    if (!m_mainWin) return;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);

    widgetWin = glfwCreateWindow((int)targetWidth, (int)targetHeight, "Floating Media Preview", NULL, NULL);

    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);

    if (!widgetWin) return;

    applyOSWindowHiding(widgetWin, m_mainWin);

    int wx, wy;
    glfwGetWindowPos(m_mainWin, &wx, &wy);
    glfwSetWindowPos(widgetWin, wx + (int)floatingX, wy + (int)floatingY);

    glfwMakeContextCurrent(widgetWin);
    widgetRenderer = new MaterialShader();
    widgetRenderer->init();

    widgetEngine = new FloatingImageView();
    if (!m_imagePath.empty()) {
        widgetEngine->loadHighResImage(m_imagePath);
    }
    widgetEngine->floatingX = 0;
    widgetEngine->floatingY = 0;
    widgetEngine->targetWidth = targetWidth;
    widgetEngine->targetHeight = targetHeight;
    widgetEngine->cornerRadius = cornerRadius;

    glfwMakeContextCurrent(m_mainWin);
}

void FloatingImageView::dockToApp() {
    closeWidget();
}

void FloatingImageView::closeWidget() {
    if (!widgetWin) return;

    // Calculate relative coordinates and clamp within main window view bounds
    if (m_mainWin) {
        int wpx = 0, wpy = 0, mpx = 0, mpy = 0;
        glfwGetWindowPos(widgetWin, &wpx, &wpy);
        glfwGetWindowPos(m_mainWin, &mpx, &mpy);
        int mww = 0, mwh = 0;
        glfwGetWindowSize(m_mainWin, &mww, &mwh);

        float relX = (float)(wpx - mpx);
        float relY = (float)(wpy - mpy);

        if (mww > (int)targetWidth && mwh > (int)targetHeight) {
            this->floatingX = std::clamp(relX, 0.0f, (float)mww - targetWidth);
            this->floatingY = std::clamp(relY, 0.0f, (float)mwh - targetHeight);
        } else {
            this->floatingX = std::max(0.0f, relX);
            this->floatingY = std::max(0.0f, relY);
        }
    }

    // Release secondary window GPU resources
    glfwMakeContextCurrent(widgetWin);
    if (widgetEngine) {
        delete widgetEngine;
        widgetEngine = nullptr;
    }
    if (widgetRenderer) {
        delete widgetRenderer;
        widgetRenderer = nullptr;
    }

    // Restore context before destroying secondary window
    if (m_mainWin) {
        glfwMakeContextCurrent(m_mainWin);
    }

    GLFWwindow* winToDestroy = widgetWin;
    widgetWin = nullptr;
    glfwDestroyWindow(winToDestroy);

    if (m_mainWin) {
        glfwMakeContextCurrent(m_mainWin);
    }
}

void FloatingImageView::update(float mouseX, float mouseY, bool isLeftMouseDown) {
    isHovered = (mouseX >= floatingX && mouseX <= floatingX + targetWidth &&
                 mouseY >= floatingY && mouseY <= floatingY + targetHeight);

    if (isHovered && isLeftMouseDown && !isDragging) {
        isDragging = true;
        dragOffsetX = mouseX - floatingX;
        dragOffsetY = mouseY - floatingY;
    }

    if (isDragging) {
        floatingX = mouseX - dragOffsetX;
        floatingY = mouseY - dragOffsetY;
    }

    if (!isLeftMouseDown) {
        isDragging = false;
    }
}

void FloatingImageView::render(MaterialShader& shader) {
    if (textureID == 0) return;

    M3Color borderColor = { 0.0f, 0.0f, 0.0f, 0.15f };
    float borderWidth = 1.0f;
    float currentWidth = targetWidth;
    float currentHeight = targetHeight;
    float currentX = floatingX;
    float currentY = floatingY;

    if (isDragging) {
        borderWidth = 2.5f;
        borderColor = { 0.4f, 0.3f, 0.9f, 0.9f };

        float boost = 4.0f;
        currentX -= boost;
        currentY -= boost;
        currentWidth += boost * 2.0f;
        currentHeight += boost * 2.0f;
    }
    else if (isHovered) {
        borderWidth = 1.5f;
        borderColor = { 0.5f, 0.4f, 1.0f, 0.6f };
    }

    shader.drawM3UI(
        currentX, currentY, currentWidth, currentHeight,
        cornerRadius, cornerRadius, cornerRadius, cornerRadius,
        { 1.0f, 1.0f, 1.0f, 1.0f },
        0, 0, 0, 0, { 0,0,0,0 },
        0, 0, { 0,0,0,0 },
        borderColor, borderWidth,
        textureID, false,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f
    );

    if (isHovered || isDragging) {
        std::string badgeStatus = isDragging ? "Dragging..." : "Hovering";
        shader.drawText(badgeStatus, currentX + 16.0f, currentY + 24.0f, 18.0f, { 1.0f, 1.0f, 1.0f, 0.9f });
    }
}

void FloatingImageView::render(MaterialShader& renderer, MaterialTheme& theme) {
    GLFWwindow* curWin = glfwGetCurrentContext();
    if (!curWin) return;
    if (!m_mainWin) {
        m_mainWin = curWin;
    }

    // 1. Detached desktop window rendering pass
    if (widgetWin) {
        if (glfwWindowShouldClose(widgetWin)) {
            dockToApp();
            if (m_onLongPress) m_onLongPress(false);
            return;
        }

        double wx, wy;
        glfwGetCursorPos(widgetWin, &wx, &wy);
        bool wLeftPress  = glfwGetMouseButton(widgetWin, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool wRightPress = glfwGetMouseButton(widgetWin, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

        // Long press detection on secondary window
        if (wRightPress) {
            if (!isWidgetRightPressing) {
                isWidgetRightPressing = true;
                widgetRightPressStart = glfwGetTime();
            }
            else if (glfwGetTime() - widgetRightPressStart >= 0.8) {
                isWidgetRightPressing = false;
                if (m_autoDetachEnabled) {
                    dockToApp();
                }
                if (m_onLongPress) {
                    m_onLongPress(isDetached());
                }
                if (!widgetWin) {
                    return;
                }
            }
        } else {
            if (isWidgetRightPressing && (glfwGetTime() - widgetRightPressStart < 0.8)) {
                if (m_onRightClick) m_onRightClick();
            }
            isWidgetRightPressing = false;
            if (!widgetWin) return;
        }

        // Secondary window titlebar-less dragging
        if (wLeftPress) {
            if (!isDraggingOS) {
                isDraggingOS = true;
                dragOffsetX_OS = wx;
                dragOffsetY_OS = wy;
            }
            int wposx, wposy;
            glfwGetWindowPos(widgetWin, &wposx, &wposy);
            glfwSetWindowPos(widgetWin, wposx + (int)wx - (int)dragOffsetX_OS, wposy + (int)wy - (int)dragOffsetY_OS);
        } else {
            isDraggingOS = false;
        }

        if (!widgetWin || !widgetRenderer || !widgetEngine) return;

        renderer.end();
        glfwMakeContextCurrent(widgetWin);

        int fbW, fbH;
        glfwGetFramebufferSize(widgetWin, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);

        widgetRenderer->begin(fbW, fbH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        widgetEngine->floatingX = 0.0f;
        widgetEngine->floatingY = 0.0f;
        widgetEngine->targetWidth = (float)fbW;
        widgetEngine->targetHeight = (float)fbH;
        widgetEngine->update((float)wx, (float)wy, false);
        widgetEngine->render(*widgetRenderer);

        widgetRenderer->end();
        glfwSwapBuffers(widgetWin);

        if (glfwGetWindowAttrib(widgetWin, GLFW_VISIBLE) == GLFW_FALSE) {
            glfwShowWindow(widgetWin);
        }

        glfwMakeContextCurrent(m_mainWin);
        int mwW, mwH;
        glfwGetFramebufferSize(m_mainWin, &mwW, &mwH);
        renderer.begin(mwW, mwH);
        return;
    }

    if (glfwGetWindowAttrib(m_mainWin, GLFW_ICONIFIED)) {
        return;
    }

    // 2. In-app floating drag rendering pass
    double cx, cy;
    glfwGetCursorPos(m_mainWin, &cx, &cy);

    bool isRightPressMain = glfwGetMouseButton(m_mainWin, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    bool isLeftPressMain  = glfwGetMouseButton(m_mainWin, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool isHoverCard = (cx >= floatingX && cx <= floatingX + targetWidth &&
                        cy >= floatingY && cy <= floatingY + targetHeight);

    if (isRightPressMain && isHoverCard) {
        if (!isMainRightPressing) {
            isMainRightPressing = true;
            mainRightPressStart = glfwGetTime();
        }
        else if (glfwGetTime() - mainRightPressStart >= 0.8) {
            isMainRightPressing = false;
            if (m_autoDetachEnabled) {
                detachToDesktop();
            }
            if (m_onLongPress) {
                m_onLongPress(isDetached());
            }
            if (widgetWin) return;
        }
    } else {
        if (isMainRightPressing && isHoverCard && (glfwGetTime() - mainRightPressStart < 0.8)) {
            if (m_onRightClick) m_onRightClick();
        }
        isMainRightPressing = false;
        if (widgetWin) return;
    }

    if (isLeftPressMain && isHoverCard) {
        if (!isMainLeftPressing) {
            isMainLeftPressing = true;
            mainLeftPressStart = glfwGetTime();
            mainLeftDragged = false;
        }
        if (isDragging) {
            mainLeftDragged = true;
        }
    } else {
        if (isMainLeftPressing && isHoverCard && !mainLeftDragged && (glfwGetTime() - mainLeftPressStart < 0.35)) {
            if (m_onClick) m_onClick();
        }
        isMainLeftPressing = false;
        if (widgetWin) return;
    }

    update((float)cx, (float)cy, isLeftPressMain);

    int ww = 0, wh = 0;
    glfwGetWindowSize(m_mainWin, &ww, &wh);

    if (ww > 0 && wh > 0 && ww >= (int)targetWidth && wh >= (int)targetHeight) {
        float limitX = (float)ww - targetWidth;
        float limitY = (float)wh - targetHeight;

        if (floatingX < 0.0f) floatingX = 0.0f;
        if (floatingY < 0.0f) floatingY = 0.0f;
        if (floatingX > limitX) floatingX = limitX;
        if (floatingY > limitY) floatingY = limitY;
    }

    bool scissor = glIsEnabled(GL_SCISSOR_TEST);
    if (scissor) glDisable(GL_SCISSOR_TEST);

    render(renderer);

    if (scissor) glEnable(GL_SCISSOR_TEST);
}

bool FloatingImageView::handleMouseButton(int button, int action, float mx, float my) {
    if (widgetWin) return false;
    if (action == GLFW_RELEASE && !isDragging) return false;
    return (mx >= floatingX && mx <= floatingX + targetWidth &&
            my >= floatingY && my <= floatingY + targetHeight) || isDragging;
}

bool FloatingImageView::handleMouseMove(float mx, float my) {
    if (widgetWin) return false;
    bool isLeftPress = (glfwGetMouseButton(m_mainWin ? m_mainWin : glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    if (isLeftPress && !isDragging) return false;
    return (mx >= floatingX && mx <= floatingX + targetWidth &&
            my >= floatingY && my <= floatingY + targetHeight) || isDragging;
}

bool FloatingImageView::handleScroll(float mx, float my, float ox, float oy) {
    if (widgetWin) return false;
    return (mx >= floatingX && mx <= floatingX + targetWidth &&
            my >= floatingY && my <= floatingY + targetHeight);
}