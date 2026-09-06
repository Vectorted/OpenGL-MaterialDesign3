/**
 * @file Chip.cpp
 * @brief Implementation of Chip interaction states, leading/trailing icon layouts, and rendering passes.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include <glad/glad.h>
#include "Chip.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <GLFW/glfw3.h>

/**
 * @brief Constructs a Chip component with specified type and text.
 * 
 * @param type Chip variant type.
 * @param text Label text.
 */
Chip::Chip(ChipType type, const std::string& text) : m_type(type), m_text(text) {
    layout_width = WRAP_CONTENT;
    layout_height = WRAP_CONTENT;

    if (type == ChipType::Filter) {
        m_selectable = true;
    }
    if (type == ChipType::Input && m_trailingIcon.empty()) {
        m_trailingIcon = "m3:close";
    }
}

/**
 * @brief Sets the chip variant type.
 * 
 * @param type Chip variant type.
 * @return Pointer to this Chip instance.
 */
Chip* Chip::setType(ChipType type) {
    m_type = type;
    if (type == ChipType::Filter) m_selectable = true;
    if (type == ChipType::Input && m_trailingIcon.empty()) m_trailingIcon = "m3:close";
    return this;
}

/**
 * @brief Sets the text label.
 * 
 * @param text Text string.
 * @return Pointer to this Chip instance.
 */
Chip* Chip::setText(const std::string& text) { m_text = text; return this; }

/**
 * @brief Sets the leading icon resource path or identifier.
 * 
 * @param iconPath Resource identifier or asset path.
 * @return Pointer to this Chip instance.
 */
Chip* Chip::setLeadingIcon(const std::string& iconPath) { m_leadingIcon = iconPath; return this; }

/**
 * @brief Sets the trailing icon resource path or identifier.
 * 
 * @param iconPath Resource identifier or asset path.
 * @return Pointer to this Chip instance.
 */
Chip* Chip::setTrailingIcon(const std::string& iconPath) { m_trailingIcon = iconPath; return this; }

/**
 * @brief Sets the selection state.
 * 
 * @param selected True to select the chip, false otherwise.
 * @return Pointer to this Chip instance.
 */
Chip* Chip::setSelected(bool selected) { m_selected = selected; return this; }

/**
 * @brief Sets whether the chip can be toggled by user interaction.
 * 
 * @param selectable True if selectable, false otherwise.
 * @return Pointer to this Chip instance.
 */
Chip* Chip::setSelectable(bool selectable) { m_selectable = selectable; return this; }

/**
 * @brief Sets whether the chip renders with an elevated surface style.
 * 
 * @param elevated True if elevated, false otherwise.
 * @return Pointer to this Chip instance.
 */
Chip* Chip::setElevated(bool elevated) { m_elevated = elevated; return this; }

/**
 * @brief Sets whether the chip is enabled for user interaction.
 * 
 * @param enabled True to enable, false to disable.
 * @return Pointer to this Chip instance.
 */
Chip* Chip::setEnabled(bool enabled) { m_enabled = enabled; return this; }

/**
 * @brief Sets the selection change callback listener.
 * 
 * @param listener Callback function receiving the new selection state.
 * @return Pointer to this Chip instance.
 */
Chip* Chip::setOnSelectedChangeListener(std::function<void(bool)> listener) { m_onSelectedChange = listener; return this; }

/**
 * @brief Sets the trailing icon click callback listener.
 * 
 * @param listener Callback function invoked upon clicking the trailing icon.
 * @return Pointer to this Chip instance.
 */
Chip* Chip::setOnTrailingIconClickListener(std::function<void()> listener) { m_onTrailingClick = listener; return this; }

/**
 * @brief Sets the main chip click callback listener.
 * 
 * @param listener Callback function invoked upon clicking the chip body.
 * @return Pointer to this Chip instance.
 */
Chip* Chip::setOnClickListener(std::function<void()> listener) { m_onClickListener = listener; return this; }

/**
 * @brief Computes preferred layout width based on content metrics and DPI scaling.
 * 
 * @return Preferred width in device-independent pixels (dp).
 */
float Chip::getPreferredWidth() {
    if (layout_width >= 0.0f) return layout_width;

    if (width > 10.0f) {
        extern float g_dpiScale;
        return width / (g_dpiScale > 0.0f ? g_dpiScale : 1.0f);
    }

    float fontSize = 14.0f;
    float iconSize = 18.0f;
    float iconGap = 8.0f;
    float textW = m_text.length() * (fontSize * 0.55f);

    float pLeft = (m_leadingIcon.empty() && !(m_type == ChipType::Filter && m_selected)) ? 12.0f : 8.0f;
    float pRight = (m_trailingIcon.empty()) ? 12.0f : 8.0f;

    float itemsW = textW;
    if (m_type == ChipType::Filter && m_selected) { itemsW += iconSize + iconGap; }
    else if (!m_leadingIcon.empty()) { itemsW += iconSize + iconGap; }

    if (!m_trailingIcon.empty()) { itemsW += iconSize + iconGap; }
    return pLeft + itemsW + pRight;
}

