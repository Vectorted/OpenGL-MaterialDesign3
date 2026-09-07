/**
 * @file MaterialSegmentedButton.cpp
 * @brief Implementation of Material 3 Segmented Button rendering with ultra-smooth sliding animations and full-sized crystal-clear checkmark.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "MaterialSegmentedButton.hpp"
#include <cmath>
#include <algorithm>

extern void requestUIWakeup(double seconds);

// --- Constructors ---

MaterialSegmentedButton::MaterialSegmentedButton(const std::vector<std::string>& items, int defaultSelected)
    : m_isMultiSelect(false), m_selectedIndex(defaultSelected) {
    if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(items.size())) m_selectedIndex = 0;
    for (const auto& item : items) {
        Segment seg; 
        seg.label = item; 
        seg.selectAnim = 0.0f; 
        seg.selectVel = 0.0f;
        m_segments.push_back(seg);
    }
    if (!m_segments.empty()) m_segments[m_selectedIndex].selectAnim = 1.0f;
}

MaterialSegmentedButton::MaterialSegmentedButton(const std::vector<std::string>& items, const std::vector<bool>& defaultStates)
    : m_isMultiSelect(true), m_selectedStates(defaultStates) {
    if (m_selectedStates.size() < items.size()) m_selectedStates.resize(items.size(), false);
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        Segment seg; 
        seg.label = items[i];
        seg.selectAnim = m_selectedStates[i] ? 1.0f : 0.0f; 
        seg.selectVel = 0.0f;
        m_segments.push_back(seg);
    }
}

// --- Public API ---

void MaterialSegmentedButton::setSelectedIndex(int index) {
    if (!m_isMultiSelect && index >= 0 && index < static_cast<int>(m_segments.size()) && m_selectedIndex != index) {
        m_selectedIndex = index;
        if (m_onSelectionChanged) m_onSelectionChanged(m_selectedIndex);
        requestUIWakeup(0.35); // Wake for 350ms to ensure smooth full animation cycle
    }
}

void MaterialSegmentedButton::toggleMultiSelectIndex(int index) {
    if (m_isMultiSelect && index >= 0 && index < static_cast<int>(m_segments.size())) {
        m_selectedStates[index] = !m_selectedStates[index];
        if (m_onMultiSelectionChanged) m_onMultiSelectionChanged(m_selectedStates);
        requestUIWakeup(0.35);
    }
}

// --- View overrides ---

float MaterialSegmentedButton::getPreferredWidth() { 
    return static_cast<float>(m_segments.size()) * dp(110.0f); 
}

float MaterialSegmentedButton::getPreferredHeight() { 
    return dp(40.0f); 
}

void MaterialSegmentedButton::update(float dt) {
    View::update(dt);
    float fdt = std::min(dt, 0.033f);
    float stateAnimSpeed = 16.0f * fdt;
    bool isAnimating = false;

    // Material 3 standard transition duration is approximately 220ms (1.0 / 0.22 ≈ 4.5f)
    const float animSpeed = 4.5f;

    for (int i = 0; i < static_cast<int>(m_segments.size()); ++i) {
        // Hover and press state blending
        float targetHover = (i == m_hoveredIndex) ? 1.0f : 0.0f;
        float targetPress = (i == m_pressedIndex) ? 1.0f : 0.0f;
        m_segments[i].hoverAnim += (targetHover - m_segments[i].hoverAnim) * stateAnimSpeed;
        m_segments[i].pressAnim += (targetPress - m_segments[i].pressAnim) * stateAnimSpeed;

        float targetSelect = m_isMultiSelect ? (m_selectedStates[i] ? 1.0f : 0.0f) : ((i == m_selectedIndex) ? 1.0f : 0.0f);

        // Strict time-stepped animation ensuring 100% completion — never clips or stalls
        if (m_segments[i].selectAnim < targetSelect) {
            m_segments[i].selectAnim = std::min(targetSelect, m_segments[i].selectAnim + animSpeed * fdt);
            isAnimating = true;
        } else if (m_segments[i].selectAnim > targetSelect) {
            m_segments[i].selectAnim = std::max(targetSelect, m_segments[i].selectAnim - animSpeed * fdt);
            isAnimating = true;
        }

        if (std::abs(targetHover - m_segments[i].hoverAnim) > 0.001f ||
            std::abs(targetPress - m_segments[i].pressAnim) > 0.001f) {
            isAnimating = true;
        }
    }

    if (isAnimating) {
        requestUIWakeup(0.08); // Continuous wake until animation completes precisely
    }
}

// --- Mouse event handling ---

bool MaterialSegmentedButton::handleMouseMove(float mx, float my) {
    if (mx >= x && mx <= x + width && my >= y && my <= y + height) {
        float segmentW = width / static_cast<float>(m_segments.size());
        int newHover = std::clamp(static_cast<int>((mx - x) / segmentW), 0, static_cast<int>(m_segments.size()) - 1);
        if (newHover != m_hoveredIndex) {
            m_hoveredIndex = newHover;
            requestUIWakeup(0.1);
        }
        return true;
    }
    if (m_hoveredIndex != -1) {
        m_hoveredIndex = -1;
        requestUIWakeup(0.1);
    }
    return false;
}

void MaterialSegmentedButton::onMouseLeave() { 
    m_hoveredIndex = -1; 
    m_pressedIndex = -1; 
    requestUIWakeup(0.1);
}

bool MaterialSegmentedButton::handleMouseButton(int button, int action, float mx, float my) {
    if (button == 0) { // GLFW_MOUSE_BUTTON_LEFT
        if (action == 1) { // GLFW_PRESS
            if (m_hoveredIndex != -1) { 
                m_pressedIndex = m_hoveredIndex; 
                requestUIWakeup(0.1);
                return true; 
            }
        }
        else if (action == 0) { // GLFW_RELEASE
            if (m_pressedIndex != -1) {
                if (m_hoveredIndex == m_pressedIndex) {
                    if (m_isMultiSelect) toggleMultiSelectIndex(m_pressedIndex); 
                    else setSelectedIndex(m_pressedIndex);
                }
                m_pressedIndex = -1; 
                requestUIWakeup(0.1);
                return true;
            }
        }
    }
    return false;
}

// --- Rendering ---

void MaterialSegmentedButton::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (m_segments.empty()) return;

    float pX = std::round(x); 
    float pY = std::round(y); 
    float pW = std::round(width); 
    float pH = std::round(height);
    float borderW = std::max(1.0f, std::round(dp(1.0f)));
    int n = static_cast<int>(m_segments.size());
    float outerRadius = pH / 2.0f;
    float innerRadius = std::max(0.0f, outerRadius - borderW);

    // 1. Draw outer border and base surface
    renderer.drawM3UI(pX, pY, pW, pH, outerRadius, outerRadius, outerRadius, outerRadius, theme.outline);
    renderer.drawM3UI(pX + borderW, pY + borderW, pW - 2.0f * borderW, pH - 2.0f * borderW, innerRadius, innerRadius, innerRadius, innerRadius, theme.surface);

    float availableW = pW - 2.0f * borderW;
    float segmentW = availableW / static_cast<float>(n);
    float innerY = pY + borderW;
    float innerH = pH - 2.0f * borderW;

    // 2. Draw selected background capsules with smooth quadratic ease-in/ease-out
    for (int i = 0; i < n; ++i) {
        float t = std::clamp(m_segments[i].selectAnim, 0.0f, 1.0f);
        if (t > 0.001f) {
            float segX = pX + borderW + segmentW * i;
            float rTL = (i == 0) ? innerRadius : 0.0f; 
            float rBL = rTL;
            float rTR = (i == n - 1) ? innerRadius : 0.0f; 
            float rBR = rTR;

            float drawX = segX - (i > 0 ? (borderW / 2.0f) : 0.0f);
            float drawW = segmentW + (i > 0 ? (borderW / 2.0f) : 0.0f) + (i < n - 1 ? (borderW / 2.0f) : 0.0f);

            M3Color selBg = theme.secondaryContainer;
            selBg.a *= t;
            renderer.drawM3UI(std::round(drawX), innerY, std::round(drawW), innerH, rTL, rTR, rBR, rBL, selBg);
        }
    }

    // 3. Draw segment dividers
    for (int i = 1; i < n; ++i) {
        float divX = std::round(pX + borderW + segmentW * i);
        float neighborActiveLvl = std::clamp(std::max(m_segments[i - 1].selectAnim, m_segments[i].selectAnim), 0.0f, 1.0f);

        M3Color divColor = theme.outline;
        divColor.a *= (1.0f - neighborActiveLvl);
        if (divColor.a > 0.001f) {
            renderer.drawM3UI(divX - borderW / 2.0f, pY, borderW, pH, 0.0f, 0.0f, 0.0f, 0.0f, divColor);
        }
    }

    // 4. Draw state layers, smoothly animated text, and fully expanded checkmark for each segment
    for (int i = 0; i < n; ++i) {
        float segX = pX + borderW + segmentW * i;
        float rTL = (i == 0) ? innerRadius : 0.0f; 
        float rBL = rTL;
        float rTR = (i == n - 1) ? innerRadius : 0.0f; 
        float rBR = rTR;

        // Hover & Press state overlay
        float stateAlpha = m_segments[i].hoverAnim * 0.08f + m_segments[i].pressAnim * 0.12f;
        if (stateAlpha > 0.001f) {
            M3Color stateObj = theme.onSurface; 
            stateObj.a *= stateAlpha;
            renderer.drawM3UI(std::round(segX), innerY, std::round(segmentW), innerH, rTL, rTR, rBR, rBL, stateObj);
        }

        // Material 3 Ease-Out Cubic curve: provides a high-quality decelerating feel, no harshness
        float t = std::clamp(m_segments[i].selectAnim, 0.0f, 1.0f);
        float ease = 1.0f - std::pow(1.0f - t, 3.0f);

        // Smooth text color transition
        M3Color fTxt;
        fTxt.r = theme.onSurface.r + (theme.onSecondaryContainer.r - theme.onSurface.r) * ease;
        fTxt.g = theme.onSurface.g + (theme.onSecondaryContainer.g - theme.onSurface.g) * ease;
        fTxt.b = theme.onSurface.b + (theme.onSecondaryContainer.b - theme.onSurface.b) * ease;
        fTxt.a = 1.0f; // Always fully opaque — eliminates any grayish appearance

        float fontSize = dp(14.0f);
        float baseTextW = renderer.getTextWidth(m_segments[i].label, fontSize);
        float verticalCenterY = innerY + innerH / 2.0f;

        float iconSize = m_hasCheckmark ? dp(16.0f) : 0.0f;
        float iconGap = m_hasCheckmark ? dp(8.0f) : 0.0f;

        // Geometrically symmetric layout: unselected text centered alone; selected text + icon centered as a group
        float unselectedTextX = segX + (segmentW - baseTextW) * 0.5f;
        float shiftAmount = m_hasCheckmark ? ((iconSize + iconGap) * 0.5f) : 0.0f;

        // Current text X position smoothly moving with ease
        float currentTextX = unselectedTextX + shiftAmount * ease;
        float textY = verticalCenterY - fontSize * 0.5f;

        // Draw checkmark icon: ensures 100% full expansion — never grayed or shrunk
        if (m_hasCheckmark && ease > 0.001f) {
            // Icon size expands smoothly to full 16dp
            float curIconSize = iconSize * ease;
            
            // Maintain consistent spacing between icon and text
            float iconCenterX = currentTextX - iconGap - iconSize * 0.5f;
            float iconDrawX = iconCenterX - curIconSize * 0.5f;
            float iconDrawY = verticalCenterY - curIconSize * 0.5f;

            // Use pure Material 3 standard colors — fully saturated and vibrant
            M3Color checkColor = theme.onSecondaryContainer;
            checkColor.a = ease; // Fades in smoothly to 1.0

            renderer.drawIcon(iconDrawX, iconDrawY, curIconSize, IconM3{ M3Icon::Check }, checkColor);
        }

        // Render the segment label
        renderer.drawText(m_segments[i].label, currentTextX, textY, fontSize, fTxt);
    }
}
