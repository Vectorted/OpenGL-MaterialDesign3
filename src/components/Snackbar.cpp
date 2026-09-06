/**
 * @file Snackbar.cpp
 * @brief Implementation of Snackbar message queue processing, responsive text truncation, and rendering.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include <glad/glad.h>  
#include "Snackbar.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h> 

extern void requestUIWakeup(double seconds);

Snackbar::Snackbar() {
    layout_width = MATCH_PARENT;
    layout_height = MATCH_PARENT;
}

void Snackbar::show(const std::string& text, const std::string& actionText, std::function<void()> onAction, float duration) {
    m_msgQueue.push({ text, actionText, onAction, duration });
    
    if (m_sstate == SnackState::Hidden) {
        processNext();
    }
    else if (m_sstate == SnackState::Visible) {
        dismiss();
    }
}

void Snackbar::dismiss() {
    if (m_sstate == SnackState::Visible || m_sstate == SnackState::AnimatingIn) {
        m_sstate = SnackState::AnimatingOut;
        m_stateTime = 0.0f;
        requestUIWakeup(0.5);
    }
}

void Snackbar::processNext() {
    if (!m_msgQueue.empty()) {
        m_currentMsg = m_msgQueue.front();
        m_msgQueue.pop();
        m_sstate = SnackState::AnimatingIn;
        m_progress = 0.0f;
        m_stateTime = 0.0f;
        m_lastLayoutW = -1.0f;
        requestUIWakeup(m_currentMsg.duration + 1.0);
    }
}

void Snackbar::doLayout(float parentX, float parentY, float parentW, float parentH) {
    if (parentW <= 10.0f || parentH <= 10.0f) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        parentW = static_cast<float>(viewport[2]);
        parentH = static_cast<float>(viewport[3]);
        parentX = 0.0f;
        parentY = 0.0f;
    }
    x = parentX;
    y = parentY;
    width = parentW;
    height = parentH;
}

void Snackbar::update(float dt) {
    View::update(dt);

    if (m_sstate == SnackState::Hidden) return;

    m_stateTime += dt;
    const float animSpeed = 5.0f;

    if (m_sstate == SnackState::AnimatingIn) {
        m_progress += dt * animSpeed;
        if (m_progress >= 1.0f) {
            m_progress = 1.0f;
            m_sstate = SnackState::Visible;
            m_stateTime = 0.0f;
        }
    }
    else if (m_sstate == SnackState::Visible) {
        if (m_stateTime >= m_currentMsg.duration) {
            m_sstate = SnackState::AnimatingOut;
            m_stateTime = 0.0f;
        }
    }
    else if (m_sstate == SnackState::AnimatingOut) {
        m_progress -= dt * animSpeed;
        if (m_progress <= 0.0f) {
            m_progress = 0.0f;
            m_sstate = SnackState::Hidden;
            processNext();
        }
    }
}

void Snackbar::formatSingleLineText(MaterialShader& renderer, float maxAvailableWidth, float fontSize) {
    std::string raw = m_currentMsg.text;

    /** Flatten line breaks and tab delimiters to uniform single space */
    for (char& c : raw) {
        if (c == '\r' || c == '\n' || c == '\t') {
            c = ' ';
        }
    }

    /** Collapse redundant continuous whitespace sequences */
    std::string singleLine;
    singleLine.reserve(raw.size());
    bool lastWasSpace = false;
    for (char c : raw) {
        if (c == ' ') {
            if (!lastWasSpace) {
                singleLine += ' ';
                lastWasSpace = true;
            }
        } else {
            singleLine += c;
            lastWasSpace = false;
        }
    }
    raw = std::move(singleLine);

    /** Parse UTF-8 multi-byte boundary offsets */
    std::vector<size_t> charIndices;
    charIndices.reserve(std::min(raw.size(), m_maxTextLength + 1));
    
    for (size_t i = 0; i < raw.size(); ) {
        charIndices.push_back(i);
        if (charIndices.size() > m_maxTextLength) break;

        unsigned char c = static_cast<unsigned char>(raw[i]);
        if (c < 0x80) i += 1;
        else if ((c & 0xE0) == 0xC0) i += 2;
        else if ((c & 0xF0) == 0xE0) i += 3;
        else if ((c & 0xF8) == 0xF0) i += 4;
        else i += 1;
    }
    charIndices.push_back(raw.size());

    if (charIndices.size() > m_maxTextLength + 1) {
        raw = raw.substr(0, charIndices[m_maxTextLength]);
    }

    /** Evaluate full single-line width */
    float currentWidth = renderer.getTextWidth(raw, fontSize);
    if (currentWidth <= maxAvailableWidth || maxAvailableWidth <= 0.0f) {
        m_cachedDisplayMsg = raw;
        m_cachedMsgWidth = currentWidth;
        return;
    }

    /** Perform binary search for optimal ellipsis cutoff index */
    int low = 1;
    int high = static_cast<int>(charIndices.size()) - 1;
    int bestIndex = 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        size_t bytePos = charIndices[std::min(static_cast<size_t>(mid), charIndices.size() - 1)];
        std::string candidate = raw.substr(0, bytePos) + "...";
        float testW = renderer.getTextWidth(candidate, fontSize);

        if (testW <= maxAvailableWidth) {
            bestIndex = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    size_t finalBytePos = charIndices[std::min(static_cast<size_t>(bestIndex), charIndices.size() - 1)];
    m_cachedDisplayMsg = raw.substr(0, finalBytePos) + "...";
    m_cachedMsgWidth = renderer.getTextWidth(m_cachedDisplayMsg, fontSize);
}

