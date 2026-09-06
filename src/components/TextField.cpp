/**
 * @file TextField.cpp
 * @brief Implementation of Material Design 3 TextField component with 4-byte UTF-8 emoji support, smooth subpixel animations, and interactive icons.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "TextField.hpp"
#include "../shader/TextureLoader.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstring>

extern void requestUIWakeup(double seconds);

/**
 * @brief Ken Perlin C2-continuous Smootherstep easing function: 6t^5 - 15t^4 + 10t^3.
 * Guarantees zero first and second derivatives at boundaries for soft landings.
 */
static inline float md3SmootherStep(float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

TextField::TextField() {
    layout_width = WRAP_CONTENT;
    layout_height = WRAP_CONTENT;
    m_labelSizeDp = 16.0f;  
}

void TextField::setText(const std::string& text) {
    if (m_maxLength > 0 && (int)getUtf8Length(text) > m_maxLength) {
        std::string truncated;
        size_t count = 0;
        for (size_t i = 0; i < text.length() && count < (size_t)m_maxLength; ) {
            unsigned char c = text[i];
            int len = 1;
            if ((c & 0x80) == 0x80) {
                if ((c & 0xE0) == 0xC0) len = 2;
                else if ((c & 0xF0) == 0xE0) len = 3;
                else if ((c & 0xF8) == 0xF0) len = 4;
            }
            if (i + len <= text.length()) {
                truncated += text.substr(i, len);
                count++;
            }
            i += len;
        }
        m_text = truncated;
    }
    else {
        m_text = text;
    }
    m_cursorPos = (int)m_text.length();
    clearSelection();
    updateDisplayText();
    if (m_onTextChanged) m_onTextChanged(m_text);
}

void TextField::setLabel(const std::string& label) {
    m_label = label;
}

void TextField::setHint(const std::string& hint) {
    m_hint = hint;
}

void TextField::setError(const std::string& error) {
    m_error = error;
    if (!error.empty()) {
        m_state = TextFieldState::Error;
    }
    else if (m_hasFocus) {
        m_state = TextFieldState::Focused;
    }
    else {
        m_state = TextFieldState::Normal;
    }
}

void TextField::setStyle(TextFieldStyle style) {
    m_style = style;
}

void TextField::setTextSize(float sizeDp) {
    m_textSizeDp = sizeDp;
}

void TextField::setLabelSize(float sizeDp) {
    m_labelSizeDp = sizeDp;
}

void TextField::setCornerRadius(float radius) {
    m_cornerRadius = radius;
}

void TextField::setMaxLength(int maxLen) {
    m_maxLength = maxLen;
    if (m_maxLength > 0 && (int)getUtf8Length(m_text) > m_maxLength) {
        std::string truncated;
        size_t count = 0;
        for (size_t i = 0; i < m_text.length() && count < (size_t)m_maxLength; ) {
            unsigned char c = m_text[i];
            int len = 1;
            if ((c & 0x80) == 0x80) {
                if ((c & 0xE0) == 0xC0) len = 2;
                else if ((c & 0xF0) == 0xE0) len = 3;
                else if ((c & 0xF8) == 0xF0) len = 4;
            }
            if (i + len <= m_text.length()) {
                truncated += m_text.substr(i, len);
                count++;
            }
            i += len;
        }
        m_text = truncated;
        clearSelection();
        updateDisplayText();
    }
}

void TextField::setReadOnly(bool readonly) {
    m_readOnly = readonly;
}

void TextField::setPassword(bool password) {
    m_password = password;
    updateDisplayText();
}

void TextField::setLeadingIcon(const Icon& icon) {
    m_leadingIcon = icon;
    m_hasLeadingIcon = !isIconEmpty(icon);
}

void TextField::setLeadingIcon(const std::string& str) {
    m_leadingIcon = iconFromString(str);
    m_hasLeadingIcon = !isIconEmpty(m_leadingIcon);
}

void TextField::setTrailingIcon(const Icon& icon) {
    m_trailingIcon = icon;
    m_hasTrailingIcon = !isIconEmpty(icon);
}

void TextField::setTrailingIcon(const std::string& str) {
    m_trailingIcon = iconFromString(str);
    m_hasTrailingIcon = !isIconEmpty(m_trailingIcon);
}

void TextField::setClearIcon(const Icon& icon) {
    m_clearIcon = icon;
}

void TextField::setClearIcon(const std::string& str) {
    m_clearIcon = iconFromString(str);
}

void TextField::setIconSize(float sizeDp) {
    m_iconSizeDp = sizeDp;
}

void TextField::setOnTextChanged(std::function<void(const std::string&)> callback) {
    m_onTextChanged = std::move(callback);
}

void TextField::setOnSubmit(std::function<void(const std::string&)> callback) {
    m_onSubmit = std::move(callback);
}

void TextField::setOnFocusChange(std::function<void(bool)> callback) {
    m_onFocusChange = std::move(callback);
}

void TextField::setOnTrailingIconClick(std::function<void()> callback) {
    m_onTrailingIconClick = std::move(callback);
}

void TextField::setOnClearClick(std::function<void()> callback) {
    m_onClearClick = std::move(callback);
}

std::string TextField::getSelectedText() const {
    if (!hasSelection()) return "";
    int s = std::min(m_selectionStart, m_selectionEnd);
    int e = std::max(m_selectionStart, m_selectionEnd);
    s = std::clamp(s, 0, (int)m_text.length());
    e = std::clamp(e, 0, (int)m_text.length());
    return m_text.substr(s, e - s);
}

void TextField::deleteSelection() {
    if (!hasSelection() || m_readOnly) return;
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

void TextField::selectAll() {
    m_selectionStart = 0;
    m_selectionEnd = (int)m_text.length();
    m_cursorPos = (int)m_text.length();
}

void TextField::clearSelection() {
    m_selectionStart = -1;
    m_selectionEnd = -1;
}

void TextField::appendChar(char c) {
    if (m_readOnly) return;
    if (hasSelection()) deleteSelection();
    if (m_maxLength > 0 && (int)getUtf8Length(m_text) >= m_maxLength) return;

    std::string str(1, c);
    m_text.insert(m_cursorPos, str);
    m_cursorPos += (int)str.length();
    updateDisplayText();
    if (m_onTextChanged) m_onTextChanged(m_text);
}

void TextField::appendString(const std::string& str) {
    if (m_readOnly || str.empty()) return;
    if (hasSelection()) deleteSelection();

    size_t curLen = getUtf8Length(m_text);
    size_t insLen = getUtf8Length(str);

    std::string validStr = str;
    if (m_maxLength > 0 && (int)(curLen + insLen) > m_maxLength) {
        int allow = m_maxLength - (int)curLen;
        if (allow <= 0) return;
        validStr = "";
        int cnt = 0;
        for (size_t i = 0; i < str.length() && cnt < allow;) {
            unsigned char c = str[i];
            int len = 1;
            if ((c & 0x80) == 0x80) {
                if ((c & 0xE0) == 0xC0) len = 2;
                else if ((c & 0xF0) == 0xE0) len = 3;
                else if ((c & 0xF8) == 0xF0) len = 4;
            }
            if (i + len <= str.length()) {
                validStr += str.substr(i, len);
                cnt++;
            }
            i += len;
        }
    }

    m_text.insert(m_cursorPos, validStr);
    m_cursorPos += (int)validStr.length();
    updateDisplayText();
    if (m_onTextChanged) m_onTextChanged(m_text);
}

void TextField::backspace() {
    if (m_readOnly) return;
    if (hasSelection()) {
        deleteSelection();
        return;
    }
    if (m_text.empty() || m_cursorPos <= 0) return;

    int charLen = 1;
    unsigned char c = m_text[m_cursorPos - 1];
    if ((c & 0x80) == 0x80) {
        int count = 0;
        for (int i = m_cursorPos - 1; i >= 0; --i) {
            count++;
            if ((m_text[i] & 0xC0) != 0x80) break;
        }
        charLen = count;
    }

    m_text.erase(m_cursorPos - charLen, charLen);
    m_cursorPos -= charLen;
    updateDisplayText();
    if (m_onTextChanged) m_onTextChanged(m_text);
}

void TextField::clear() {
    if (m_readOnly) return;
    m_text.clear();
    m_cursorPos = 0;
    clearSelection();
    updateDisplayText();
    if (m_onTextChanged) m_onTextChanged(m_text);
}

size_t TextField::getUtf8Length(const std::string& str) const {
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

void TextField::updateDisplayText() {
    if (m_password && !m_text.empty()) {
        size_t len = getUtf8Length(m_text);
        m_displayText = std::string(len, '*');
    }
    else {
        m_displayText = m_text;
    }
}

M3Color TextField::getBorderColor(const MaterialTheme& theme) const {
    if (m_state == TextFieldState::Error) return { 0.85f, 0.15f, 0.15f, 1.0f };
    if (m_state == TextFieldState::Focused) return theme.primary;
    if (m_state == TextFieldState::Hover) return theme.onSurface;
    return theme.outline;
}

M3Color TextField::getLabelColor(const MaterialTheme& theme) const {
    if (m_state == TextFieldState::Error) return { 0.85f, 0.15f, 0.15f, 1.0f };
    if (m_state == TextFieldState::Focused) return theme.primary;
    return theme.onSurfaceVariant;
}

int TextField::getCharIndexAtX(float localTextX) const {
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

bool TextField::isInsideIconPos(float mx, float my, float iconCenterX, float iconCenterY, float hitRadius) {
    float dx = mx - iconCenterX;
    float dy = my - iconCenterY;
    return (dx * dx + dy * dy) <= hitRadius * hitRadius;
}

void TextField::update(float dt) {
    View::update(dt);
    float dtSafe = std::min(dt, 0.033f);
    bool needContinuousFrame = false;

    // 1. Floating label animation: 200ms ascent, 160ms descent
    float labelTarget = (!m_text.empty() || m_hasFocus) ? 1.0f : 0.0f;
    if (m_labelAnim != labelTarget) {
        float speed = (labelTarget > m_labelAnim) ? (1.0f / 0.20f) : (1.0f / 0.16f);
        if (m_labelAnim < labelTarget) {
            m_labelAnim = std::min(labelTarget, m_labelAnim + speed * dtSafe);
        } else {
            m_labelAnim = std::max(labelTarget, m_labelAnim - speed * dtSafe);
        }
        needContinuousFrame = true;
    }

    // 2. Active indicator bar animation: 180ms expand, 140ms retract
    float indicatorTarget = (m_hasFocus || m_isHovering || m_state == TextFieldState::Error) ? 1.0f : 0.0f;
    if (m_indicatorAnim != indicatorTarget) {
        float speed = (indicatorTarget > m_indicatorAnim) ? (1.0f / 0.18f) : (1.0f / 0.14f);
        if (m_indicatorAnim < indicatorTarget) {
            m_indicatorAnim = std::min(indicatorTarget, m_indicatorAnim + speed * dtSafe);
        } else {
            m_indicatorAnim = std::max(indicatorTarget, m_indicatorAnim - speed * dtSafe);
        }
        needContinuousFrame = true;
    }

    m_cursorBlink += dt;
    if (m_cursorBlink > 1.25f) m_cursorBlink -= 1.25f;

    if (m_targetCursorX >= 0.0f) {
        if (m_smoothCursorX < 0.0f) {
            m_smoothCursorX = m_targetCursorX;
        } else {
            float speed = 28.0f;
            m_smoothCursorX += (m_targetCursorX - m_smoothCursorX) * std::min(1.0f, dtSafe * speed);
            if (std::abs(m_targetCursorX - m_smoothCursorX) < 0.15f) {
                m_smoothCursorX = m_targetCursorX;
            } else {
                needContinuousFrame = true;
            }
        }
    }

    if (needContinuousFrame) {
        requestUIWakeup(0.0);
    }

    m_showClearIcon = m_hasFocus && !m_text.empty() && !isIconEmpty(m_clearIcon);
}

float TextField::getPreferredWidth() {
    if (layout_width >= 0) return layout_width;
    if (layout_width == MATCH_PARENT) return MATCH_PARENT;
    return dp(280.0f);
}

float TextField::getPreferredHeight() {
    return dp(56.0f);
}

void TextField::doLayout(float parentX, float parentY, float parentW, float parentH) {
    View::doLayout(parentX, parentY, parentW, parentH);
    height = dp(56.0f);
    
    if (layout_width >= 0) width = dp(layout_width);
    else if (layout_width == MATCH_PARENT) width = parentW - dp(margin_left) - dp(margin_right);
    else width = dp(280.0f);
}

void TextField::render(MaterialShader& shader, MaterialTheme& theme) {
    if (m_style == TextFieldStyle::Outlined) renderOutlined(shader, theme);
    else if (m_style == TextFieldStyle::Filled) renderFilled(shader, theme);
    else renderUnderlined(shader, theme);
}

void TextField::renderOutlined(MaterialShader& shader, MaterialTheme& theme) {
    float pX = x;
    float pY = y;
    float pW = width;
    float pH = height;

    float r = dp(m_cornerRadius);
    float borderW = m_hasFocus ? dp(2.0f) : dp(1.0f);
    float normalFontSize = dp(m_textSizeDp);
    float smallFontSize = dp(12.0f);
    float iconSizePx = dp(m_iconSizeDp);

    float iconOffset = 0.0f;
    if (m_hasLeadingIcon) iconOffset += iconSizePx + dp(12.0f);
    float trailingOffset = 0.0f;
    if (m_hasTrailingIcon || m_showClearIcon) trailingOffset += iconSizePx + dp(12.0f);

    M3Color borderColor = getBorderColor(theme);
    shader.drawM3UI(pX, pY, pW, pH, r, r, r, r, { 0, 0, 0, 0 });
    shader.drawM3UI(pX, pY, pW, pH, r, r, r, r, { 0,0,0,0 }, 0, 0, 0, 0, { 0,0,0,0 }, 0, 0, { 0,0,0,0 }, borderColor, borderW);

    float ease = md3SmootherStep(m_labelAnim);

    float textY = pY + (pH - normalFontSize) * 0.5f;

    float restLabelY = textY;
    float floatingLabelY = pY - smallFontSize * 0.5f;
    float currentLabelY = restLabelY + (floatingLabelY - restLabelY) * ease;
    if (m_labelAnim <= 0.0f || m_labelAnim >= 1.0f) {
        currentLabelY = std::round(currentLabelY);
    }

    float labelX = pX + dp(16.0f) + iconOffset;

    if (!m_label.empty()) {
        float labelWidth = shader.getTextWidth(m_label, smallFontSize);
        M3Color hintColor = theme.onSurfaceVariant; 
        hintColor.a = 0.6f;
        M3Color focusColor = getLabelColor(theme);
        
        M3Color drawColor = hintColor;
        drawColor.r += (focusColor.r - hintColor.r) * ease;
        drawColor.g += (focusColor.g - hintColor.g) * ease;
        drawColor.b += (focusColor.b - hintColor.b) * ease;
        drawColor.a += (focusColor.a - hintColor.a) * ease;

        if (ease > 0.01f) {
            M3Color bgColor = theme.surface;
            bgColor.a = ease;
            float notchPad = dp(4.0f);
            shader.drawM3UI(
                labelX - notchPad, 
                pY - borderW, 
                labelWidth + notchPad * 2.0f, 
                borderW * 2.0f + dp(2.0f), 
                0, 0, 0, 0, bgColor
            );
        }

        if (ease <= 0.001f) {
            shader.drawText(m_label, labelX, currentLabelY, normalFontSize, drawColor);
        } else if (ease >= 0.999f) {
            shader.drawText(m_label, labelX, currentLabelY, smallFontSize, drawColor);
        } else {
            M3Color cLarge = drawColor;
            cLarge.a *= (1.0f - ease);
            M3Color cSmall = drawColor;
            cSmall.a *= ease;
            shader.drawText(m_label, labelX, currentLabelY, normalFontSize, cLarge);
            shader.drawText(m_label, labelX, currentLabelY, smallFontSize, cSmall);
        }
    }

    float textX = pX + dp(16.0f) + iconOffset;
    m_cachedTextRenderX = textX;
    float maxTextWidth = pW - dp(24.0f) - iconOffset - trailingOffset;

    std::string displayText = m_displayText;
    float textWidth = shader.getTextWidth(displayText, normalFontSize);
    bool isTruncated = false;

    if (textWidth > maxTextWidth && maxTextWidth > 0) {
        isTruncated = true;
        while (textWidth > maxTextWidth && displayText.length() > 1) {
            int lastCharLen = 1;
            unsigned char last = displayText.back();
            if ((last & 0x80) == 0x80) {
                int count = 0;
                for (int i = (int)displayText.length() - 1; i >= 0; --i) {
                    count++;
                    if ((displayText[i] & 0xC0) != 0x80) break;
                }
                lastCharLen = count;
            }
            displayText.erase(displayText.length() - lastCharLen);
            textWidth = shader.getTextWidth(displayText + "...", normalFontSize);
        }
        displayText += "...";
    }

    m_charByteOffsets.clear();
    m_charByteOffsets.push_back({ 0, 0.0f });
    for (size_t i = 0; i < displayText.length(); ) {
        unsigned char c = displayText[i];
        int len = 1;
        if ((c & 0x80) == 0x80) {
            if ((c & 0xE0) == 0xC0) len = 2;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xF8) == 0xF0) len = 4;
        }
        i += len;
        float w = shader.getTextWidth(displayText.substr(0, i), normalFontSize);
        m_charByteOffsets.push_back({ (int)i, w });
    }

    if (m_hasFocus && hasSelection()) {
        int s = std::min(m_selectionStart, m_selectionEnd);
        int e = std::max(m_selectionStart, m_selectionEnd);
        float selX0 = textX + shader.getTextWidth(displayText.substr(0, std::min(s, (int)displayText.length())), normalFontSize);
        float selX1 = textX + shader.getTextWidth(displayText.substr(0, std::min(e, (int)displayText.length())), normalFontSize);
        float selW = selX1 - selX0;

        M3Color selColor = theme.primary;
        selColor.a = 0.28f;
        shader.drawM3UI(selX0, textY - dp(1.0f), selW, normalFontSize + dp(2.0f), dp(2.0f), dp(2.0f), dp(2.0f), dp(2.0f), selColor);
    }

    M3Color textColor = (m_state == TextFieldState::Error) ? M3Color{ 0.85f, 0.15f, 0.15f, 1.0f } : theme.onSurface;
    if (!m_text.empty()) {
        shader.drawText(displayText, textX, textY, normalFontSize, textColor);
    }
    else if (!m_hint.empty()) {
        float hintAlpha = m_label.empty() ? 0.6f : (0.6f * ease);
        if (hintAlpha > 0.01f) {
            M3Color hintColor = theme.onSurfaceVariant;
            hintColor.a = hintAlpha;
            shader.drawText(m_hint, textX, textY, normalFontSize, hintColor);
        }
    }

    float trailingCenterY = pY + pH * 0.5f;
    float trailingCenterX = pX + pW - dp(12.0f) - iconSizePx * 0.5f;
    Icon activeTrailingIcon = m_showClearIcon ? m_clearIcon : m_trailingIcon;
    bool isClearAction = m_showClearIcon;

    if (m_hasLeadingIcon && !isIconEmpty(m_leadingIcon)) {
        M3Color leadingColor = (m_state == TextFieldState::Error) ? M3Color{ 0.85f, 0.15f, 0.15f, 1.0f } : theme.onSurfaceVariant;
        shader.drawIcon(pX + dp(12.0f), trailingCenterY - iconSizePx * 0.5f, iconSizePx, m_leadingIcon, leadingColor);
    }

    if (!isIconEmpty(activeTrailingIcon)) {
        M3Color trailingColor = (m_state == TextFieldState::Error) ? M3Color{ 0.85f, 0.15f, 0.15f, 1.0f } : theme.onSurfaceVariant;
        bool iconHovered = isClearAction ? m_isClearHovered : m_isTrailingHovered;

        if (iconHovered) {
            float hoverR = iconSizePx * 0.8f;
            M3Color hoverBg = theme.onSurfaceVariant; 
            hoverBg.a = 0.12f;
            shader.drawM3UI(trailingCenterX - hoverR, trailingCenterY - hoverR, hoverR * 2.0f, hoverR * 2.0f, hoverR, hoverR, hoverR, hoverR, hoverBg);
            trailingColor = theme.onSurface;
        }
        shader.drawIcon(trailingCenterX - iconSizePx * 0.5f, trailingCenterY - iconSizePx * 0.5f, iconSizePx, activeTrailingIcon, trailingColor);
    }

    if (m_hasFocus && !m_readOnly) {
        float cursorWidth = dp(2.0f);
        float rawTargetX = textX;

        if (isTruncated && m_cursorPos >= (int)m_text.length()) rawTargetX += textWidth;
        else {
            std::string before = displayText.substr(0, std::min(m_cursorPos, (int)displayText.length()));
            rawTargetX += shader.getTextWidth(before, normalFontSize);
        }

        float maxCursorX = textX + maxTextWidth;
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
            float curH = normalFontSize * curScaleY;
            float curY = textY + (normalFontSize - curH) * 0.5f;
            float cr = cursorWidth * 0.5f;

            M3Color cursorCol = theme.primary; 
            cursorCol.a = curAlpha;
            shader.drawM3UI(m_smoothCursorX, curY, cursorWidth, curH, cr, cr, cr, cr, cursorCol);
        }
    }
}

void TextField::renderFilled(MaterialShader& shader, MaterialTheme& theme) {
    float pX = x;
    float pY = y;
    float pW = width;
    float pH = height;

    float r = dp(m_cornerRadius);
    float normalFontSize = dp(m_textSizeDp);
    float smallFontSize = dp(12.0f);
    float iconSizePx = dp(m_iconSizeDp);

    float iconOffset = 0.0f, trailingOffset = 0.0f;
    if (m_hasLeadingIcon) iconOffset += iconSizePx + dp(12.0f);
    if (m_hasTrailingIcon || m_showClearIcon) trailingOffset += iconSizePx + dp(12.0f);

    M3Color bgColor = theme.surfaceVariant;
    if (m_state == TextFieldState::Error) bgColor = { 0.95f, 0.85f, 0.85f, 1.0f };
    shader.drawM3UI(pX, pY, pW, pH, r, r, 0.0f, 0.0f, bgColor);

    float baseLineH = dp(1.0f);
    M3Color baseLineColor = theme.onSurfaceVariant; 
    baseLineColor.a = 0.35f;
    shader.drawM3UI(pX, pY + pH - baseLineH, pW, baseLineH, 0, 0, 0, 0, baseLineColor);

    if (m_indicatorAnim > 0.001f) {
        float indEase = md3SmootherStep(m_indicatorAnim);
        float activeLineH = dp(2.0f);
        float activeW = pW * indEase;
        float activeX = pX + (pW - activeW) * 0.5f;
        float lineR = activeLineH * 0.5f;

        M3Color activeColor = (m_state == TextFieldState::Error) ? M3Color{ 0.85f, 0.15f, 0.15f, 1.0f }
                            : (m_hasFocus ? theme.primary : theme.onSurface);
        if (!m_hasFocus && m_state != TextFieldState::Error) activeColor.a *= 0.75f;

        shader.drawM3UI(activeX, pY + pH - activeLineH, activeW, activeLineH, lineR, lineR, lineR, lineR, activeColor);
    }

    float ease = md3SmootherStep(m_labelAnim);

    float textY = m_label.empty() ? (pY + (pH - normalFontSize) * 0.5f) : (pY + dp(26.0f));

    float restLabelY = pY + (pH - normalFontSize) * 0.5f;
    float floatingLabelY = pY + dp(8.0f);
    float currentLabelY = restLabelY + (floatingLabelY - restLabelY) * ease;
    if (m_labelAnim <= 0.0f || m_labelAnim >= 1.0f) {
        currentLabelY = std::round(currentLabelY);
    }

    float labelX = pX + dp(16.0f) + iconOffset;
    if (!m_label.empty()) {
        M3Color hintColor = theme.onSurfaceVariant; 
        hintColor.a = 0.6f;
        M3Color focusColor = getLabelColor(theme);
        M3Color drawColor = hintColor;
        drawColor.r += (focusColor.r - hintColor.r) * ease;
        drawColor.g += (focusColor.g - hintColor.g) * ease;
        drawColor.b += (focusColor.b - hintColor.b) * ease;
        drawColor.a += (focusColor.a - hintColor.a) * ease;

        if (ease <= 0.001f) {
            shader.drawText(m_label, labelX, currentLabelY, normalFontSize, drawColor);
        } else if (ease >= 0.999f) {
            shader.drawText(m_label, labelX, currentLabelY, smallFontSize, drawColor);
        } else {
            M3Color cLarge = drawColor;
            cLarge.a *= (1.0f - ease);
            M3Color cSmall = drawColor;
            cSmall.a *= ease;
            shader.drawText(m_label, labelX, currentLabelY, normalFontSize, cLarge);
            shader.drawText(m_label, labelX, currentLabelY, smallFontSize, cSmall);
        }
    }

    float textX = pX + dp(16.0f) + iconOffset;
    m_cachedTextRenderX = textX;
    float maxTextWidth = pW - dp(24.0f) - iconOffset - trailingOffset;

    std::string displayText = m_displayText;
    float textWidth = shader.getTextWidth(displayText, normalFontSize);
    bool isTruncated = false;

    if (textWidth > maxTextWidth && maxTextWidth > 0) {
        isTruncated = true;
        while (textWidth > maxTextWidth && displayText.length() > 1) {
            int len = 1; 
            unsigned char last = displayText.back();
            if ((last & 0x80) == 0x80) {
                int count = 0; 
                for (int i = (int)displayText.length() - 1; i >= 0; --i) { 
                    count++; 
                    if ((displayText[i] & 0xC0) != 0x80) break; 
                } 
                len = count;
            }
            displayText.erase(displayText.length() - len);
            textWidth = shader.getTextWidth(displayText + "...", normalFontSize);
        }
        displayText += "...";
    }

    m_charByteOffsets.clear();
    m_charByteOffsets.push_back({ 0, 0.0f });
    for (size_t i = 0; i < displayText.length(); ) {
        unsigned char c = displayText[i];
        int len = 1;
        if ((c & 0x80) == 0x80) {
            if ((c & 0xE0) == 0xC0) len = 2;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xF8) == 0xF0) len = 4;
        }
        i += len;
        float w = shader.getTextWidth(displayText.substr(0, i), normalFontSize);
        m_charByteOffsets.push_back({ (int)i, w });
    }

    if (m_hasFocus && hasSelection()) {
        int s = std::min(m_selectionStart, m_selectionEnd);
        int e = std::max(m_selectionStart, m_selectionEnd);
        float selX0 = textX + shader.getTextWidth(displayText.substr(0, std::min(s, (int)displayText.length())), normalFontSize);
        float selX1 = textX + shader.getTextWidth(displayText.substr(0, std::min(e, (int)displayText.length())), normalFontSize);
        float selW = selX1 - selX0;

        M3Color selColor = theme.primary;
        selColor.a = 0.28f;
        shader.drawM3UI(selX0, textY - dp(1.0f), selW, normalFontSize + dp(2.0f), dp(2.0f), dp(2.0f), dp(2.0f), dp(2.0f), selColor);
    }

    M3Color textColor = (m_state == TextFieldState::Error) ? M3Color{ 0.85f, 0.15f, 0.15f, 1.0f } : theme.onSurface;
    if (!m_text.empty()) {
        shader.drawText(displayText, textX, textY, normalFontSize, textColor);
    }
    else if (!m_hint.empty()) {
        float hintAlpha = m_label.empty() ? 0.6f : (0.6f * ease);
        if (hintAlpha > 0.01f) {
            M3Color hintColor = theme.onSurfaceVariant;
            hintColor.a = hintAlpha;
            shader.drawText(m_hint, textX, textY, normalFontSize, hintColor);
        }
    }

    float trailingCenterY = pY + pH * 0.5f;
    float trailingCenterX = pX + pW - dp(12.0f) - iconSizePx * 0.5f;
    Icon activeTrailingIcon = m_showClearIcon ? m_clearIcon : m_trailingIcon;
    bool isClearAction = m_showClearIcon;

    if (m_hasLeadingIcon && !isIconEmpty(m_leadingIcon)) {
        M3Color leadingColor = (m_state == TextFieldState::Error) ? M3Color{ 0.85f, 0.15f, 0.15f, 1.0f } : theme.onSurfaceVariant;
        shader.drawIcon(pX + dp(12.0f), trailingCenterY - iconSizePx * 0.5f, iconSizePx, m_leadingIcon, leadingColor);
    }

    if (!isIconEmpty(activeTrailingIcon)) {
        M3Color trailingColor = (m_state == TextFieldState::Error) ? M3Color{ 0.85f, 0.15f, 0.15f, 1.0f } : theme.onSurfaceVariant;
        bool iconHovered = isClearAction ? m_isClearHovered : m_isTrailingHovered;

        if (iconHovered) {
            float hoverR = iconSizePx * 0.8f;
            M3Color hoverBg = theme.onSurfaceVariant; 
            hoverBg.a = 0.12f;
            shader.drawM3UI(trailingCenterX - hoverR, trailingCenterY - hoverR, hoverR * 2.0f, hoverR * 2.0f, hoverR, hoverR, hoverR, hoverR, hoverBg);
            trailingColor = theme.onSurface;
        }
        shader.drawIcon(trailingCenterX - iconSizePx * 0.5f, trailingCenterY - iconSizePx * 0.5f, iconSizePx, activeTrailingIcon, trailingColor);
    }

    if (m_hasFocus && !m_readOnly) {
        float cursorWidth = dp(2.0f); 
        float rawTargetX = textX;
        if (isTruncated && m_cursorPos >= (int)m_text.length()) rawTargetX += textWidth;
        else {
            std::string before = displayText.substr(0, std::min(m_cursorPos, (int)displayText.length())); 
            rawTargetX += shader.getTextWidth(before, normalFontSize); 
        }
        float maxCursorX = textX + maxTextWidth; 
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
            float curH = normalFontSize * curScaleY;
            float curY = textY + (normalFontSize - curH) * 0.5f;
            float cr = cursorWidth * 0.5f;

            M3Color cursorCol = theme.primary; 
            cursorCol.a = curAlpha;
            shader.drawM3UI(m_smoothCursorX, curY, cursorWidth, curH, cr, cr, cr, cr, cursorCol);
        }
    }
}

