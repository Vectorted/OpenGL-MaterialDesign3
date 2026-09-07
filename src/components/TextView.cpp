/**
 * @file TextView.cpp
 * @brief Implementation of TextView multi-line layout measurements, color string parsers, and text rendering.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "TextView.hpp"
#include <GLFW/glfw3.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cmath>

/**
 * @brief Removes whitespace and converts a string to lowercase for case‑insensitive parsing.
 * @param str Input string.
 * @return Cleaned string with no spaces and all lowercase.
 */
static std::string cleanString(const std::string& str) {
    std::string cleaned;
    for (char c : str) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            cleaned += std::tolower(static_cast<unsigned char>(c));
        }
    }
    return cleaned;
}

/**
 * @brief Returns a proportional width factor for a single ASCII character relative to font size.
 * @param c The character.
 * @return A unitless factor; typical range 0.2–0.9.
 */
static float getCharRelativeWidth(char c) {
    if (c == ' ' || c == '\t') return 0.28f;
    if (c == 'i' || c == 'l' || c == 'j' || c == '!' || c == '|' || c == '.' || c == ',' || c == ':' || c == ';') return 0.26f;
    if (c == 'f' || c == 't' || c == 'r' || c == 'I' || c == '(' || c == ')' || c == '[' || c == ']') return 0.35f;
    if (c == 'W' || c == 'M' || c == 'w' || c == 'm' || c == '@' || c == '%') return 0.85f;
    if (c >= 'A' && c <= 'Z') return 0.68f;
    if (c >= '0' && c <= '9') return 0.55f;
    if (c >= 'a' && c <= 'z') return 0.52f;
    return 0.50f;
}

/**
 * @brief Measures the width of a slice of UTF-8 text using a proportional font model.
 * @param data Pointer to the raw UTF-8 string data.
 * @param len Byte length of the slice.
 * @param fontSizeDp The font size in dp.
 * @return Estimated width in dp.
 */
static float measureLineSliceDp(const char* data, size_t len, float fontSizeDp) {
    float totalUnits = 0.0f;
    for (size_t i = 0; i < len; ) {
        unsigned char c = static_cast<unsigned char>(data[i]);
        if (c < 128) {
            totalUnits += getCharRelativeWidth(static_cast<char>(c));
            i++;
        } else {
            int bytes = 1;
            if ((c & 0xE0) == 0xC0) bytes = 2; 
            else if ((c & 0xF0) == 0xE0) bytes = 3; 
            else if ((c & 0xF8) == 0xF0) bytes = 4;
            totalUnits += 1.0f;
            i += bytes;
        }
    }
    return totalUnits * fontSizeDp;
}

// --- Constructor ---

TextView::TextView() {
    layout_width = WRAP_CONTENT;
    layout_height = WRAP_CONTENT;
}

// --- Public API implementations ---

void TextView::setText(const std::string& text) { 
    if (m_text != text) {
        m_text = text; 
        m_isDirty = true;
    }
}

void TextView::setTextSize(float sizeDp) { 
    if (m_textSize != sizeDp) {
        m_textSize = sizeDp; 
        m_isDirty = true;
    }
}

void TextView::setTextColor(const M3Color& color) { 
    m_color = color; 
    m_useThemeColor = false; 
}

void TextView::setTextColor(int r, int g, int b, int a) { 
    m_color = { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }; 
    m_useThemeColor = false; 
}