void Snackbar::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (m_sstate == SnackState::Hidden) return;

    if (width <= 10.0f || height <= 10.0f) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        x = 0.0f;
        y = 0.0f;
        width = static_cast<float>(viewport[2]);
        height = static_cast<float>(viewport[3]);
        if (width <= 10.0f || height <= 10.0f) return;
    }

    const float scale = (m_progress < 1.0f) ? (1.0f - std::pow(1.0f - m_progress, 3.0f)) : 1.0f;
    const float alpha = m_progress;

    const float paddingX = dp(16.0f);        
    const float h = dp(48.0f);
    const float fontSize = dp(14.0f);

    const float actionGap = dp(16.0f);       
    const float btnPaddingX = dp(12.0f);     
    const float maxCapsuleWidth = std::min(width - dp(32.0f), dp(480.0f));

    /** Invalidate metrics and reformat text on viewport resolution change */
    if (std::abs(m_lastLayoutW - width) > 0.5f) {
        m_lastLayoutW = width;
        m_cachedActWidth = m_currentMsg.actionText.empty() ? 0.0f : renderer.getTextWidth(m_currentMsg.actionText, fontSize);
        
        const float rightOccupied = (m_cachedActWidth > 0.0f) ? (m_cachedActWidth + btnPaddingX * 2.0f + actionGap) : 0.0f;
        const float maxMsgWidth = maxCapsuleWidth - paddingX * 2.0f - rightOccupied;

        formatSingleLineText(renderer, maxMsgWidth, fontSize);
    }

    const float rightOccupied = (m_cachedActWidth > 0.0f) ? (m_cachedActWidth + btnPaddingX * 2.0f + actionGap) : 0.0f;
    float totalWidth = std::round(paddingX + m_cachedMsgWidth + rightOccupied + (m_cachedActWidth > 0.0f ? dp(8.0f) : paddingX));
    totalWidth = std::min(totalWidth, maxCapsuleWidth);

    const float drawX = std::round(x + (width - totalWidth) * 0.5f);
    const float animOffset = (1.0f - scale) * dp(40.0f);
    const float drawY = std::round(y + height - dp(24.0f) - h + animOffset);

    m_bubbleX = drawX;
    m_bubbleY = drawY;
    m_bubbleW = totalWidth;
    m_bubbleH = h;

    M3Color bgColor = theme.onSurface;
    bgColor.a *= alpha;
    M3Color textColor = theme.surface;
    textColor.a *= alpha;
    M3Color actionColor = theme.primary;
    actionColor.a *= alpha;

    /** 1. Render Snackbar container with 4dp corner radius */
    const float cornerR = dp(4.0f);
    renderer.drawM3UI(drawX, drawY, totalWidth, h, cornerR, cornerR, cornerR, cornerR, bgColor);

    /** 2. Render cached single-line message body text */
    const float textY = std::round(drawY + (h - fontSize) * 0.5f);
    renderer.drawText(m_cachedDisplayMsg, std::round(drawX + paddingX), textY, fontSize, textColor);

    /** 3. Render action button component */
    if (m_cachedActWidth > 0.0f) {
        const float btnH = dp(36.0f);
        const float btnW = std::round(m_cachedActWidth + btnPaddingX * 2.0f);
        const float btnY = std::round(drawY + (h - btnH) * 0.5f);
        const float btnX = std::round(drawX + totalWidth - dp(8.0f) - btnW);

        m_actionReqX = btnX;
        m_actionReqW = btnW;

        M3Color btnStateCol = actionColor;
        btnStateCol.a = (m_wasMouseDown ? 0.20f : (m_isActionHovered ? 0.10f : 0.0f)) * alpha;

        if (btnStateCol.a > 0.001f) {
            const float btnR = dp(4.0f);
            renderer.drawM3UI(btnX, btnY, btnW, btnH, btnR, btnR, btnR, btnR, btnStateCol);
        }

        const float actTextX = std::round(btnX + (btnW - m_cachedActWidth) * 0.5f);
        const float actTextY = std::round(btnY + (btnH - fontSize) * 0.5f);

        renderer.drawText(m_currentMsg.actionText, actTextX, actTextY, fontSize, actionColor);
    }
}

bool Snackbar::handleMouseMove(float mx, float my) {
    if (m_sstate == SnackState::Hidden) {
        m_isActionHovered = false;
        return false;
    }

    const bool inBubble = (mx >= m_bubbleX && mx <= m_bubbleX + m_bubbleW && my >= m_bubbleY && my <= m_bubbleY + m_bubbleH);
    if (inBubble) {
        m_isActionHovered = (mx >= m_actionReqX && mx <= m_actionReqX + m_actionReqW);
        return true;
    }

    m_isActionHovered = false;
    return false;
}

bool Snackbar::handleMouseButton(int button, int action, float mx, float my) {
    if (m_sstate == SnackState::Hidden) return false;

    const bool inBubble = (mx >= m_bubbleX && mx <= m_bubbleX + m_bubbleW && my >= m_bubbleY && my <= m_bubbleY + m_bubbleH);
    if (inBubble) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                m_wasMouseDown = true;
            }
            else if (action == GLFW_RELEASE) {
                const bool wasDown = m_wasMouseDown;
                m_wasMouseDown = false;

                if (wasDown && m_isActionHovered && m_currentMsg.onAction) {
                    m_currentMsg.onAction();
                    dismiss();
                }
            }
        }
        return true;
    }
    return false;
}