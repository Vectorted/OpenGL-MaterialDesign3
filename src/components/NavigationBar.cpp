/**
 * @file NavigationBar.cpp
 * @brief Implementation of NavigationBar tab distribution, active pill indicator kinetics, and rendering passes.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "NavigationBar.hpp"
#include "../shader/TextureLoader.hpp"
#include <algorithm>
#include <utility>

NavigationTab::NavigationTab(
    const std::string& label,
    const Icon& icon,
    NavigationBar* parent
)
    : m_label(label),
    m_icon(icon),
    m_parent(parent) {
    layout_width = MATCH_PARENT;
    layout_height = MATCH_PARENT;
}

NavigationTab::NavigationTab(
    const std::string& label,
    NavigationIcon icon,
    NavigationBar* parent
)
    : m_label(label),
    m_legacyIcon(icon),
    m_useLegacyIcon(true),
    m_parent(parent) {
    layout_width = MATCH_PARENT;
    layout_height = MATCH_PARENT;
}

NavigationTab::NavigationTab(
    const std::string& label,
    NavigationIconRenderer customIconRenderer,
    NavigationBar* parent
)
    : m_label(label),
    m_customIconRenderer(std::move(customIconRenderer)),
    m_parent(parent) {
    layout_width = MATCH_PARENT;
    layout_height = MATCH_PARENT;
}

void NavigationTab::setSelected(bool selected, bool instant) {
    m_selected = selected;
    if (instant) {
        m_indicatorAnim = selected ? 1.0f : 0.0f;
    }
}

float NavigationTab::getPreferredWidth() { return 64.0f; }
float NavigationTab::getPreferredHeight() { return 80.0f; }

void NavigationTab::update(float dt) {
    View::update(dt);
    float target = m_selected ? 1.0f : 0.0f;
    m_indicatorAnim += (target - m_indicatorAnim) * 12.0f * dt;
    m_indicatorAnim = std::clamp(m_indicatorAnim, 0.0f, 1.0f);
}

bool NavigationTab::handleMouseMove(float mx, float my) {
    return View::handleMouseMove(mx, my);
}

bool NavigationTab::handleMouseButton(int button, int action, float mx, float my) {
    return View::handleMouseButton(button, action, mx, my);
}

void NavigationTab::onClick() {
    if (m_parent != nullptr) {
        m_parent->onTabClicked(this);
    }
}

void NavigationTab::renderIcon(
    MaterialShader& renderer,
    MaterialTheme& theme,
    float centerX,
    float centerY
) {
    M3Color color = m_selected ? theme.onSecondaryContainer : theme.onSurfaceVariant;
    const float iconSize = dp(24.0f);

    if (m_customIconRenderer) {
        m_customIconRenderer(renderer, theme, centerX, centerY, iconSize, color, m_selected);
        return;
    }

    if (!isIconEmpty(m_icon)) {
        renderer.drawIcon(centerX - iconSize / 2.0f, centerY - iconSize / 2.0f, iconSize, m_icon, color);
        return;
    }

    if (m_useLegacyIcon) {
        const float stroke = dp(2.5f);
        const float smallRadius = dp(1.5f);
        switch (m_legacyIcon) {
        case NavigationIcon::Home: {
            renderer.drawM3UI(centerX - dp(9.0f), centerY - dp(5.0f), dp(18.0f), stroke, smallRadius, smallRadius, smallRadius, smallRadius, color);
            renderer.drawM3UI(centerX - dp(7.0f), centerY - dp(2.0f), dp(14.0f), dp(11.0f), dp(2.0f), dp(2.0f), dp(2.0f), dp(2.0f), color);
            M3Color doorColor = m_selected ? theme.secondaryContainer : theme.surface;
            renderer.drawM3UI(centerX - dp(2.0f), centerY + dp(4.0f), dp(4.0f), dp(5.0f), dp(1.0f), dp(1.0f), 0.0f, 0.0f, doorColor);
            break;
        }
        case NavigationIcon::Grid: {
            const float cellSize = dp(7.0f);
            const float gap = dp(3.0f);
            const float startX = centerX - cellSize - gap / 2.0f;
            const float startY = centerY - cellSize - gap / 2.0f;
            for (int r = 0; r < 2; ++r) {
                for (int c = 0; c < 2; ++c) {
                    renderer.drawM3UI(startX + c * (cellSize + gap), startY + r * (cellSize + gap), cellSize, cellSize, dp(2.0f), dp(2.0f), dp(2.0f), dp(2.0f), color);
                }
            }
            break;
        }
        case NavigationIcon::Message: {
            renderer.drawM3UI(centerX - dp(10.0f), centerY - dp(7.0f), dp(20.0f), dp(14.0f), dp(4.0f), dp(4.0f), dp(4.0f), dp(4.0f), color);
            M3Color mLineCol = m_selected ? theme.secondaryContainer : theme.surface;
            renderer.drawM3UI(centerX - dp(5.0f), centerY - dp(1.5f), dp(10.0f), dp(3.0f), dp(1.5f), dp(1.5f), dp(1.5f), dp(1.5f), mLineCol);
            renderer.drawM3UI(centerX - dp(6.0f), centerY + dp(5.0f), dp(6.0f), dp(5.0f), dp(1.0f), dp(1.0f), dp(1.0f), dp(1.0f), color);
            break;
        }
        case NavigationIcon::Person: {
            renderer.drawM3UI(centerX - dp(5.0f), centerY - dp(9.0f), dp(10.0f), dp(10.0f), dp(5.0f), dp(5.0f), dp(5.0f), dp(5.0f), color);
            renderer.drawM3UI(centerX - dp(9.0f), centerY + dp(3.0f), dp(18.0f), dp(9.0f), dp(8.0f), dp(8.0f), dp(2.0f), dp(2.0f), color);
            break;
        }
        }
    }
}

void NavigationTab::render(
    MaterialShader& renderer,
    MaterialTheme& theme
) {
    float pillHeight = dp(32.0f);
    float maximumPillWidth = std::max(0.0f, width - dp(8.0f));
    float pillWidth = std::min(dp(64.0f), maximumPillWidth);
    float pillY = y + dp(12.0f);

    if (m_indicatorAnim > 0.001f) {
        float animatedWidth = pillWidth * (0.72f + 0.28f * m_indicatorAnim);
        float animatedX = x + (width - animatedWidth) / 2.0f;
        M3Color indicatorColor = theme.secondaryContainer;
        indicatorColor.a *= m_indicatorAnim;

        renderer.drawM3UI(animatedX, pillY, animatedWidth, pillHeight, pillHeight / 2.0f, pillHeight / 2.0f, pillHeight / 2.0f, pillHeight / 2.0f, indicatorColor);
    }

    float stateLayerAlpha = hoverAnim * 0.08f + pressAnim * 0.12f;
    if (stateLayerAlpha > 0.001f) {
        float stateLayerX = x + (width - pillWidth) / 2.0f;
        M3Color stateLayerColor = theme.onSurfaceVariant;
        stateLayerColor.a = std::clamp(stateLayerAlpha, 0.0f, 1.0f);

        renderer.drawM3UI(stateLayerX, pillY, pillWidth, pillHeight, pillHeight / 2.0f, pillHeight / 2.0f, pillHeight / 2.0f, pillHeight / 2.0f, stateLayerColor);
    }

    renderIcon(renderer, theme, x + width / 2.0f, pillY + pillHeight / 2.0f);

    if (!m_label.empty()) {
        float fontSizePx = dp(12.0f);
        float fontWidth = renderer.getTextWidth(m_label, fontSizePx);
        float textX = x + (width - fontWidth) / 2.0f;
        float textY = y + dp(52.0f);

        M3Color labelColor = m_selected ? theme.onSurface : theme.onSurfaceVariant;
        renderer.drawText(m_label, textX, textY, fontSizePx, labelColor);
    }
}

NavigationBar::NavigationBar() {
    layout_width = MATCH_PARENT;
    layout_height = WRAP_CONTENT;
}

void NavigationBar::addTab(const std::string& label, const Icon& icon) {
    bool isFirst = children.empty();
    NavigationTab* tab = new NavigationTab(label, icon, this);
    tab->setSelected(isFirst, true);
    if (isFirst) m_selectedIndex = 0;
    addView(tab);
}

void NavigationBar::addTab(const std::string& label, NavigationIcon icon) {
    bool isFirst = children.empty();
    NavigationTab* tab = new NavigationTab(label, icon, this);
    tab->setSelected(isFirst, true);
    if (isFirst) m_selectedIndex = 0;
    addView(tab);
}

void NavigationBar::addTab(const std::string& label, NavigationIconRenderer customIconRenderer) {
    bool isFirst = children.empty();
    NavigationTab* tab = new NavigationTab(label, std::move(customIconRenderer), this);
    tab->setSelected(isFirst, true);
    if (isFirst) m_selectedIndex = 0;
    addView(tab);
}

void NavigationBar::setSelectedIndex(int index) {
    int tabCount = static_cast<int>(children.size());
    if (index < 0 || index >= tabCount) return;
    if (index == m_selectedIndex) return;

    if (m_selectedIndex >= 0 && m_selectedIndex < tabCount) {
        NavigationTab* previousTab = static_cast<NavigationTab*>(children[m_selectedIndex]);
        previousTab->setSelected(false);
    }

    NavigationTab* selectedTab = static_cast<NavigationTab*>(children[index]);
    selectedTab->setSelected(true);
    m_selectedIndex = index;

    if (m_onTabSelectedCallback) m_onTabSelectedCallback(index);
}

void NavigationBar::setOnTabSelected(std::function<void(int)> callback) {
    m_onTabSelectedCallback = std::move(callback);
}

void NavigationBar::onTabClicked(NavigationTab* tab) {
    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i] == tab) {
            setSelectedIndex(static_cast<int>(i));
            return;
        }
    }
}

void NavigationBar::setBottomPlacement(float leftInset, float rightInset, float bottomInset) {
    m_placement = NavigationBarPlacement::Bottom;
    m_insetLeft = leftInset;
    m_insetRight = rightInset;
    m_insetBottom = bottomInset;
}

void NavigationBar::setRegion(float regionX, float regionY, float regionWidth, float regionHeight) {
    m_placement = NavigationBarPlacement::Custom;
    m_regionX = regionX;
    m_regionY = regionY;
    m_regionWidth = regionWidth;
    m_regionHeight = regionHeight;
}

void NavigationBar::setCornerRadius(float radius) {
    m_cornerRadius = std::max(0.0f, radius);
}

void NavigationBar::setDrawBackground(bool enabled) {
    m_drawBackground = enabled;
}

void NavigationBar::setTopDivider(bool enabled, float thickness, float horizontalInset, float alpha) {
    m_showTopDivider = enabled;
    m_dividerThickness = std::max(0.0f, thickness);
    m_dividerHorizontalInset = std::max(0.0f, horizontalInset);
    m_dividerAlpha = std::clamp(alpha, 0.0f, 1.0f);
}

float NavigationBar::getPreferredWidth() { return 320.0f; }
float NavigationBar::getPreferredHeight() { return 80.0f; }

void NavigationBar::doLayout(float parentX, float parentY, float parentW, float parentH) {
    if (m_placement == NavigationBarPlacement::Custom) {
        float regionX = dp(m_regionX);
        float regionY = dp(m_regionY);
        x = parentX + regionX;
        y = parentY + regionY;

        width = (layout_width > 0.0f) ? dp(layout_width) : std::min(dp(m_regionWidth), std::max(0.0f, parentW - regionX));
        height = (layout_height > 0.0f) ? dp(layout_height) : std::min(dp(m_regionHeight), std::max(0.0f, parentH - regionY));
    }
    else {
        float left = dp(m_insetLeft + margin_left);
        float right = dp(m_insetRight + margin_right);
        float bottom = dp(m_insetBottom + margin_bottom);

        if (layout_width > 0.0f && layout_width != MATCH_PARENT) {
            width = dp(layout_width);
            x = parentX + (parentW - width) / 2.0f;
        }
        else {
            x = parentX + left;
            width = std::max(0.0f, parentW - left - right);
        }

        if (layout_height > 0.0f && layout_height != MATCH_PARENT) {
            height = dp(layout_height);
        }
        else {
            height = std::min(dp(getPreferredHeight()), std::max(0.0f, parentH - bottom));
        }
        y = parentY + parentH - bottom - height;
    }

    if (children.empty()) return;

    float tabWidth = width / static_cast<float>(children.size());
    float currentX = x;
    for (View* child : children) {
        child->doLayout(currentX, y, tabWidth, height);
        currentX += tabWidth;
    }
}

void NavigationBar::render(MaterialShader& renderer, MaterialTheme& theme) {
    float radius = dp(m_cornerRadius);
    if (m_drawBackground) {
        M3Color background = theme.surfaceContainer;
        background = lerpColor(background, theme.primary, 0.10f);
        renderer.drawM3UI(x, y, width, height, radius, radius, radius, radius, background);
    }

    if (m_showTopDivider && m_dividerThickness > 0.0f) {
        float dividerInset = std::min(dp(m_dividerHorizontalInset), width / 2.0f);
        float dividerX = x + dividerInset;
        float dividerWidth = std::max(0.0f, width - dividerInset * 2.0f);
        float dividerHeight = std::max(1.0f, dp(m_dividerThickness));
        M3Color dividerColor = theme.outlineVariant;
        dividerColor.a = m_dividerAlpha;
        renderer.drawM3UI(dividerX, y, dividerWidth, dividerHeight, dividerHeight / 2.0f, dividerHeight / 2.0f, dividerHeight / 2.0f, dividerHeight / 2.0f, dividerColor);
    }

    ViewGroup::render(renderer, theme);
}