void TextField::renderUnderlined(MaterialShader& shader, MaterialTheme& theme) {
    float pX = x;
    float pY = y;
    float pW = width;
    float pH = height;

    float normalFontSize = dp(m_textSizeDp);
    float smallFontSize = dp(12.0f);
    float iconSizePx = dp(m_iconSizeDp);

    float iconOffset = 0.0f, trailingOffset = 0.0f;
    if (m_hasLeadingIcon) iconOffset += iconSizePx + dp(12.0f);
    if (m_hasTrailingIcon || m_showClearIcon) trailingOffset += iconSizePx + dp(12.0f);

    float baseLineH = dp(1.0f);
    M3Color baseLineColor = theme.outline;
    baseLineColor.a = 0.35f;
    shader.drawM3UI(pX, pY + pH - baseLineH, pW, baseLineH, 0, 0, 0, 0, baseLineColor);

    if (m_indicatorAnim > 0.001f) {
        float indEase = md3SmootherStep(m_indicatorAnim);
        float activeLineH = dp(2.0f);
        float activeW = pW * indEase;
        float activeX = pX + (pW - activeW) * 0.5f;
        float lineR = activeLineH * 0.5f;

        M3Color activeColor = (m_state == TextFieldState::Error) ? M3Color{ 0.85f, 0.15f, 0.15f, 1.0f }
                            : (m_hasFocus ? theme.primary : theme.onSurface);
        
        if (!m_hasFocus && m_state != TextFieldState::Error) {
            activeColor.a *= 0.75f;
        }

        shader.drawM3UI(activeX, pY + pH - activeLineH, activeW, activeLineH, lineR, lineR, lineR, lineR, activeColor);
    }

    float ease = md3SmootherStep(m_labelAnim);

    float textY = m_label.empty() ? (pY + (pH - normalFontSize) * 0.5f) : (pY + dp(24.0f));

    float restLabelY = pY + (pH - normalFontSize) * 0.5f;
    float floatingLabelY = pY + dp(4.0f);
    float currentLabelY = restLabelY + (floatingLabelY - restLabelY) * ease;
    if (m_labelAnim <= 0.0f || m_labelAnim >= 1.0f) {
        currentLabelY = std::round(currentLabelY);
    }

    float labelX = pX + dp(16.0f) + iconOffset;
    if (!m_label.empty()) {
        M3Color hintColor = theme.onSurfaceVariant; 
        hintColor.a = 0.6f;
        M3Color focusColor = getLabelColor(theme);
        M3Color drawColor = hintColor;
        drawColor.r += (focusColor.r - hintColor.r) * ease;
        drawColor.g += (focusColor.g - hintColor.g) * ease;
        drawColor.b += (focusColor.b - hintColor.b) * ease;
        drawColor.a += (focusColor.a - hintColor.a) * ease;

        if (ease <= 0.001f) {
            shader.drawText(m_label, labelX, currentLabelY, normalFontSize, drawColor);
        } else if (ease >= 0.999f) {
            shader.drawText(m_label, labelX, currentLabelY, smallFontSize, drawColor);
        } else {
            M3Color cLarge = drawColor;
            cLarge.a *= (1.0f - ease);
            M3Color cSmall = drawColor;
            cSmall.a *= ease;
            shader.drawText(m_label, labelX, currentLabelY, normalFontSize, cLarge);
            shader.drawText(m_label, labelX, currentLabelY, smallFontSize, cSmall);
        }
    }

    float textX = pX + dp(16.0f) + iconOffset;
    m_cachedTextRenderX = textX;
    float maxTextWidth = pW - dp(24.0f) - iconOffset - trailingOffset;

    std::string displayText = m_displayText;
    float textWidth = shader.getTextWidth(displayText, normalFontSize);
    bool isTruncated = false;

    if (textWidth > maxTextWidth && maxTextWidth > 0) {
        isTruncated = true;
        while (textWidth > maxTextWidth && displayText.length() > 1) {
            int len = 1; 
            unsigned char last = displayText.back();
            if ((last & 0x80) == 0x80) {
                int count = 0; 
                for (int i = (int)displayText.length() - 1; i >= 0; --i) { 
                    count++; 
                    if ((displayText[i] & 0xC0) != 0x80) break; 
                } 
                len = count;
            }
            displayText.erase(displayText.length() - len);
            textWidth = shader.getTextWidth(displayText + "...", normalFontSize);
        }
        displayText += "...";
    }

    m_charByteOffsets.clear();
    m_charByteOffsets.push_back({ 0, 0.0f });
    for (size_t i = 0; i < displayText.length(); ) {
        unsigned char c = displayText[i];
        int len = 1;
        if ((c & 0x80) == 0x80) {
            if ((c & 0xE0) == 0xC0) len = 2;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xF8) == 0xF0) len = 4;
        }
        i += len;
        float w = shader.getTextWidth(displayText.substr(0, i), normalFontSize);
        m_charByteOffsets.push_back({ (int)i, w });
    }

    if (m_hasFocus && hasSelection()) {
        int s = std::min(m_selectionStart, m_selectionEnd);
        int e = std::max(m_selectionStart, m_selectionEnd);
        float selX0 = textX + shader.getTextWidth(displayText.substr(0, std::min(s, (int)displayText.length())), normalFontSize);
        float selX1 = textX + shader.getTextWidth(displayText.substr(0, std::min(e, (int)displayText.length())), normalFontSize);
        float selW = selX1 - selX0;

        M3Color selColor = theme.primary;
        selColor.a = 0.28f;
        shader.drawM3UI(selX0, textY - dp(1.0f), selW, normalFontSize + dp(2.0f), dp(2.0f), dp(2.0f), dp(2.0f), dp(2.0f), selColor);
    }

    M3Color textColor = (m_state == TextFieldState::Error) ? M3Color{ 0.85f, 0.15f, 0.15f, 1.0f } : theme.onSurface;
    if (!m_text.empty()) {
        shader.drawText(displayText, textX, textY, normalFontSize, textColor);
    }
    else if (!m_hint.empty()) {
        float hintAlpha = m_label.empty() ? 0.6f : (0.6f * ease);
        if (hintAlpha > 0.01f) {
            M3Color hintColor = theme.onSurfaceVariant;
            hintColor.a = hintAlpha;
            shader.drawText(m_hint, textX, textY, normalFontSize, hintColor);
        }
    }

    float trailingCenterY = pY + pH * 0.5f;
    float trailingCenterX = pX + pW - dp(12.0f) - iconSizePx * 0.5f;
    Icon activeTrailingIcon = m_showClearIcon ? m_clearIcon : m_trailingIcon;
    bool isClearAction = m_showClearIcon;

    if (m_hasLeadingIcon && !isIconEmpty(m_leadingIcon)) {
        M3Color leadingColor = (m_state == TextFieldState::Error) ? M3Color{ 0.85f, 0.15f, 0.15f, 1.0f } : theme.onSurfaceVariant;
        shader.drawIcon(pX + dp(12.0f), trailingCenterY - iconSizePx * 0.5f, iconSizePx, m_leadingIcon, leadingColor);
    }

    if (!isIconEmpty(activeTrailingIcon)) {
        M3Color trailingColor = (m_state == TextFieldState::Error) ? M3Color{ 0.85f, 0.15f, 0.15f, 1.0f } : theme.onSurfaceVariant;
        bool iconHovered = isClearAction ? m_isClearHovered : m_isTrailingHovered;

        if (iconHovered) {
            float hoverR = iconSizePx * 0.8f;
            M3Color hoverBg = theme.onSurfaceVariant; 
            hoverBg.a = 0.12f;
            shader.drawM3UI(trailingCenterX - hoverR, trailingCenterY - hoverR, hoverR * 2.0f, hoverR * 2.0f, hoverR, hoverR, hoverR, hoverR, hoverBg);
            trailingColor = theme.onSurface;
        }
        shader.drawIcon(trailingCenterX - iconSizePx * 0.5f, trailingCenterY - iconSizePx * 0.5f, iconSizePx, activeTrailingIcon, trailingColor);
    }

    if (m_hasFocus && !m_readOnly) {
        float cursorWidth = dp(2.0f); 
        float rawTargetX = textX;
        if (isTruncated && m_cursorPos >= (int)m_text.length()) rawTargetX += textWidth;
        else {
            std::string before = displayText.substr(0, std::min(m_cursorPos, (int)displayText.length())); 
            rawTargetX += shader.getTextWidth(before, normalFontSize); 
        }
        float maxCursorX = textX + maxTextWidth; 
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
            float curH = normalFontSize * curScaleY;
            float curY = textY + (normalFontSize - curH) * 0.5f;
            float cr = cursorWidth * 0.5f;

            M3Color cursorCol = theme.primary; 
            cursorCol.a = curAlpha;
            shader.drawM3UI(m_smoothCursorX, curY, cursorWidth, curH, cr, cr, cr, cr, cursorCol);
        }
    }
}

