/**
 * @file MaterialSegmentedButton.cpp
 * @brief Implementation of Segmented Button rendering, spring-damper selection kinetics, and vector checkmark layout.
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

void MaterialSegmentedButton::setSelectedIndex(int index) {
    if (!m_isMultiSelect && index >= 0 && index < static_cast<int>(m_segments.size()) && m_selectedIndex != index) {
        m_selectedIndex = index;
        if (m_onSelectionChanged) m_onSelectionChanged(m_selectedIndex);
    }
}

void MaterialSegmentedButton::toggleMultiSelectIndex(int index) {
    if (m_isMultiSelect && index >= 0 && index < static_cast<int>(m_segments.size())) {
        m_selectedStates[index] = !m_selectedStates[index];
        if (m_onMultiSelectionChanged) m_onMultiSelectionChanged(m_selectedStates);
    }
}

float MaterialSegmentedButton::getPreferredWidth() { 
    return static_cast<float>(m_segments.size()) * dp(110.0f); 
}

float MaterialSegmentedButton::getPreferredHeight() { 
    return dp(40.0f); 
}

void MaterialSegmentedButton::update(float dt) {
    View::update(dt);
    float fdt = std::min(dt, 0.033f);
    float animSpeed = 16.0f * fdt;

    for (int i = 0; i < static_cast<int>(m_segments.size()); ++i) {
        float targetHover = (i == m_hoveredIndex) ? 1.0f : 0.0f;
        float targetPress = (i == m_pressedIndex) ? 1.0f : 0.0f;
        m_segments[i].hoverAnim += (targetHover - m_segments[i].hoverAnim) * animSpeed;
        m_segments[i].pressAnim += (targetPress - m_segments[i].pressAnim) * animSpeed;

        float targetSelect = m_isMultiSelect ? (m_selectedStates[i] ? 1.0f : 0.0f) : ((i == m_selectedIndex) ? 1.0f : 0.0f);

        float stiffness = 260.0f;
        float damping = 22.0f;
        float selectDiff = targetSelect - m_segments[i].selectAnim;
        m_segments[i].selectVel += (selectDiff * stiffness - m_segments[i].selectVel * damping) * fdt;
        m_segments[i].selectAnim += m_segments[i].selectVel * fdt;
    }
}

bool MaterialSegmentedButton::handleMouseMove(float mx, float my) {
    if (mx >= x && mx <= x + width && my >= y && my <= y + height) {
        float segmentW = width / static_cast<float>(m_segments.size());
        m_hoveredIndex = std::clamp(static_cast<int>((mx - x) / segmentW), 0, static_cast<int>(m_segments.size()) - 1);
        return true;
    }
    m_hoveredIndex = -1;
    return false;
}

void MaterialSegmentedButton::onMouseLeave() { 
    m_hoveredIndex = -1; 
    m_pressedIndex = -1; 
}

bool MaterialSegmentedButton::handleMouseButton(int button, int action, float mx, float my) {
    if (action == 1) {
        if (m_hoveredIndex != -1) { 
            m_pressedIndex = m_hoveredIndex; 
            return true; 
        }
    }
    else if (action == 0) {
        if (m_pressedIndex != -1) {
            if (m_hoveredIndex == m_pressedIndex) {
                if (m_isMultiSelect) toggleMultiSelectIndex(m_pressedIndex); 
                else setSelectedIndex(m_pressedIndex);
            }
            m_pressedIndex = -1; 
            return true;
        }
    }
    return false;
}

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

    renderer.drawM3UI(pX, pY, pW, pH, outerRadius, outerRadius, outerRadius, outerRadius, theme.outline);
    renderer.drawM3UI(pX + borderW, pY + borderW, pW - 2.0f * borderW, pH - 2.0f * borderW, innerRadius, innerRadius, innerRadius, innerRadius, theme.surface);

    float availableW = pW - 2.0f * borderW;
    float segmentW = availableW / static_cast<float>(n);
    float innerY = pY + borderW;
    float innerH = pH - 2.0f * borderW;

    for (int i = 0; i < n; ++i) {
        if (m_segments[i].selectAnim > 0.001f) {
            float segX = pX + borderW + segmentW * i;
            float rTL = (i == 0) ? innerRadius : 0.0f; 
            float rBL = rTL;
            float rTR = (i == n - 1) ? innerRadius : 0.0f; 
            float rBR = rTR;

            float drawX = segX - (i > 0 ? (borderW / 2.0f) : 0);
            float drawW = segmentW + (i > 0 ? (borderW / 2.0f) : 0) + (i < n - 1 ? (borderW / 2.0f) : 0);

            M3Color selBg = theme.secondaryContainer;
            selBg.a *= std::clamp(m_segments[i].selectAnim, 0.0f, 1.0f);
            renderer.drawM3UI(std::round(drawX), innerY, std::round(drawW), innerH, rTL, rTR, rBR, rBL, selBg);
        }
    }

    for (int i = 1; i < n; ++i) {
        float divX = std::round(pX + borderW + segmentW * i);
        float neighborActiveLvl = std::clamp(std::max(m_segments[i - 1].selectAnim, m_segments[i].selectAnim), 0.0f, 1.0f);

        M3Color divColor = theme.outline;
        divColor.a *= (1.0f - neighborActiveLvl);
        if (divColor.a > 0.001f) {
            renderer.drawM3UI(divX - borderW / 2.0f, pY, borderW, pH, 0, 0, 0, 0, divColor);
        }
    }

    for (int i = 0; i < n; ++i) {
        float segX = pX + borderW + segmentW * i;
        float rTL = (i == 0) ? innerRadius : 0.0f; 
        float rBL = rTL;
        float rTR = (i == n - 1) ? innerRadius : 0.0f; 
        float rBR = rTR;

        float stateAlpha = m_segments[i].hoverAnim * 0.08f + m_segments[i].pressAnim * 0.12f;
        if (stateAlpha > 0.001f) {
            M3Color stateObj = theme.onSurface; 
            stateObj.a *= stateAlpha;
            renderer.drawM3UI(std::round(segX), innerY, std::round(segmentW), innerH, rTL, rTR, rBR, rBL, stateObj);
        }

        float selFrac = std::clamp(m_segments[i].selectAnim, 0.0f, 1.0f);
        M3Color fTxt;
        fTxt.r = theme.onSurface.r + (theme.onSecondaryContainer.r - theme.onSurface.r) * selFrac;
        fTxt.g = theme.onSurface.g + (theme.onSecondaryContainer.g - theme.onSurface.g) * selFrac;
        fTxt.b = theme.onSurface.b + (theme.onSecondaryContainer.b - theme.onSurface.b) * selFrac;
        fTxt.a = theme.onSurface.a + (theme.onSecondaryContainer.a - theme.onSurface.a) * selFrac;

        float fontSize = dp(14.0f);
        float baseTextW = renderer.getTextWidth(m_segments[i].label, fontSize);

        float iconSize = m_hasCheckmark ? dp(16.0f) : 0.0f;
        float iconGap = m_hasCheckmark ? dp(6.0f) : 0.0f;

        float activeIconSpace = m_hasCheckmark ? ((iconSize + iconGap) * selFrac) : 0.0f;
        float totalLayoutW = baseTextW + activeIconSpace;

        float contentBoxX = segX + (segmentW - totalLayoutW) / 2.0f;
        float verticalCenterY = innerY + innerH / 2.0f;

        // 1. Render vector Checkmark Icon
        if (m_hasCheckmark && selFrac > 0.001f) {
            float activeIconSize = iconSize * selFrac;
            float iconDrawX = contentBoxX + (iconSize - activeIconSize) / 2.0f;
            float iconDrawY = verticalCenterY - activeIconSize / 2.0f;

            M3Color checkColor = fTxt;
            checkColor.a *= selFrac;

            renderer.drawIcon(std::round(iconDrawX), std::round(iconDrawY), activeIconSize, IconM3{ M3Icon::Check }, checkColor);
        }

        // 2. Render Text Label centered vertically
        float textX = contentBoxX + activeIconSpace;
        float textDrawY = verticalCenterY - fontSize * 0.5f;
        renderer.drawText(m_segments[i].label, std::round(textX), std::round(textDrawY), fontSize, fTxt);
    }
}