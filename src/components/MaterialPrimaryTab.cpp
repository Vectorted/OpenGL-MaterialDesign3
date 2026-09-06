/**
 * @file MaterialPrimaryTab.cpp
 * @brief Implementation of MaterialTabRow layout distribution, smooth indicator kinetics, and rendering passes.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "MaterialPrimaryTab.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

MaterialTabRow::MaterialTabRow() {
    setLayoutParams(MATCH_PARENT, 48.0f);
}

float MaterialTabRow::getPreferredHeight() {
    return 48.0f;
}

void MaterialTabRow::addTab(const std::string& label, const Icon& icon) {
    m_tabs.push_back({ label, icon, 0.0f, 0.0f });
}

void MaterialTabRow::setSelectedTab(size_t index, bool animate) {
    if (index >= m_tabs.size()) return;
    m_selectedIndex = index;

    if (!animate) {
        m_indicatorX = m_tabs[index].boundsX;
        m_indicatorWidth = m_tabs[index].boundsWidth;
    }
    
    m_targetIndicatorX = m_tabs[index].boundsX;
    m_targetIndicatorW = m_tabs[index].boundsWidth;

    if (m_onTabSelected) {
        m_onTabSelected(index);
    }
}

void MaterialTabRow::doLayout(float parentX, float parentY, float parentW, float parentH) {
    View::doLayout(parentX, parentY, parentW, parentH);

    if (m_tabs.empty()) return;

    float tabWidth = this->width / (float)m_tabs.size();
    for (size_t i = 0; i < m_tabs.size(); ++i) {
        m_tabs[i].boundsX = this->x + i * tabWidth;
        m_tabs[i].boundsWidth = tabWidth;
    }

    if (m_selectedIndex < m_tabs.size()) {
        m_targetIndicatorX = m_tabs[m_selectedIndex].boundsX;
        m_targetIndicatorW = m_tabs[m_selectedIndex].boundsWidth;

        if (m_indicatorWidth == 0.0f) {
            m_indicatorX = m_targetIndicatorX;
            m_indicatorWidth = m_targetIndicatorW;
        }
    }
}

void MaterialTabRow::update(float dt) {
    View::update(dt);

    float speed = 18.0f * dt;
    m_indicatorX += (m_targetIndicatorX - m_indicatorX) * std::min(speed, 1.0f);
    m_indicatorWidth += (m_targetIndicatorW - m_indicatorWidth) * std::min(speed, 1.0f);
}

void MaterialTabRow::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (!isVisible()) return;

    // 1. Render bottom background container
    M3Color bgCol = theme.surface;
    renderer.drawM3UI(x, y, width, height, 0, 0, 0, 0, bgCol);

    // 2. Render tab text labels
    for (size_t i = 0; i < m_tabs.size(); ++i) {
        bool isSelected = (i == m_selectedIndex);
        M3Color itemColor = isSelected ? theme.primary : theme.onSurfaceVariant;

        float itemX = m_tabs[i].boundsX;
        float itemW = m_tabs[i].boundsWidth;
        float itemCenterY = y + height * 0.5f;

        float textW = renderer.getTextWidth(m_tabs[i].text, dp(14.0f));
        float textX = itemX + (itemW - textW) * 0.5f;
        float textY = itemCenterY - dp(7.0f);

        renderer.drawText(m_tabs[i].text, textX, textY, dp(14.0f), itemColor);
    }

    // 3. Render active sliding indicator bar
    if (m_indicatorWidth > 0.0f) {
        float indH = dp(m_indicatorHeightDp);
        float indR = dp(m_indicatorRadiusDp);
        float indY = y + height - indH;

        float inset = dp(16.0f);
        float finalIndX = m_indicatorX + inset;
        float finalIndW = m_indicatorWidth - inset * 2.0f;

        if (finalIndW > 0.0f) {
            M3Color lineCol = theme.primary;
            renderer.drawM3UI(
                finalIndX, indY, finalIndW, indH,
                indR, indR, 0.0f, 0.0f,
                lineCol
            );
        }
    }
}

bool MaterialTabRow::handleMouseMove(float mx, float my) {
    return View::handleMouseMove(mx, my);
}

bool MaterialTabRow::handleMouseButton(int button, int action, float mx, float my) {
    if (!isVisible()) return false;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && isInside(mx, my)) {
            state = UIState::Pressed;
            return true;
        }
        else if (action == GLFW_RELEASE && state == UIState::Pressed) {
            bool releasedInside = isInside(mx, my);
            state = releasedInside ? UIState::Hover : UIState::Normal;
            
            if (releasedInside && !m_tabs.empty()) {
                float relativeX = mx - this->x;
                float tabWidth = this->width / (float)m_tabs.size();
                size_t clickedIndex = static_cast<size_t>(relativeX / tabWidth);
                
                if (clickedIndex < m_tabs.size()) {
                    setSelectedTab(clickedIndex, true);
                }
            }
            return true;
        }
    }
    return false;
}