bool TextField::handleMouseMove(float mx, float my) {
    bool inside = isInside(mx, my);
    m_isHovering = inside;

    float iconSizePx = dp(m_iconSizeDp);
    float trailingCenterX = x + width - dp(12.0f) - iconSizePx / 2.0f;
    float trailingCenterY = y + height / 2.0f;
    float hitR = iconSizePx * 0.9f;

    if (m_isHovering && m_showClearIcon) {
        m_isClearHovered = isInsideIconPos(mx, my, trailingCenterX, trailingCenterY, hitR);
        m_isTrailingHovered = false;
    }
    else if (m_isHovering && m_hasTrailingIcon && !isIconEmpty(m_trailingIcon)) {
        m_isTrailingHovered = isInsideIconPos(mx, my, trailingCenterX, trailingCenterY, hitR);
        m_isClearHovered = false;
    }
    else {
        m_isClearHovered = false;
        m_isTrailingHovered = false;
    }

    if (m_isMouseSelecting) {
        int idx = getCharIndexAtX(mx - m_cachedTextRenderX);
        m_selectionEnd = idx;
        m_cursorPos = idx;
        m_cursorBlink = 0.0f;
        return true;
    }

    if (inside) {
        if (m_state == TextFieldState::Normal || m_state == TextFieldState::Hover) {
            m_state = m_hasFocus ? TextFieldState::Focused : TextFieldState::Hover;
        }
        return true;
    }
    else {
        if (!m_hasFocus && m_error.empty()) m_state = TextFieldState::Normal;
        return false;
    }
}

