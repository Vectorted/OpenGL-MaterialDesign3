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
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstdio>

/**
 * @brief Helper utility to sanitize color identification strings.
 * Discards whitespace and standardizes lower-case formatting for robust string token matching.
 * 
 * @param str Raw color string descriptor.
 * @return std::string Cleaned token string.
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
 * @brief Computes proportional glyph advance weight for standard ASCII characters.
 * 
 * Used during layout phase estimation to closely approximate vector font bounding widths.
 * 
 * @param c Target character code.
 * @return float Relative glyph width factor scaled against font size.
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
 * @brief Measures approximate horizontal pixel length for a single text line in dp.
 * 
 * Handles variable UTF-8 multi-byte sequence advance assumptions.
 * 
 * @param line Single-line text slice.
 * @param fontSizeDp Font size in density-independent pixels.
 * @return float Computed horizontal line span in dp.
 */
static float measureLineWidthDp(const std::string& line, float fontSizeDp) {
    float totalUnits = 0.0f;
    for (size_t i = 0; i < line.length(); ) {
        unsigned char c = line[i];
        if (c < 128) {
            totalUnits += getCharRelativeWidth((char)c);
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

TextView::TextView() {
    layout_width = WRAP_CONTENT;
    layout_height = WRAP_CONTENT;
}

void TextView::setText(const std::string& text) { m_text = text; }
void TextView::setTextSize(float sizeDp) { m_textSize = sizeDp; }

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
    
    m_useThemeColor = true;
    m_themeColorToken = colorStr;
}

float TextView::getPreferredWidth() {
    float maxLineW = 0.0f;
    size_t start = 0;
    size_t end = m_text.find('\n');

    while (end != std::string::npos) {
        std::string line = m_text.substr(start, end - start);
        float lw = measureLineWidthDp(line, m_textSize);
        if (lw > maxLineW) maxLineW = lw;
        start = end + 1;
        end = m_text.find('\n', start);
    }
    float lw = measureLineWidthDp(m_text.substr(start), m_textSize);
    if (lw > maxLineW) maxLineW = lw;

    float finalWidthDp = maxLineW;
    if (m_hasIcon && !isIconEmpty(m_icon)) {
        float iconSizeDp = m_iconSizeDp > 0.0f ? m_iconSizeDp : m_textSize * 1.2f;
        finalWidthDp += iconSizeDp + 8.0f;
    }
    
    return finalWidthDp + padding_left + padding_right;
}

float TextView::getPreferredHeight() {
    int lineCount = 1;
    for (char c : m_text) { 
        if (c == '\n') lineCount++; 
    }

    float stepY = m_textSize * m_lineSpacingMult + m_lineSpacingExtra;
    float textHeightDp = (lineCount - 1) * stepY + m_textSize;

    if (textHeightDp < m_textSize) {
        textHeightDp = m_textSize;
    }

    return textHeightDp + padding_top + padding_bottom;
}

void TextView::render(MaterialShader& renderer, MaterialTheme& theme) {
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

    int lineCount = 1;
    for (char c : m_text) {
        if (c == '\n') lineCount++;
    }
    
    float totalTextHeight = (lineCount - 1) * stepY + pSize;
    if (totalTextHeight < pSize) {
        totalTextHeight = pSize;
    }

    float textY = y + dp(padding_top) + (height - dp(padding_top + padding_bottom) - totalTextHeight) / 2.0f;
    float contentLeft = x + dp(padding_left);
    float availW = width - dp(padding_left + padding_right);

    float iconOffset = 0.0f;
    if (m_hasIcon && !isIconEmpty(m_icon)) {
        float iconSize = m_iconSizeDp > 0.0f ? dp(m_iconSizeDp) : pSize * 1.2f;
        float iconY = y + (height - iconSize) / 2.0f;
        renderer.drawIcon(contentLeft, iconY, iconSize, m_icon, finalColor);
        iconOffset = iconSize + dp(8.0f);
    }

    if (!m_text.empty()) {
        size_t start = 0;
        size_t end = m_text.find('\n');
        float currentY = textY;

        while (end != std::string::npos) {
            std::string line = m_text.substr(start, end - start);
            if (!line.empty() && line.back() == '\r') line.pop_back();

            if (!line.empty()) {
                float realTextW = renderer.getTextWidth(line, pSize);
                float drawX = contentLeft + iconOffset;

                if (m_alignment == TextAlignment::Center) {
                    drawX = contentLeft + iconOffset + (availW - iconOffset - realTextW) * 0.5f;
                } else if (m_alignment == TextAlignment::Right) {
                    drawX = contentLeft + availW - realTextW;
                }

                renderer.drawText(line, drawX, currentY, pSize, finalColor);
            }

            currentY += stepY;
            start = end + 1;
            end = m_text.find('\n', start);
        }

        std::string finalLine = m_text.substr(start);
        if (!finalLine.empty() && finalLine.back() == '\r') finalLine.pop_back();

        if (!finalLine.empty()) {
            float realTextW = renderer.getTextWidth(finalLine, pSize);
            float drawX = contentLeft + iconOffset;

            if (m_alignment == TextAlignment::Center) {
                drawX = contentLeft + iconOffset + (availW - iconOffset - realTextW) * 0.5f;
            } else if (m_alignment == TextAlignment::Right) {
                drawX = contentLeft + availW - realTextW;
            }

            renderer.drawText(finalLine, drawX, currentY, pSize, finalColor);
        }
    }
}