void TextView::setTextColor(const std::string& colorStr) {
    std::string clean = cleanString(colorStr);
    if (clean.empty()) return;
    
    std::string hexPart = clean;
    if (hexPart[0] == '#') {
        hexPart = hexPart.substr(1);
    }
    
    bool isHex = true;
    for (char c : hexPart) { 
        if (!std::isxdigit(static_cast<unsigned char>(c))) { 
            isHex = false; 
            break; 
        } 
    }

    if (isHex && (hexPart.length() == 6 || hexPart.length() == 8)) {
        unsigned int value = 0; 
        std::stringstream ss; 
        ss << std::hex << hexPart; 
        ss >> value;
        
        if (hexPart.length() == 6) {
            m_color = { ((value >> 16) & 0xFF) / 255.0f, ((value >> 8) & 0xFF) / 255.0f, (value & 0xFF) / 255.0f, 1.0f };
        }
        else {
            m_color = { ((value >> 16) & 0xFF) / 255.0f, ((value >> 8) & 0xFF) / 255.0f, (value & 0xFF) / 255.0f, ((value >> 24) & 0xFF) / 255.0f };
        }
        m_useThemeColor = false; 
        return;
    }

    if (clean.rfind("rgba(", 0) == 0) {
        int r = 0, g = 0, b = 0; 
        float a = 1.0f;
#if defined(_MSC_VER)
        if (sscanf_s(clean.c_str(), "rgba(%d,%d,%d,%f)", &r, &g, &b, &a) >= 4) {
#else
        if (sscanf(clean.c_str(), "rgba(%d,%d,%d,%f)", &r, &g, &b, &a) >= 4) {
#endif
            m_color = { r / 255.0f, g / 255.0f, b / 255.0f, a }; 
            m_useThemeColor = false; 
            return;
        }
    }
    else if (clean.rfind("rgb(", 0) == 0) {
        int r = 0, g = 0, b = 0;
        float a = 1.0f;
#if defined(_MSC_VER)
        if (sscanf_s(clean.c_str(), "rgb(%d,%d,%d)", &r, &g, &b, &a) >= 3) {
#else
        if (sscanf(clean.c_str(), "rgb(%d,%d,%d)", &r, &g, &b, &a) >= 3) {
#endif
            m_color = { r / 255.0f, g / 255.0f, b / 255.0f, 1.0f }; 
            m_useThemeColor = false; 
            return;
        }
    }
    
    // If not parsed as a literal color, treat as a theme token.
    m_useThemeColor = true;
    m_themeColorToken = colorStr;
}

void TextView::updateTextLayoutMetrics() {
    if (!m_isDirty) return;
    m_isDirty = false;
    m_lineRanges.clear();

    if (m_text.empty()) {
        m_cachedPreferredWidth = 0.0f;
        m_cachedPreferredHeight = m_textSize;
        return;
    }

    // Single-pass scan to build line range indices, avoiding repeated memory allocations.
    size_t len = m_text.length();
    size_t start = 0;
    float maxLineW = 0.0f;

    for (size_t i = 0; i <= len; ++i) {
        if (i == len || m_text[i] == '\n') {
            size_t lineLen = i - start;
            // Strip trailing carriage return if present.
            if (lineLen > 0 && m_text[start + lineLen - 1] == '\r') {
                lineLen--;
            }
            m_lineRanges.push_back({ start, lineLen });

            float lw = measureLineSliceDp(m_text.data() + start, lineLen, m_textSize);
            if (lw > maxLineW) maxLineW = lw;

            start = i + 1;
        }
    }

    if (m_hasIcon && !isIconEmpty(m_icon)) {
        float iconSizeDp = m_iconSizeDp > 0.0f ? m_iconSizeDp : m_textSize * 1.2f;
        maxLineW += iconSizeDp + 8.0f;
    }

    m_cachedPreferredWidth = maxLineW;

    size_t lineCount = m_lineRanges.empty() ? 1 : m_lineRanges.size();
    float stepY = m_textSize * m_lineSpacingMult + m_lineSpacingExtra;
    float textHeightDp = (lineCount - 1) * stepY + m_textSize;
    if (textHeightDp < m_textSize) textHeightDp = m_textSize;

    m_cachedPreferredHeight = textHeightDp;
}

float TextView::getPreferredWidth() {
    updateTextLayoutMetrics();
    return m_cachedPreferredWidth + padding_left + padding_right;
}

float TextView::getPreferredHeight() {
    updateTextLayoutMetrics();
    return m_cachedPreferredHeight + padding_top + padding_bottom;
}

void TextView::render(MaterialShader& renderer, MaterialTheme& theme) {
    updateTextLayoutMetrics();

    M3Color finalColor = theme.onSurface;
    if (!m_useThemeColor) { 
        finalColor = m_color; 
    }
    else {
        std::string t = cleanString(m_themeColorToken);
        if (t == "primary") finalColor = theme.primary;
        else if (t == "onprimary") finalColor = theme.onPrimary;
        else if (t == "secondary") finalColor = theme.secondary;
        else if (t == "surface") finalColor = theme.surface;
        else if (t == "onsurface") finalColor = theme.onSurface;
        else if (t == "onsurfacevariant") finalColor = theme.onSurfaceVariant;
    }

    float pSize = dp(m_textSize);
    float stepY = pSize * m_lineSpacingMult + dp(m_lineSpacingExtra);
    if (stepY <= 0.0f) stepY = pSize;

    size_t lineCount = m_lineRanges.size();
    float totalTextHeight = (lineCount > 1) ? ((lineCount - 1) * stepY + pSize) : pSize;

    float availContentH = height - dp(padding_top + padding_bottom);
    // Align to top if text exceeds available height; otherwise center vertically.
    float textY = y + dp(padding_top);
    if (availContentH > totalTextHeight) {
        textY += (availContentH - totalTextHeight) * 0.5f;
    }

    float contentLeft = x + dp(padding_left);
    float availW = width - dp(padding_left + padding_right);

    // Draw leading icon.
    float iconOffset = 0.0f;
    if (m_hasIcon && !isIconEmpty(m_icon)) {
        float iconSize = m_iconSizeDp > 0.0f ? dp(m_iconSizeDp) : pSize * 1.2f;
        float iconY = y + (height - iconSize) / 2.0f;
        renderer.drawIcon(contentLeft, iconY, iconSize, m_icon, finalColor);
        iconOffset = iconSize + dp(8.0f);
    }

    if (m_lineRanges.empty()) return;

    // --- Obtain screen/viewport clipping bounds ---
    GLFWwindow* win = glfwGetCurrentContext();
    int fbW = 0, fbH = 0;
    if (win) {
        glfwGetFramebufferSize(win, &fbW, &fbH);
    } else {
        fbH = 100000; // Safe fallback
    }

    // Guard margin (prevents clipping of partially visible characters).
    float clipTop = 0.0f - pSize * 2.0f;
    float clipBottom = static_cast<float>(fbH) + pSize * 2.0f;

    // --- Core optimization: O(1) compute visible line range via math ---
    int startLineIdx = 0;
    int endLineIdx = static_cast<int>(lineCount) - 1;

    if (textY < clipTop) {
        startLineIdx = static_cast<int>(std::floor((clipTop - textY) / stepY));
        if (startLineIdx < 0) startLineIdx = 0;
    }

    if (textY + totalTextHeight > clipBottom) {
        endLineIdx = static_cast<int>(std::ceil((clipBottom - textY) / stepY));
        if (endLineIdx >= static_cast<int>(lineCount)) {
            endLineIdx = static_cast<int>(lineCount) - 1;
        }
    }

    if (startLineIdx > endLineIdx || startLineIdx >= static_cast<int>(lineCount)) {
        return; // Completely outside viewport, skip rendering.
    }

    // Render only the visible lines.
    for (int i = startLineIdx; i <= endLineIdx; ++i) {
        const auto& range = m_lineRanges[i];
        if (range.length == 0) continue;

        float currentY = textY + i * stepY;
        std::string line = m_text.substr(range.start, range.length);

        float realTextW = renderer.getTextWidth(line, pSize);
        float drawX = contentLeft + iconOffset;

        if (m_alignment == TextAlignment::Center) {
            drawX = contentLeft + iconOffset + (availW - iconOffset - realTextW) * 0.5f;
        } else if (m_alignment == TextAlignment::Right) {
            drawX = contentLeft + availW - realTextW;
        }

        renderer.drawText(line, drawX, currentY, pSize, finalColor);
    }
}