bool TextField::handleMouseButton(int button, int action, float mx, float my) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            if (m_isClearHovered && m_showClearIcon) {
                clear();
                m_cursorBlink = 0.0f;
                m_smoothCursorX = -1.0f;
                if (m_onClearClick) m_onClearClick();
                return true;
            }

            if (m_isTrailingHovered && !m_showClearIcon && m_hasTrailingIcon) {
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

void TextField::onFocusGained() {
    m_hasFocus = true;
    m_state = TextFieldState::Focused;
    m_cursorBlink = 0.0f;
    m_smoothCursorX = -1.0f;
    if (m_onFocusChange) m_onFocusChange(true);
}

void TextField::onFocusLost() {
    m_hasFocus = false;
    m_isMouseSelecting = false;
    m_isClearHovered = false;
    m_isTrailingHovered = false;
    clearSelection();
    if (m_error.empty()) m_state = TextFieldState::Normal;
    else m_state = TextFieldState::Error;
    m_showClearIcon = false;
    if (m_onFocusChange) m_onFocusChange(false);
}

bool TextField::handleKey(int key, int action) {
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
        if (win && !m_password) {
            std::string toCopy = hasSelection() ? getSelectedText() : m_text;
            if (!toCopy.empty()) {
                glfwSetClipboardString(win, toCopy.c_str());
            }
        }
        return true;
    }

    if (ctrlDown && key == GLFW_KEY_X) {
        if (!m_readOnly && win && !m_password) {
            std::string toCut = hasSelection() ? getSelectedText() : m_text;
            if (!toCut.empty()) {
                glfwSetClipboardString(win, toCut.c_str());
                if (hasSelection()) deleteSelection();
                else clear();
            }
        }
        m_cursorBlink = 0.0f;
        return true;
    }

    if (ctrlDown && key == GLFW_KEY_V) {
        if (!m_readOnly && win) {
            const char* clip = glfwGetClipboardString(win);
            if (clip && strlen(clip) > 0) {
                appendString(std::string(clip));
            }
        }
        m_cursorBlink = 0.0f;
        return true;
    }

    if (m_readOnly) return false;

    if (key == GLFW_KEY_BACKSPACE) {
        backspace();
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

    if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
        if (m_onSubmit) m_onSubmit(m_text);
        return true;
    }

    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_TAB) {
        onFocusLost();
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

    return false;
}

