/**
 * @file NotificationOverlay.cpp
 * @brief Implementation of NotificationOverlay stack kinetics, cubic easing, text truncation, and rendering passes.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "NotificationOverlay.hpp"
#include <cmath>
#include <algorithm>

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
static std::string truncateNotificationText(MaterialShader& renderer, const std::string& text, float fontSize, float maxWidth) {
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

NotificationOverlay::NotificationOverlay() {
    setLayoutParams(MATCH_PARENT, MATCH_PARENT);
}

void NotificationOverlay::push(const std::string& title, const std::string& message, float duration, NotificationStyle style) {
    Notification n;
    n.id = m_nextId++;
    n.title = title;
    n.message = message;
    n.duration = duration;
    n.style = style;

    if (title.empty()) n.height = std::round(dp(52.0f));
    else n.height = std::round(dp(76.0f));

    n.width = std::round(dp(320.0f));
    n.hasInitY = false;

    m_notifications.push_back(n);

    requestUIWakeup(duration + 1.2);
}

void NotificationOverlay::pushError(const std::string& title, const std::string& message, float duration) {
    push(title, message, duration, NotificationStyle::Error);
}

void NotificationOverlay::update(float dt) {
    View::update(dt);
    float fdt = (std::min)(dt, 0.033f);

    float paddingBottom = dp(24.0f);
    float spacing = dp(14.0f);
    float currentTargetY = height - paddingBottom;

    for (auto it = m_notifications.rbegin(); it != m_notifications.rend(); ++it) {
        auto& n = *it;
        currentTargetY -= n.height;
        float desiredTargetY = currentTargetY;
        currentTargetY -= spacing;

        if (!n.hasInitY) {
            n.hasInitY = true;
            n.currentY = desiredTargetY;
            n.startY = desiredTargetY;
            n.targetY = desiredTargetY;
            n.yProgress = 1.0f;
        } else if (std::abs(desiredTargetY - n.targetY) > 0.5f) {
            n.startY = n.currentY;
            n.targetY = desiredTargetY;
            n.yProgress = 0.0f;
        }

        if (n.yProgress < 1.0f) {
            n.yProgress += fdt / 0.28f;
            if (n.yProgress >= 1.0f) {
                n.yProgress = 1.0f;
                n.currentY = n.targetY;
            } else {
                float p = n.yProgress;
                float easeOut = 1.0f - std::pow(1.0f - p, 3.0f);
                n.currentY = n.startY + (n.targetY - n.startY) * easeOut;
            }
        } else {
            n.currentY = n.targetY;
        }

        float phaseSpeed = 3.6f * fdt;

        if (n.state == Notification::State::Entering) {
            n.animPhase += phaseSpeed;
            if (n.animPhase >= 1.0f) { 
                n.animPhase = 1.0f; 
                n.state = Notification::State::Holding; 
            }
        }
        else if (n.state == Notification::State::Holding) {
            if (!n.isHovered) { 
                n.timer += fdt; 
            }
            if (n.timer >= n.duration) { 
                n.state = Notification::State::Exiting; 
            }
        }
        else if (n.state == Notification::State::Exiting) {
            n.animPhase -= phaseSpeed * 1.4f;
        }
    }

    m_notifications.remove_if([](const Notification& n) {
        return n.state == Notification::State::Exiting && n.animPhase <= 0.001f;
    });

    if (!m_notifications.empty()) {
        requestUIWakeup(0.016);
    }
}

bool NotificationOverlay::handleMouseMove(float mx, float my) {
    bool hitAny = false;
    float paddingRight = dp(24.0f);

    for (auto& n : m_notifications) {
        float cX = width - paddingRight - n.width;
        float cY = n.currentY;

        if (mx >= cX && mx <= cX + n.width && my >= cY && my <= cY + n.height) {
            n.isHovered = true;
            hitAny = true;
            if (mx >= cX + n.width - dp(40.0f)) n.isCloseHovered = true;
            else n.isCloseHovered = false;
        }
        else {
            n.isHovered = false;
            n.isCloseHovered = false;
        }
    }
    return hitAny;
}

bool NotificationOverlay::handleMouseButton(int button, int action, float mx, float my) {
    if (action != 0) {
        bool hitAny = false;
        float paddingRight = dp(24.0f);
        for (auto& n : m_notifications) {
            float cX = width - paddingRight - n.width;
            float cY = n.currentY;
            if (mx >= cX && mx <= cX + n.width && my >= cY && my <= cY + n.height) {
                hitAny = true;
                if (n.isCloseHovered) {
                    n.state = Notification::State::Exiting;
                }
            }
        }
        return hitAny;
    }
    return false;
}

void NotificationOverlay::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (m_notifications.empty()) return;

    float paddingRight = std::round(dp(24.0f));
    float absoluteRightX = width - paddingRight;

    for (auto& n : m_notifications) {
        float phase = (std::clamp)(n.animPhase, 0.0f, 1.0f);
        float easeOutPhase = 1.0f - std::pow(1.0f - phase, 3.0f);

        float currentOffsetX = (1.0f - easeOutPhase) * dp(50.0f);
        float currentAlpha = easeOutPhase;

        float drawX = std::round(absoluteRightX - n.width + currentOffsetX);
        float drawY = std::round(n.currentY);

        M3Color bg;
        M3Color onBg;

        if (n.style == NotificationStyle::Error) {
            bg = { 0.86f, 0.22f, 0.27f, 1.0f };
            onBg = { 1.0f, 1.0f, 1.0f, 1.0f };
        }
        else {
            bg = theme.onSurface;
            onBg = theme.surface;
        }

        bg.a *= currentAlpha;
        onBg.a *= currentAlpha;

        // 1. Draw rounded background container
        float radius = dp(12.0f);
        renderer.drawM3UI(drawX, drawY, n.width, n.height, radius, radius, radius, radius, bg);

        // 2. Close button interactive container calculation
        float closeBoxSize = std::round(dp(26.0f));
        float closeX = std::round(drawX + n.width - closeBoxSize - dp(8.0f));
        float closeY = std::round(drawY + (n.height - closeBoxSize) * 0.5f);

        // 3. Draw typography with strict bounds and truncation
        float textStartX = std::round(drawX + dp(20.0f));
        float maxTextW = closeX - textStartX - dp(6.0f);

        if (!n.title.empty()) {
            std::string displayTitle = truncateNotificationText(renderer, n.title, dp(14.5f), maxTextW);
            renderer.drawText(displayTitle, textStartX, std::round(drawY + dp(16.0f)), dp(14.5f), onBg);
            
            M3Color secMsg = onBg;
            secMsg.a *= 0.75f;
            std::string displayMessage = truncateNotificationText(renderer, n.message, dp(13.0f), maxTextW);
            renderer.drawText(displayMessage, textStartX, std::round(drawY + dp(40.0f)), dp(13.0f), secMsg);
        }
        else {
            std::string displayMessage = truncateNotificationText(renderer, n.message, dp(14.0f), maxTextW);
            renderer.drawText(displayMessage, textStartX, std::round(drawY + (n.height - dp(14.0f)) * 0.5f), dp(14.0f), onBg);
        }

        // 4. Draw close button hover background
        if (n.isCloseHovered) {
            M3Color closeHoverBg = onBg;
            closeHoverBg.a = 0.12f * currentAlpha;
            float closeRadius = closeBoxSize * 0.5f;
            renderer.drawM3UI(closeX, closeY, closeBoxSize, closeBoxSize, closeRadius, closeRadius, closeRadius, closeRadius, closeHoverBg);
        }

        // 5. Draw close button icon (compact 8.5dp vector icon)
        M3Color closeBtn = onBg;
        closeBtn.a *= (n.isCloseHovered ? 1.0f : 0.55f);

        float iconDim = std::round(dp(8.5f));
        float iconDrawX = std::round(closeX + (closeBoxSize - iconDim) * 0.5f);
        float iconDrawY = std::round(closeY + (closeBoxSize - iconDim) * 0.5f);

        renderer.drawIcon(
            iconDrawX,
            iconDrawY,
            iconDim,
            IconM3(M3Icon::Close),
            closeBtn
        );
    }
}