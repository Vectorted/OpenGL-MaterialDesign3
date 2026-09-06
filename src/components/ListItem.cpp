/**
 * @file ListItem.cpp
 * @brief Implementation of ListItem layout metrics, typography alignments, and interaction handling.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "ListItem.hpp"
#include <GLFW/glfw3.h>
#include <cmath>

ListItem::ListItem(const std::string& headline) : m_headline(headline) {
    layout_width = MATCH_PARENT; 
    layout_height = WRAP_CONTENT;
}

ListItem* ListItem::setHeadline(const std::string& text) { m_headline = text; return this; }
ListItem* ListItem::setSupportingText(const std::string& text) { m_supportingText = text; return this; }
ListItem* ListItem::setLeadingIcon(const std::string& icon) { m_leadingIcon = icon; return this; }
ListItem* ListItem::setTrailingIcon(const std::string& icon) { m_trailingIcon = icon; return this; }
ListItem* ListItem::setTrailingText(const std::string& text) { m_trailingText = text; return this; }
ListItem* ListItem::setInteractive(bool interactive) { m_interactive = interactive; return this; }
ListItem* ListItem::setOnClickListener(std::function<void()> listener) { m_onclick = listener; return this; }

float ListItem::getPreferredHeight() {
    return 72.0f;
}

void ListItem::doLayout(float parentX, float parentY, float parentW, float parentH) {
    x = std::round(parentX + dp(margin_left));
    y = std::round(parentY + dp(margin_top));

    if (layout_width == MATCH_PARENT) width = std::round(parentW - dp(margin_left) - dp(margin_right));
    else if (layout_width >= 0.0f) width = std::round(dp(layout_width));

    height = std::round(dp(getPreferredHeight()));
}

void ListItem::render(MaterialShader& renderer, MaterialTheme& theme) {
    float drawX = std::round(x);
    float drawY = std::round(y);
    float drawW = std::round(width);
    float drawH = std::round(height);

    // 1. Render interactive hover / press state overlay
    if (m_interactive && (m_isHovered || m_isPressed)) {
        M3Color stateCol = m_isPressed ?
            M3Color{ theme.onSurface.r, theme.onSurface.g, theme.onSurface.b, 0.12f } :
            M3Color{ theme.onSurface.r, theme.onSurface.g, theme.onSurface.b, 0.08f };
        renderer.drawM3UI(drawX, drawY, drawW, drawH, 0, 0, 0, 0, stateCol);
    }

    float cursorX = std::round(drawX + dp(16.0f));         
    float centerY = std::round(drawY + drawH * 0.5f);     

    // 2. Render leading icon
    if (!m_leadingIcon.empty()) {
        M3Color iconCol = { theme.onSurfaceVariant.r, theme.onSurfaceVariant.g, theme.onSurfaceVariant.b, 1.0f };
        renderer.drawIcon(cursorX, std::round(centerY - dp(12.0f)), dp(24.0f), m_leadingIcon, iconCol);
        cursorX = std::round(cursorX + dp(24.0f + 16.0f));
    }

    M3Color headCol = { theme.onSurface.r, theme.onSurface.g, theme.onSurface.b, 1.0f };
    M3Color suppCol = { theme.onSurfaceVariant.r, theme.onSurfaceVariant.g, theme.onSurfaceVariant.b, 1.0f };

    // 3. Render headline and supporting text with pixel-aligned positions
    if (m_supportingText.empty()) {
        renderer.drawText(m_headline, cursorX, std::round(centerY - dp(8.0f)), dp(16.0f), headCol);
    }
    else {
        renderer.drawText(m_headline, cursorX, std::round(centerY - dp(16.0f)), dp(16.0f), headCol);
        renderer.drawText(m_supportingText, cursorX, std::round(centerY + dp(2.0f)), dp(14.0f), suppCol);
    }

    // 4. Render trailing elements (icon and metadata text)
    float trailX = std::round(drawX + drawW - dp(16.0f));

    if (!m_trailingIcon.empty()) {
        trailX = std::round(trailX - dp(24.0f));
        renderer.drawIcon(trailX, std::round(centerY - dp(12.0f)), dp(24.0f), m_trailingIcon, suppCol);
        trailX = std::round(trailX - dp(16.0f));
    }

    if (!m_trailingText.empty()) {
        float exactTextW = renderer.getTextWidth(m_trailingText, dp(12.0f));
        trailX = std::round(trailX - exactTextW);
        renderer.drawText(m_trailingText, trailX, std::round(centerY - dp(6.0f)), dp(12.0f), suppCol);
    }
}

bool ListItem::handleMouseMove(float mx, float my) {
    if (!m_interactive || !isVisible()) return false;
    m_isHovered = (mx >= x && mx <= x + width && my >= y && my <= y + height);
    return m_isHovered;
}

bool ListItem::handleMouseButton(int button, int action, float mx, float my) {
    if (!m_interactive || !isVisible() || button != GLFW_MOUSE_BUTTON_LEFT) return false;

    bool hit = (mx >= x && mx <= x + width && my >= y && my <= y + height);
    if (action == GLFW_PRESS && hit) {
        m_isPressed = true;
        return true;
    }
    else if (action == GLFW_RELEASE) {
        if (m_isPressed) {
            m_isPressed = false;
            if (hit && m_onclick) m_onclick();
            return true;
        }
    }
    return false;
}