/**
 * @brief Computes preferred layout height conforming to Material Design 3 guidelines.
 * 
 * @return Preferred height in device-independent pixels (dp).
 */
float Chip::getPreferredHeight() {
    return layout_height >= 0.0f ? layout_height : 32.0f;
}

/**
 * @brief Computes total content width based on current typography and icon configurations.
 * 
 * @param renderer Reference to the shader renderer for measuring font bounds.
 * @param outContentW Output reference receiving total content width in pixels.
 * @param outTextW Output reference receiving text width in pixels.
 */
void Chip::calculateContentWidth(MaterialShader& renderer, float& outContentW, float& outTextW) {
    float fontSize = dp(14.0f);
    float iconSize = std::floor(dp(18.0f) * 0.5f) * 2.0f;
    float iconGap = std::round(dp(8.0f));

    outTextW = renderer.getTextWidth(m_text, fontSize);

    float pLeft = std::round(padLeft());
    float pRight = std::round((m_trailingIcon.empty()) ? dp(12.0f) : dp(8.0f));

    float itemsW = outTextW;

    if (m_type == ChipType::Filter && m_selected) {
        itemsW += iconSize + iconGap;
    }
    else if (!m_leadingIcon.empty()) {
        itemsW += iconSize + iconGap;
    }

    if (!m_trailingIcon.empty()) {
        itemsW += iconSize + iconGap;
    }

    outContentW = std::round(pLeft + itemsW + pRight);
}

/**
 * @brief Calculates absolute positions and bounds during layout resolution.
 * 
 * @param parentX Parent origin X coordinate in pixels.
 * @param parentY Parent origin Y coordinate in pixels.
 * @param parentW Parent allocated width in pixels.
 * @param parentH Parent allocated height in pixels.
 */
void Chip::doLayout(float parentX, float parentY, float parentW, float parentH) {
    x = std::round(parentX + dp(margin_left));
    y = std::round(parentY + dp(margin_top));
    height = std::floor(dp(32.0f) * 0.5f) * 2.0f;

    if (layout_width == WRAP_CONTENT || width <= 0.0f) {
        width = std::round(dp(getPreferredWidth()));
    }
}

/**
 * @brief Updates state animations and interaction transitions.
 * 
 * @param dt Delta time in seconds.
 */
void Chip::update(float dt) {
    View::update(dt);
    if (!m_enabled) return;

    float targetStateAlpha = m_isPressed ? 0.16f : (m_isHovered ? 0.08f : 0.0f);
    m_stateAlpha += (targetStateAlpha - m_stateAlpha) * 15.0f * dt;

    float targetSelectAnim = m_selected ? 1.0f : 0.0f;
    m_selectAnim += (targetSelectAnim - m_selectAnim) * 12.0f * dt;
}

/**
 * @brief Renders the chip background, borders, icons, text, and state overlays.
 * 
 * @param renderer Reference to the active Material UI shader pipeline.
 * @param theme Reference to the active Material color tokens.
 */
