/**
 * @file MaterialSearch.cpp
 * @brief Implementation of MaterialSearch text input, smooth cursor interpolation, text selections, and clipboard operations.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "MaterialSearch.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstring>

MaterialSearch::MaterialSearch() {
    layout_width = MATCH_PARENT;
    layout_height = WRAP_CONTENT;

    m_leadingIcon = iconFromString("m3:search");
    m_clearIcon = iconFromString("m3:close");
}

void MaterialSearch::setText(const std::string& text) {
    m_text = text;
    m_cursorPos = (int)m_text.length();
    clearSelection();
    updateDisplayText();
    if (m_onTextChanged) m_onTextChanged(m_text);
}

void MaterialSearch::setHint(const std::string& hint) { m_hint = hint; }
void MaterialSearch::setLeadingIcon(const std::string& str) { m_leadingIcon = iconFromString(str); }
void MaterialSearch::setTrailingIcon(const std::string& str) { m_trailingIcon = iconFromString(str); }

void MaterialSearch::setOnTextChanged(std::function<void(const std::string&)> cb) { m_onTextChanged = std::move(cb); }
void MaterialSearch::setOnSubmit(std::function<void(const std::string&)> cb) { m_onSubmit = std::move(cb); }
void MaterialSearch::setOnTrailingIconClick(std::function<void()> cb) { m_onTrailingIconClick = std::move(cb); }

float MaterialSearch::getPreferredWidth() { return dp(360.0f); }
float MaterialSearch::getPreferredHeight() { return dp(56.0f); } 

void MaterialSearch::doLayout(float parentX, float parentY, float parentW, float parentH) {
    View::doLayout(parentX, parentY, parentW, parentH);
    height = dp(56.0f);
    if (layout_width >= 0) width = dp(layout_width);
    else if (layout_width == MATCH_PARENT) width = parentW - dp(margin_left) - dp(margin_right);
}

std::string MaterialSearch::getSelectedText() const {
    if (!hasSelection()) return "";
    int s = std::min(m_selectionStart, m_selectionEnd);
    int e = std::max(m_selectionStart, m_selectionEnd);
    s = std::clamp(s, 0, (int)m_text.length());
    e = std::clamp(e, 0, (int)m_text.length());
    return m_text.substr(s, e - s);
}

void MaterialSearch::deleteSelection() {
    if (!hasSelection()) return;
    int s = std::min(m_selectionStart, m_selectionEnd);
    int e = std::max(m_selectionStart, m_selectionEnd);
    s = std::clamp(s, 0, (int)m_text.length());
    e = std::clamp(e, 0, (int)m_text.length());

    m_text.erase(s, e - s);
    m_cursorPos = s;
    clearSelection();
    updateDisplayText();
    if (m_onTextChanged) m_onTextChanged(m_text);
}

void MaterialSearch::selectAll() {
    m_selectionStart = 0;
    m_selectionEnd = (int)m_text.length();
    m_cursorPos = (int)m_text.length();
}

void MaterialSearch::clearSelection() {
    m_selectionStart = -1;
    m_selectionEnd = -1;
}

void MaterialSearch::appendString(const std::string& str) {
    if (str.empty()) return;
    if (hasSelection()) deleteSelection();

    m_text.insert(m_cursorPos, str);
    m_cursorPos += (int)str.length();
    updateDisplayText();
    if (m_onTextChanged) m_onTextChanged(m_text);
}

int MaterialSearch::getCharIndexAtX(float localTextX) const {
    if (m_charByteOffsets.empty() || localTextX <= 0.0f) return 0;
    if (localTextX >= m_charByteOffsets.back().second) return (int)m_text.length();

    for (size_t i = 0; i < m_charByteOffsets.size() - 1; ++i) {
        float x1 = m_charByteOffsets[i].second;
        float x2 = m_charByteOffsets[i + 1].second;
        float mid = (x1 + x2) * 0.5f;
        if (localTextX < mid) return m_charByteOffsets[i].first;
    }
    return (int)m_text.length();
}

void MaterialSearch::update(float dt) {
    View::update(dt);

    float targetFocus = m_hasFocus ? 1.0f : 0.0f;
    m_focusAnim += (targetFocus - m_focusAnim) * 14.0f * dt;

    float targetHover = m_isHovered ? 1.0f : 0.0f;
    m_hoverAnim += (targetHover - m_hoverAnim) * 10.0f * dt;

    if (m_hasFocus) {
        m_cursorBlink += dt;
        if (m_cursorBlink > 1.25f) m_cursorBlink -= 1.25f;
    }

    if (m_targetCursorX >= 0.0f) {
        if (m_smoothCursorX < 0.0f) {
            m_smoothCursorX = m_targetCursorX;
        } else {
            float speed = 28.0f;
            m_smoothCursorX += (m_targetCursorX - m_smoothCursorX) * std::min(1.0f, dt * speed);
        }
    }
}

void MaterialSearch::render(MaterialShader& shader, MaterialTheme& theme) {
    float r = height / 2.0f; 

    M3Color bgColor = theme.surfaceContainerHigh;
    M3Color focusBgColor = theme.surface;

    M3Color currentBg;
    currentBg.r = bgColor.r + (focusBgColor.r - bgColor.r) * m_focusAnim;
    currentBg.g = bgColor.g + (focusBgColor.g - bgColor.g) * m_focusAnim;
    currentBg.b = bgColor.b + (focusBgColor.b - bgColor.b) * m_focusAnim;
    currentBg.a = bgColor.a + (focusBgColor.a - bgColor.a) * m_focusAnim;

    if (m_hoverAnim > 0.001f && !m_hasFocus) {
        currentBg = lerpColor(currentBg, theme.onSurface, m_hoverAnim * 0.08f);
    }

    shader.drawM3UI(x, y, width, height, r, r, r, r, currentBg);
    if (m_focusAnim > 0.01f) {
        M3Color borderColor = theme.primary;
        borderColor.a = m_focusAnim * 0.8f;
        shader.drawM3UI(x, y, width, height, r, r, r, r, { 0,0,0,0 }, 0, 0, 0, 0, { 0,0,0,0 }, 0, 0, { 0,0,0,0 }, borderColor, dp(1.5f));
    }

    float iconSize = dp(24.0f);
    float verticalCenterY = y + height / 2.0f;
    float paddingX = dp(16.0f);

    float leadingX = x + paddingX + iconSize / 2.0f;
    M3Color leadingColor = theme.onSurface;
    shader.drawIcon(leadingX - iconSize / 2.0f, verticalCenterY - iconSize / 2.0f, iconSize, m_leadingIcon, leadingColor);

    float fontSize = dp(16.0f);
    M3Color textColor = theme.onSurface;

    float textStartX = x + paddingX + iconSize + dp(12.0f);
    m_cachedTextRenderX = textStartX;
    float textMaxW = width - paddingX * 2.0f - iconSize * 2.0f - dp(24.0f);

    std::string display = m_displayText;
    float currW = shader.getTextWidth(display, fontSize);
    bool isTruncated = false;

    // Truncation with ellipsis
    if (currW > textMaxW && textMaxW > 0) {
        isTruncated = true;
        while (currW > textMaxW && display.length() > 1) {
            int len = 1; 
            unsigned char last = display.back();
            if ((last & 0x80) == 0x80) {
                int c = 0; 
                for (int i = (int)display.length() - 1; i >= 0; --i) { 
                    c++; 
                    if ((display[i] & 0xC0) != 0x80) break; 
                } 
                len = c;
            }
            display.erase(display.length() - len);
            currW = shader.getTextWidth(display + "...", fontSize);
        }
        display += "...";
    }

    // Cache character horizontal byte offsets
    m_charByteOffsets.clear();
    m_charByteOffsets.push_back({ 0, 0.0f });
    for (size_t i = 0; i < display.length(); ) {
        unsigned char c = display[i];
        int len = 1;
        if ((c & 0x80) == 0x80) {
            if ((c & 0xE0) == 0xC0) len = 2;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xF8) == 0xF0) len = 4;
        }
        i += len;
        float w = shader.getTextWidth(display.substr(0, i), fontSize);
        m_charByteOffsets.push_back({ (int)i, w });
    }

    // Selection highlight rendering
    if (m_hasFocus && hasSelection()) {
        int s = std::min(m_selectionStart, m_selectionEnd);
        int e = std::max(m_selectionStart, m_selectionEnd);
        float selX0 = textStartX + shader.getTextWidth(display.substr(0, std::min(s, (int)display.length())), fontSize);
        float selX1 = textStartX + shader.getTextWidth(display.substr(0, std::min(e, (int)display.length())), fontSize);
        float selW = selX1 - selX0;

        M3Color selColor = theme.primary;
        selColor.a = 0.28f;
        shader.drawM3UI(selX0, verticalCenterY - fontSize * 0.5f - dp(1.0f), selW, fontSize + dp(2.0f), dp(2.0f), dp(2.0f), dp(2.0f), dp(2.0f), selColor);
    }

    if (m_text.empty()) {
        M3Color hintColor = theme.onSurfaceVariant;
        hintColor.a *= 0.8f;
        shader.drawText(m_hint, std::round(textStartX), std::round(verticalCenterY - fontSize * 0.45f), fontSize, hintColor);
    }
    else {
        shader.drawText(display, std::round(textStartX), std::round(verticalCenterY - fontSize * 0.45f), fontSize, textColor);
    }

    // Smooth breathing cursor rendering
    if (m_hasFocus) {
        float cursorWidth = dp(2.0f);
        float rawTargetX = textStartX;
        if (isTruncated && m_cursorPos >= (int)m_text.length()) rawTargetX += currW;
        else {
            std::string before = display.substr(0, std::min(m_cursorPos, (int)display.length()));
            rawTargetX += shader.getTextWidth(before, fontSize);
        }
        float maxCursorX = textStartX + textMaxW;
        if (rawTargetX > maxCursorX) rawTargetX = maxCursorX;

        m_targetCursorX = rawTargetX;
        if (m_smoothCursorX < 0.0f) m_smoothCursorX = m_targetCursorX;

        bool isMoving = std::abs(m_smoothCursorX - m_targetCursorX) > 0.5f;

        if (!hasSelection() || m_isMouseSelecting) {
            float pulse = 1.0f;
            if (!isMoving && !m_isMouseSelecting) {
                float t = (m_cursorBlink / 1.25f) * 6.2831853f;
                float rawCos = (std::cos(t) + 1.0f) * 0.5f;
                pulse = rawCos * rawCos * (3.0f - 2.0f * rawCos);
            }

            float curAlpha = 0.05f + 0.95f * pulse;
            float curScaleY = 0.35f + 0.65f * pulse;
            float curH = fontSize * curScaleY;
            float curY = verticalCenterY - curH * 0.5f;
            float cr = cursorWidth * 0.5f;

            M3Color cursorColor = theme.primary;
            cursorColor.a = curAlpha;
            shader.drawM3UI(m_smoothCursorX, curY, cursorWidth, curH, cr, cr, cr, cr, cursorColor);
        }
    }

    // Trailing icon button rendering
    float trailingX = x + width - paddingX - iconSize / 2.0f;
    Icon activeTrailingIcon = m_text.empty() ? m_trailingIcon : m_clearIcon;
    bool isClearAction = !m_text.empty();

    if (!isIconEmpty(activeTrailingIcon)) {
        M3Color trailingColor = theme.onSurfaceVariant;
        bool iconHovered = isClearAction ? m_isClearHovered : m_isTrailingHovered;

        if (iconHovered) {
            float hoverR = dp(18.0f);
            M3Color hoverBg = theme.onSurfaceVariant; 
            hoverBg.a = 0.12f;
            shader.drawM3UI(trailingX - hoverR, verticalCenterY - hoverR, hoverR * 2, hoverR * 2, hoverR, hoverR, hoverR, hoverR, hoverBg);
            trailingColor = theme.onSurface;
        }
        shader.drawIcon(trailingX - iconSize / 2.0f, verticalCenterY - iconSize / 2.0f, iconSize, activeTrailingIcon, trailingColor);
    }
}

bool MaterialSearch::isInsideIconPos(float mx, float my, float iconCenterX, float iconCenterY, float hitRadius) {
    float dx = mx - iconCenterX;
    float dy = my - iconCenterY;
    return (dx * dx + dy * dy) <= hitRadius * hitRadius;
}

bool MaterialSearch::handleMouseMove(float mx, float my) {
    m_isHovered = isInside(mx, my);

    if (m_isMouseSelecting) {
        int idx = getCharIndexAtX(mx - m_cachedTextRenderX);
        m_selectionEnd = idx;
        m_cursorPos = idx;
        m_cursorBlink = 0.0f;
        return true;
    }

    float trailingX = x + width - dp(16.0f) - dp(12.0f);
    float verticalCenterY = y + height / 2.0f;
    float hitR = dp(16.0f);

    if (m_isHovered && !m_text.empty()) {
        m_isClearHovered = isInsideIconPos(mx, my, trailingX, verticalCenterY, hitR);
        m_isTrailingHovered = false;
    }
    else if (m_isHovered && !isIconEmpty(m_trailingIcon)) {
        m_isTrailingHovered = isInsideIconPos(mx, my, trailingX, verticalCenterY, hitR);
        m_isClearHovered = false;
    }
    else {
        m_isClearHovered = false; 
        m_isTrailingHovered = false;
    }

    return m_isHovered;
}

bool MaterialSearch::handleMouseButton(int button, int action, float mx, float my) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            if (m_isClearHovered) {
                m_text.clear();
                m_cursorPos = 0;
                clearSelection();
                updateDisplayText();
                if (m_onTextChanged) m_onTextChanged(m_text);
                m_cursorBlink = 0.0f;
                m_smoothCursorX = -1.0f;
                return true;
            }
            if (m_isTrailingHovered) {
                if (m_onTrailingIconClick) m_onTrailingIconClick();
                return true;
            }

            if (isInside(mx, my)) {
                if (!m_hasFocus) onFocusGained();

                int idx = getCharIndexAtX(mx - m_cachedTextRenderX);
                m_cursorPos = idx;
                m_selectionStart = idx;
                m_selectionEnd = idx;
                m_isMouseSelecting = true;
                m_cursorBlink = 0.0f;
                return true;
            }
            else {
                if (m_hasFocus) onFocusLost();
                return false;
            }
        }
        else if (action == GLFW_RELEASE) {
            if (m_isMouseSelecting) {
                m_isMouseSelecting = false;
                if (m_selectionStart == m_selectionEnd) clearSelection();
                return true;
            }
        }
    }
    return false;
}

void MaterialSearch::onFocusGained() { 
    m_hasFocus = true; 
    m_cursorBlink = 0.0f; 
    m_smoothCursorX = -1.0f;
}

void MaterialSearch::onFocusLost() { 
    m_hasFocus = false; 
    m_isMouseSelecting = false;
    clearSelection();
}

bool MaterialSearch::handleKey(int key, int action) {
    if (!m_hasFocus) return false;
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return false;

    GLFWwindow* win = glfwGetCurrentContext();
    bool ctrlDown = false;
    bool shiftDown = false;
    if (win) {
        ctrlDown = (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                    glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
                    glfwGetKey(win, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
                    glfwGetKey(win, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS);

        shiftDown = (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                     glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
    }

    if (ctrlDown && key == GLFW_KEY_A) {
        selectAll();
        m_cursorBlink = 0.0f;
        return true;
    }

    if (ctrlDown && key == GLFW_KEY_C) {
        if (win) {
            std::string toCopy = hasSelection() ? getSelectedText() : m_text;
            if (!toCopy.empty()) {
                glfwSetClipboardString(win, toCopy.c_str());
            }
        }
        return true;
    }

    if (ctrlDown && key == GLFW_KEY_X) {
        if (win) {
            std::string toCut = hasSelection() ? getSelectedText() : m_text;
            if (!toCut.empty()) {
                glfwSetClipboardString(win, toCut.c_str());
                if (hasSelection()) deleteSelection();
                else {
                    m_text.clear();
                    m_cursorPos = 0;
                    clearSelection();
                    updateDisplayText();
                    if (m_onTextChanged) m_onTextChanged(m_text);
                }
            }
        }
        m_cursorBlink = 0.0f;
        return true;
    }

    if (ctrlDown && key == GLFW_KEY_V) {
        if (win) {
            const char* clip = glfwGetClipboardString(win);
            if (clip && strlen(clip) > 0) {
                appendString(std::string(clip));
            }
        }
        m_cursorBlink = 0.0f;
        return true;
    }

    if (key == GLFW_KEY_BACKSPACE) {
        if (hasSelection()) {
            deleteSelection();
            m_cursorBlink = 0.0f;
            return true;
        }
        if (m_text.empty() || m_cursorPos <= 0) return true;
        int charLen = 1;
        unsigned char c = m_text[m_cursorPos - 1];
        if ((c & 0x80) == 0x80) {
            int count = 0;
            for (int i = m_cursorPos - 1; i >= 0; --i) { count++; if ((m_text[i] & 0xC0) != 0x80) break; }
            charLen = count;
        }
        m_text.erase(m_cursorPos - charLen, charLen);
        m_cursorPos -= charLen;
        updateDisplayText();
        if (m_onTextChanged) m_onTextChanged(m_text);
        m_cursorBlink = 0.0f;
        return true;
    }

    if (key == GLFW_KEY_DELETE) {
        if (hasSelection()) {
            deleteSelection();
            m_cursorBlink = 0.0f;
            return true;
        }
        if (m_cursorPos < (int)m_text.length()) {
            int charLen = 1;
            unsigned char c = m_text[m_cursorPos];
            if ((c & 0x80) == 0x80) {
                int count = 0;
                for (int i = m_cursorPos; i < (int)m_text.length(); ++i) {
                    count++;
                    if ((m_text[i] & 0xC0) != 0x80) break;
                }
                charLen = count;
            }
            m_text.erase(m_cursorPos, charLen);
            updateDisplayText();
            if (m_onTextChanged) m_onTextChanged(m_text);
        }
        m_cursorBlink = 0.0f;
        return true;
    }

    if (key == GLFW_KEY_LEFT) {
        if (shiftDown) {
            if (!hasSelection()) m_selectionStart = m_cursorPos;
            if (m_cursorPos > 0) {
                int charLen = 1;
                for (int i = m_cursorPos - 1; i >= 0; --i) {
                    if ((m_text[i] & 0xC0) != 0x80) break;
                    charLen++;
                }
                m_cursorPos = std::max(0, m_cursorPos - charLen);
            }
            m_selectionEnd = m_cursorPos;
        }
        else {
            if (hasSelection()) {
                m_cursorPos = std::min(m_selectionStart, m_selectionEnd);
                clearSelection();
            }
            else if (m_cursorPos > 0) {
                int charLen = 1;
                for (int i = m_cursorPos - 1; i >= 0; --i) {
                    if ((m_text[i] & 0xC0) != 0x80) break;
                    charLen++;
                }
                m_cursorPos = std::max(0, m_cursorPos - charLen);
            }
        }
        m_cursorBlink = 0.0f;
        return true;
    }

    if (key == GLFW_KEY_RIGHT) {
        if (shiftDown) {
            if (!hasSelection()) m_selectionStart = m_cursorPos;
            if (m_cursorPos < (int)m_text.length()) {
                int charLen = 1;
                for (int i = m_cursorPos + 1; i < (int)m_text.length(); ++i) {
                    if ((m_text[i] & 0xC0) != 0x80) break;
                    charLen++;
                }
                m_cursorPos = std::min((int)m_text.length(), m_cursorPos + charLen);
            }
            m_selectionEnd = m_cursorPos;
        }
        else {
            if (hasSelection()) {
                m_cursorPos = std::max(m_selectionStart, m_selectionEnd);
                clearSelection();
            }
            else if (m_cursorPos < (int)m_text.length()) {
                int charLen = 1;
                for (int i = m_cursorPos + 1; i < (int)m_text.length(); ++i) {
                    if ((m_text[i] & 0xC0) != 0x80) break;
                    charLen++;
                }
                m_cursorPos = std::min((int)m_text.length(), m_cursorPos + charLen);
            }
        }
        m_cursorBlink = 0.0f;
        return true;
    }

    if (key == GLFW_KEY_HOME) {
        if (shiftDown) {
            if (!hasSelection()) m_selectionStart = m_cursorPos;
            m_cursorPos = 0;
            m_selectionEnd = m_cursorPos;
        } else {
            m_cursorPos = 0;
            clearSelection();
        }
        m_cursorBlink = 0.0f;
        return true;
    }

    if (key == GLFW_KEY_END) {
        if (shiftDown) {
            if (!hasSelection()) m_selectionStart = m_cursorPos;
            m_cursorPos = (int)m_text.length();
            m_selectionEnd = m_cursorPos;
        } else {
            m_cursorPos = (int)m_text.length();
            clearSelection();
        }
        m_cursorBlink = 0.0f;
        return true;
    }

    if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
        if (m_onSubmit) m_onSubmit(m_text);
        onFocusLost();
        return true;
    }

    if (key == GLFW_KEY_ESCAPE) {
        onFocusLost();
        return true;
    }
    return false;
}

bool MaterialSearch::handleChar(unsigned int codepoint) {
    if (!m_hasFocus) return false;

    GLFWwindow* win = glfwGetCurrentContext();
    if (win) {
        bool ctrl = (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                     glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
                     glfwGetKey(win, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
                     glfwGetKey(win, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS);
        if (ctrl) return false;
    }

    if (codepoint < 32) return false;

    if (hasSelection()) deleteSelection();

    std::string utf8;
    if (codepoint < 0x80) {
        utf8.push_back((char)codepoint);
    }
    else if (codepoint < 0x800) {
        utf8.push_back((char)(0xC0 | ((codepoint >> 6) & 0x1F)));
        utf8.push_back((char)(0x80 | (codepoint & 0x3F)));
    }
    else if (codepoint < 0x10000) {
        utf8.push_back((char)(0xE0 | ((codepoint >> 12) & 0x0F)));
        utf8.push_back((char)(0x80 | ((codepoint >> 6) & 0x3F)));
        utf8.push_back((char)(0x80 | (codepoint & 0x3F)));
    }
    else {
        utf8.push_back((char)(0xF0 | ((codepoint >> 18) & 0x07)));
        utf8.push_back((char)(0x80 | ((codepoint >> 12) & 0x3F)));
        utf8.push_back((char)(0x80 | ((codepoint >> 6) & 0x3F)));
        utf8.push_back((char)(0x80 | (codepoint & 0x3F)));
    }

    m_text.insert(m_cursorPos, utf8);
    m_cursorPos += (int)utf8.length();
    updateDisplayText();
    
    m_cursorBlink = 0.0f;
    if (m_onTextChanged) m_onTextChanged(m_text);
    return true;
}

size_t MaterialSearch::getUtf8Length(const std::string& str) const {
    size_t len = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if (c < 0x80) i += 1;
        else if ((c & 0xE0) == 0xC0) i += 2;
        else if ((c & 0xF0) == 0xE0) i += 3;
        else if ((c & 0xF8) == 0xF0) i += 4;
        else i += 1;
        len++;
    }
    return len;
}

void MaterialSearch::updateDisplayText() { 
    m_displayText = m_text; 
}

MaterialSearch* MaterialSearchBuilder::build() {
    MaterialSearch* search = new MaterialSearch();
    search->setHint(m_hint);
    search->setText(m_text);
    search->setLeadingIcon(m_leadingIcon);
    search->setTrailingIcon(m_trailingIcon);
    search->setOnTextChanged(m_onTextChanged);
    search->setOnSubmit(m_onSubmit);
    search->setOnTrailingIconClick(m_onTrailingIconClick);

    search->layout_width = m_width;
    search->setMargins(m_marginLeft, m_marginTop, m_marginRight, m_marginBottom);
    return search;
}