bool TextField::handleChar(unsigned int codepoint) {
    if (!m_hasFocus || m_readOnly) return false;

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

    size_t currentLen = getUtf8Length(m_text);
    if (m_maxLength > 0 && (int)currentLen >= m_maxLength) return false;

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

TextFieldBuilder::TextFieldBuilder() = default;

TextFieldBuilder& TextFieldBuilder::text(const std::string& t) { m_text = t; return *this; }
TextFieldBuilder& TextFieldBuilder::label(const std::string& l) { m_label = l; return *this; }
TextFieldBuilder& TextFieldBuilder::hint(const std::string& h) { m_hint = h; return *this; }
TextFieldBuilder& TextFieldBuilder::error(const std::string& e) { m_error = e; return *this; }
TextFieldBuilder& TextFieldBuilder::style(TextFieldStyle s) { m_style = s; return *this; }
TextFieldBuilder& TextFieldBuilder::textSize(float sizeDp) { m_textSizeDp = sizeDp; return *this; }
TextFieldBuilder& TextFieldBuilder::labelSize(float sizeDp) { m_labelSizeDp = sizeDp; return *this; }
TextFieldBuilder& TextFieldBuilder::iconSize(float sizeDp) { m_iconSizeDp = sizeDp; return *this; }
TextFieldBuilder& TextFieldBuilder::cornerRadius(float radius) { m_cornerRadius = radius; return *this; }
TextFieldBuilder& TextFieldBuilder::maxLength(int len) { m_maxLength = len; return *this; }
TextFieldBuilder& TextFieldBuilder::readOnly(bool val) { m_readOnly = val; return *this; }
TextFieldBuilder& TextFieldBuilder::password(bool val) { m_password = val; return *this; }
TextFieldBuilder& TextFieldBuilder::width(float w) { m_width = w; return *this; }
TextFieldBuilder& TextFieldBuilder::height(float h) { m_height = h; return *this; }
TextFieldBuilder& TextFieldBuilder::margins(float l, float t, float r, float b) {
    m_marginLeft = l; m_marginTop = t; m_marginRight = r; m_marginBottom = b;
    return *this;
}
TextFieldBuilder& TextFieldBuilder::leadingIcon(const Icon& icon) { m_leadingIcon = icon; return *this; }
TextFieldBuilder& TextFieldBuilder::leadingIcon(const std::string& str) { m_leadingIcon = iconFromString(str); return *this; }
TextFieldBuilder& TextFieldBuilder::trailingIcon(const Icon& icon) { m_trailingIcon = icon; return *this; }
TextFieldBuilder& TextFieldBuilder::trailingIcon(const std::string& str) { m_trailingIcon = iconFromString(str); return *this; }
TextFieldBuilder& TextFieldBuilder::clearIcon(const Icon& icon) { m_clearIcon = icon; return *this; }
TextFieldBuilder& TextFieldBuilder::clearIcon(const std::string& str) { m_clearIcon = iconFromString(str); return *this; }
TextFieldBuilder& TextFieldBuilder::onTextChanged(std::function<void(const std::string&)> cb) { m_onTextChanged = std::move(cb); return *this; }
TextFieldBuilder& TextFieldBuilder::onSubmit(std::function<void(const std::string&)> cb) { m_onSubmit = std::move(cb); return *this; }
TextFieldBuilder& TextFieldBuilder::onFocusChange(std::function<void(bool)> cb) { m_onFocusChange = std::move(cb); return *this; }
TextFieldBuilder& TextFieldBuilder::onTrailingClick(std::function<void()> cb) { m_onTrailingIconClick = std::move(cb); return *this; }
TextFieldBuilder& TextFieldBuilder::onClearClick(std::function<void()> cb) { m_onClearClick = std::move(cb); return *this; }

TextField* TextFieldBuilder::build() {
    TextField* tf = new TextField();
    tf->setText(m_text);
    tf->setLabel(m_label);
    tf->setHint(m_hint);
    tf->setError(m_error);
    tf->setStyle(m_style);
    tf->setTextSize(m_textSizeDp);
    tf->setLabelSize(m_labelSizeDp);
    tf->setIconSize(m_iconSizeDp);
    tf->setCornerRadius(m_cornerRadius);
    tf->setMaxLength(m_maxLength);
    tf->setReadOnly(m_readOnly);
    tf->setPassword(m_password);

    if (!isIconEmpty(m_leadingIcon)) tf->setLeadingIcon(m_leadingIcon);
    if (!isIconEmpty(m_trailingIcon)) tf->setTrailingIcon(m_trailingIcon);
    if (!isIconEmpty(m_clearIcon)) tf->setClearIcon(m_clearIcon);

    tf->setOnTextChanged(m_onTextChanged);
    tf->setOnSubmit(m_onSubmit);
    tf->setOnFocusChange(m_onFocusChange);
    tf->setOnTrailingIconClick(m_onTrailingIconClick);
    tf->setOnClearClick(m_onClearClick);

    if (m_width >= 0) tf->layout_width = m_width;
    else if (m_width == MATCH_PARENT) tf->layout_width = MATCH_PARENT;
    else tf->layout_width = WRAP_CONTENT;

    if (m_height >= 0) tf->layout_height = m_height;
    tf->setMargins(m_marginLeft, m_marginTop, m_marginRight, m_marginBottom);
    return tf;
}