void Chip::render(MaterialShader& renderer, MaterialTheme& theme) {
    float textWidth = 0.0f;
    float contentWidth = 0.0f;
    calculateContentWidth(renderer, contentWidth, textWidth);

    if (layout_width == WRAP_CONTENT) {
        width = contentWidth;
    }

    float pX = std::round(x);
    float pY = std::round(y);
    float pW = std::round(width);
    
    float rawH = height > 0.0f ? height : dp(32.0f);
    float pH = std::floor(std::round(rawH) * 0.5f) * 2.0f;
    float cornerRadius = pH * 0.5f;

    float centerY = pY + pH * 0.5f;

    float fontSize = dp(14.0f);
    float iconSize = std::floor(dp(18.0f) * 0.5f) * 2.0f;
    float iconGap = std::round(dp(8.0f));

    M3Color bgCol, textCol, borderCol, iconCol;
    bool drawBorder = true;

    if (!m_enabled) {
        bgCol = { theme.onSurface.r, theme.onSurface.g, theme.onSurface.b, 0.12f };
        textCol = { theme.onSurface.r, theme.onSurface.g, theme.onSurface.b, 0.38f };
        iconCol = textCol;
        borderCol = { theme.onSurface.r, theme.onSurface.g, theme.onSurface.b, 0.12f };
    }
    else if (m_selected) {
        drawBorder = false;
        bgCol = theme.secondaryContainer;
        textCol = theme.onSecondaryContainer;
        iconCol = theme.onSecondaryContainer;
    }
    else {
        bgCol = m_elevated ? theme.surfaceContainerLow : theme.surface;
        textCol = theme.onSurfaceVariant;
        iconCol = theme.primary;
        borderCol = theme.outlineVariant;
    }

    if (m_elevated && m_enabled) {
        M3Color shadowCol = { 0.0f, 0.0f, 0.0f, m_isHovered ? 0.12f : 0.06f };
        renderer.drawM3UI(pX, pY + std::round(dp(1.5f)), pW, pH, cornerRadius, cornerRadius, cornerRadius, cornerRadius, shadowCol);
    }

    renderer.drawM3UI(pX, pY, pW, pH, cornerRadius, cornerRadius, cornerRadius, cornerRadius, bgCol);

    if (drawBorder && !m_elevated) {
        float bw = std::max(1.0f, std::round(dp(1.0f)));
        renderer.drawM3UI(pX, pY, pW, pH, cornerRadius, cornerRadius, cornerRadius, cornerRadius, borderCol);
        renderer.drawM3UI(pX + bw, pY + bw, pW - bw * 2.0f, pH - bw * 2.0f,
            cornerRadius - bw, cornerRadius - bw, cornerRadius - bw, cornerRadius - bw, bgCol);
    }

    if (m_stateAlpha > 0.001f && m_enabled) {
        M3Color overlayCol = { textCol.r, textCol.g, textCol.b, m_stateAlpha };
        renderer.drawM3UI(pX, pY, pW, pH, cornerRadius, cornerRadius, cornerRadius, cornerRadius, overlayCol);
    }

    float curX = std::round(pX + padLeft());

    float iconCenterY = centerY - iconSize * 0.5f;
    float textCenterY = std::round(centerY - fontSize * 0.5f);

    if (m_type == ChipType::Filter && m_selected) {
        renderer.drawIcon(curX, iconCenterY, iconSize, IconM3{ M3Icon::Check }, iconCol);
        curX = std::round(curX + iconSize + iconGap);
    }
    else if (!m_leadingIcon.empty()) {
        renderer.drawIcon(curX, iconCenterY, iconSize, m_leadingIcon, iconCol);
        curX = std::round(curX + iconSize + iconGap);
    }

    renderer.drawText(m_text, curX, textCenterY, fontSize, textCol);
    curX = std::round(curX + textWidth);

    if (!m_trailingIcon.empty()) {
        curX = std::round(curX + iconGap);
        
        float splashSize = std::floor((iconSize + dp(8.0f)) * 0.5f) * 2.0f;
        m_trailingBoundsX = std::round(curX + iconSize * 0.5f - splashSize * 0.5f);
        m_trailingBoundsW = splashSize;

        if (m_isTrailingHovered && m_enabled) {
            M3Color splashCol = { textCol.r, textCol.g, textCol.b, 0.14f };
            float splashY = centerY - splashSize * 0.5f;
            renderer.drawM3UI(m_trailingBoundsX, splashY,
                m_trailingBoundsW, m_trailingBoundsW, m_trailingBoundsW * 0.5f, m_trailingBoundsW * 0.5f, m_trailingBoundsW * 0.5f, m_trailingBoundsW * 0.5f, splashCol);
        }

        renderer.drawIcon(curX, iconCenterY, iconSize, m_trailingIcon, textCol);
    }
}

/**
 * @brief Computes left padding depending on leading icon presence and filter selection.
 * 
 * @return Left padding offset in physical pixels.
 */
float Chip::padLeft() {
    return (m_leadingIcon.empty() && !(m_type == ChipType::Filter && m_selected)) ? dp(12.0f) : dp(8.0f);
}

/**
 * @brief Processes mouse cursor movements to update hover states.
 * 
 * @param mx Mouse X position in window coordinates.
 * @param my Mouse Y position in window coordinates.
 * @return True if the mouse is hovering over the chip, false otherwise.
 */
bool Chip::handleMouseMove(float mx, float my) {
    if (!m_enabled || !isVisible()) return false;

    m_isHovered = (mx >= x && mx <= x + width && my >= y && my <= y + height);
    if (m_isHovered && !m_trailingIcon.empty()) {
        m_isTrailingHovered = (mx >= m_trailingBoundsX && mx <= m_trailingBoundsX + m_trailingBoundsW);
    }
    else {
        m_isTrailingHovered = false;
    }
    return m_isHovered;
}

/**
 * @brief Processes mouse button press and release events.
 * 
 * @param button Mouse button index.
 * @param action Action type (press or release).
 * @param mx Mouse X position in window coordinates.
 * @param my Mouse Y position in window coordinates.
 * @return True if the event was consumed by the chip, false otherwise.
 */
bool Chip::handleMouseButton(int button, int action, float mx, float my) {
    if (!m_enabled || !isVisible() || button != GLFW_MOUSE_BUTTON_LEFT) return false;
    bool inChip = (mx >= x && mx <= x + width && my >= y && my <= y + height);

    if (inChip) {
        if (action == GLFW_PRESS) {
            m_isPressed = true;
        }
        else if (action == GLFW_RELEASE) {
            bool wasPressed = m_isPressed;
            m_isPressed = false;

            if (wasPressed) {
                if (!m_trailingIcon.empty() && (mx >= m_trailingBoundsX && mx <= m_trailingBoundsX + m_trailingBoundsW)) {
                    if (m_onTrailingClick) m_onTrailingClick();
                }
                else {
                    if (m_selectable) {
                        m_selected = !m_selected;
                        if (m_onSelectedChange) m_onSelectedChange(m_selected);
                    }
                    if (m_onClickListener) m_onClickListener();
                }
            }
        }
        return true;
    }
    m_isPressed = false;
    return false;
}