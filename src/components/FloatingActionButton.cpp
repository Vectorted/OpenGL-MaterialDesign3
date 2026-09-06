/**
 * @file FloatingActionButton.cpp
 * @brief Implementation of FloatingActionButton rendering, shadow projection, and animated Speed Dial layout.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "FloatingActionButton.hpp"
#include "../shader/TextureLoader.hpp"
#include <algorithm>
#include <cmath>

FloatingActionButton::FloatingActionButton() {
    layout_width = WRAP_CONTENT;
    layout_height = WRAP_CONTENT;
}

void FloatingActionButton::setFloatingPosition(float customX, float customY) {
    m_hasCustomPosition = true;
    m_customX = customX;
    m_customY = customY;
    this->x = customX;
    this->y = customY;
}

static float getStandardIconSizeForFab(FabSize s) {
    switch (s) {
    case FabSize::Small: return 20.0f;
    case FabSize::Large: return 36.0f;
    case FabSize::Extended:
    case FabSize::Standard:
    default: return 24.0f;
    }
}

float FloatingActionButton::getPreferredWidth() {
    float actualIconSize = iconSizeDp > 0.0f ? iconSizeDp : getStandardIconSizeForFab(fabSize);

    switch (fabSize) {
    case FabSize::Small: return std::max(40.0f, actualIconSize + 16.0f);
    case FabSize::Large: return std::max(96.0f, actualIconSize + 48.0f);
    case FabSize::Extended: {
        float textW = static_cast<float>(text.length()) * 14.0f * 0.6f;
        float padding = isIconEmpty(icon) ? (20.0f * 2.0f) : (16.0f + actualIconSize + 12.0f + 20.0f);
        float iconW = isIconEmpty(icon) ? 0.0f : actualIconSize;
        return textW + iconW + padding;
    }
    case FabSize::Standard:
    default: return std::max(56.0f, actualIconSize + 24.0f);
    }
}

float FloatingActionButton::getPreferredHeight() {
    float actualIconSize = iconSizeDp > 0.0f ? iconSizeDp : getStandardIconSizeForFab(fabSize);

    switch (fabSize) {
    case FabSize::Small: return std::max(40.0f, actualIconSize + 16.0f);
    case FabSize::Large: return std::max(96.0f, actualIconSize + 48.0f);
    case FabSize::Extended:
    case FabSize::Standard:
    default: return std::max(56.0f, actualIconSize + 24.0f);
    }
}

void FloatingActionButton::addSpeedDialItem(const std::string& label, const Icon& icon, std::function<void()> onClick, bool showTextInButton, FabType type) {
    speedDialItems.push_back({ label, icon, onClick, false, type, showTextInButton, 0.0f, 0.0f });
}

void FloatingActionButton::addSpeedDialItem(const std::string& label, const std::string& iconStr, std::function<void()> onClick, bool showTextInButton, FabType type) {
    speedDialItems.push_back({ label, iconFromString(iconStr), onClick, false, type, showTextInButton, 0.0f, 0.0f });
}

void FloatingActionButton::toggleExpand() {
    isExpanded = !isExpanded;
}

void FloatingActionButton::collapse() {
    isExpanded = false;
}

void FloatingActionButton::update(float dt) {
    View::update(dt);

    if (m_hasCustomPosition) {
        this->x = m_customX;
        this->y = m_customY;
    }

    float target = isExpanded ? 1.0f : 0.0f;
    if (std::abs(m_expandProgress - target) > 0.001f) {
        float speed = 10.0f;
        if (m_expandProgress < target) {
            m_expandProgress = std::min(target, m_expandProgress + dt * speed);
        } else {
            m_expandProgress = std::max(target, m_expandProgress - dt * speed);
        }
    } else {
        m_expandProgress = target;
    }
}

void FloatingActionButton::updateItemSizes(MaterialShader* renderer) {
    for (auto& item : speedDialItems) {
        float baseH = dp(40.0f);
        float baseW = baseH;
        if (item.showTextInButton && !item.label.empty()) {
            bool hasIcon = !isIconEmpty(item.icon);
            float textW = (renderer != nullptr) ? renderer->getTextWidth(item.label, dp(13.0f)) : (static_cast<float>(item.label.length()) * dp(8.0f));
            float iconW = hasIcon ? dp(18.0f) : 0.0f;
            float gap = hasIcon ? dp(8.0f) : 0.0f;
            float padding = hasIcon ? dp(28.0f) : dp(32.0f);
            baseW = iconW + gap + textW + padding;
        }
        item.cachedWidth = baseW;
        item.cachedHeight = baseH;
    }
}

void FloatingActionButton::getSubItemBounds(size_t index, float mainW, float mainH, float ease, float& outX, float& outY, float& outW, float& outH, MaterialShader* renderer) {
    if (index >= speedDialItems.size()) return;

    updateItemSizes(renderer);

    auto& item = speedDialItems[index];
    outW = item.cachedWidth;
    outH = item.cachedHeight;

    float startX = x + (mainW - outW) * 0.5f;
    float startY = y + (mainH - outH) * 0.5f;

    float alignedX = startX;
    if (menuAlignment == MenuAlignment::Start) {
        alignedX = x;
    } else if (menuAlignment == MenuAlignment::End) {
        alignedX = x + mainW - outW;
    }

    float alignedY = startY;
    if (menuAlignment == MenuAlignment::Start) {
        alignedY = y;
    } else if (menuAlignment == MenuAlignment::End) {
        alignedY = y + mainH - outH;
    }

    float gap = dp(10.0f);
    float initialMargin = dp(12.0f);

    float targetX = startX;
    float targetY = startY;

    float accOffset = 0.0f;
    for (size_t i = 0; i < index; ++i) {
        if (expandDirection == ExpandDirection::Up || expandDirection == ExpandDirection::Down) {
            accOffset += speedDialItems[i].cachedHeight + gap;
        } else {
            accOffset += speedDialItems[i].cachedWidth + gap;
        }
    }

    switch (expandDirection) {
    case ExpandDirection::Up:
        targetX = alignedX;
        targetY = y - initialMargin - accOffset - outH;
        break;
    case ExpandDirection::Down:
        targetX = alignedX;
        targetY = y + mainH + initialMargin + accOffset;
        break;
    case ExpandDirection::Left:
        targetX = x - initialMargin - accOffset - outW;
        targetY = alignedY;
        break;
    case ExpandDirection::Right:
        targetX = x + mainW + initialMargin + accOffset;
        targetY = alignedY;
        break;
    }

    outX = startX + (targetX - startX) * ease;
    outY = startY + (targetY - startY) * ease;
}

void FloatingActionButton::render(MaterialShader& renderer, MaterialTheme& theme) {
    float actualW = width;
    float actualH = height;
    
    if (fabSize != FabSize::Extended) {
        float minSize = std::min(width, height);
        actualW = minSize;
        actualH = minSize;
    }

    float pX = std::round(x);
    float pY = std::round(y);
    float pW = std::round(actualW);
    float pH = std::round(actualH);

    float r = dp(16.0f);
    if (fabShape == FabShape::Circle) {
        r = std::min(pW, pH) * 0.5f;
    } else {
        if (fabSize == FabSize::Small) r = dp(12.0f);
        else if (fabSize == FabSize::Large) r = dp(28.0f);
    }

    M3Color baseBg = theme.primaryContainer;
    M3Color fg = theme.onPrimaryContainer;

    if (fabType == FabType::Surface) {
        baseBg = theme.surface;
        fg = theme.primary;
    } else if (fabType == FabType::Secondary) {
        baseBg = theme.secondaryContainer;
        fg = theme.onSecondaryContainer;
    }

    // 1. Render Speed Dial sub-action items
    if (m_expandProgress > 0.001f && !speedDialItems.empty()) {
        float ease = 1.0f - std::pow(1.0f - m_expandProgress, 3.0f);

        for (size_t i = 0; i < speedDialItems.size(); ++i) {
            auto& item = speedDialItems[i];
            float rawItemX = 0.0f, rawItemY = 0.0f, rawItemW = 0.0f, rawItemH = 0.0f;
            getSubItemBounds(i, actualW, actualH, ease, rawItemX, rawItemY, rawItemW, rawItemH, &renderer);

            float itemX = std::round(rawItemX);
            float itemY = std::round(rawItemY);
            float itemW = std::round(rawItemW);
            float itemH = std::round(rawItemH);
            float subRadius = itemH * 0.5f;

            M3Color subBg = theme.secondaryContainer;
            M3Color subFg = theme.onSecondaryContainer;
            if (item.type == FabType::Primary) {
                subBg = theme.primaryContainer;
                subFg = theme.onPrimaryContainer;
            } else if (item.type == FabType::Surface) {
                subBg = theme.surfaceContainerHigh;
                subFg = theme.onSurface;
            }

            // Sub-item shadow
            M3Color subShadow = theme.onSurface;
            subShadow.a = 0.12f * ease;
            float subBlur = dp(5.0f);
            renderer.drawM3UI(
                itemX - subBlur, itemY + dp(2.0f) - subBlur, 
                itemW + 2.0f * subBlur, itemH + 2.0f * subBlur,
                subRadius + subBlur, subRadius + subBlur, subRadius + subBlur, subRadius + subBlur,
                subShadow, 0.0f, 0.0f, 0.0f, 0.0f, {0,0,0,0}, 0, 0.0f, {0,0,0,0}, {0,0,0,0}, 0.0f, 0, false, 0.0f, 0.0f, 1.0f, 1.0f, subBlur
            );

            // Sub-item background
            subBg.a *= ease;
            renderer.drawM3UI(itemX, itemY, itemW, itemH, subRadius, subRadius, subRadius, subRadius, subBg);

            // Sub-item hover overlay
            if (item.isHovered) {
                M3Color hoverState = subFg;
                hoverState.a = 0.12f * ease;
                renderer.drawM3UI(itemX, itemY, itemW, itemH, subRadius, subRadius, subRadius, subRadius, hoverState);
            }

            // Render sub-item content (icon and text) centered
            if (item.showTextInButton && !item.label.empty()) {
                float fontSizePx = dp(13.0f);
                float textW = renderer.getTextWidth(item.label, fontSizePx);
                bool hasIcon = !isIconEmpty(item.icon);
                subFg.a *= ease;

                float textCenterY = std::round(itemY + (itemH - fontSizePx) * 0.5f);

                if (hasIcon) {
                    float iconPx = dp(18.0f);
                    float gap = dp(8.0f);
                    float contentW = iconPx + gap + textW;
                    float startX = std::round(itemX + (itemW - contentW) * 0.5f);
                    float iconCenterY = std::round(itemY + (itemH - iconPx) * 0.5f);

                    renderer.drawIcon(startX, iconCenterY, iconPx, item.icon, subFg);
                    float textX = startX + iconPx + gap;
                    renderer.drawText(item.label, textX, textCenterY, fontSizePx, subFg);
                } 
                else {
                    float textX = std::round(itemX + (itemW - textW) * 0.5f);
                    renderer.drawText(item.label, textX, textCenterY, fontSizePx, subFg);
                }
            } 
            else {
                if (!isIconEmpty(item.icon)) {
                    float iconPx = dp(20.0f);
                    subFg.a *= ease;
                    float iconDrawX = std::round(itemX + (itemW - iconPx) * 0.5f);
                    float iconDrawY = std::round(itemY + (itemH - iconPx) * 0.5f);
                    renderer.drawIcon(iconDrawX, iconDrawY, iconPx, item.icon, subFg);
                }
            }
        }
    }

    // 2. Main FAB rendering
    float shadowAlpha = isPressed ? 0.06f : (isHovered ? 0.16f : 0.12f);
    float shadowOffsetY = dp(isPressed ? 1.5f : (isHovered ? 6.0f : 3.0f));
    float shadowBlur = dp(isPressed ? 4.0f : (isHovered ? 14.0f : 8.0f));

    M3Color shadowCol = theme.onSurface;
    shadowCol.a = shadowAlpha;

    float pad = shadowBlur;
    
    renderer.drawM3UI(
        pX - pad, pY + shadowOffsetY - pad, pW + 2.0f * pad, pH + 2.0f * pad,
        r + pad, r + pad, r + pad, r + pad,
        shadowCol,
        0.0f, 0.0f, 0.0f, 0.0f, { 0,0,0,0 },
        0, 0.0f, { 0,0,0,0 }, { 0,0,0,0 }, 0.0f,
        0, false, 0.0f, 0.0f, 1.0f, 1.0f,
        shadowBlur
    );

    renderer.drawM3UI(
        pX, pY, pW, pH,
        r, r, r, r,
        baseBg,
        ripple.localX, ripple.localY, ripple.radius, ripple.alpha, fg,
        0, 0.0f, { 0,0,0,0 },
        (fabType == FabType::Surface) ? theme.outlineVariant : M3Color{ 0,0,0,0 },
        (fabType == FabType::Surface) ? dp(1.0f) : 0.0f
    );

    if (isHovered || isPressed) {
        float overlayAlpha = isPressed ? 0.12f : 0.08f;
        M3Color overlay = fg;
        overlay.a = overlayAlpha;
        renderer.drawM3UI(
            pX, pY, pW, pH,
            r, r, r, r,
            overlay
        );
    }

    float customIconSize = iconSizeDp > 0.0f ? iconSizeDp : getStandardIconSizeForFab(fabSize);
    float iconSizePx = dp(customIconSize);

    if (fabSize == FabSize::Extended && !text.empty()) {
        float fontSizePx = dp(14.0f);
        float textW = renderer.getTextWidth(text, fontSizePx);
        float iconW = isIconEmpty(icon) ? 0.0f : iconSizePx;
        float gap = isIconEmpty(icon) ? 0.0f : dp(12.0f);
        float contentW = textW + iconW + gap;

        float startX = std::round(pX + (pW - contentW) * 0.5f);
        float currentX = startX;

        float iconY = std::round(pY + (pH - iconSizePx) * 0.5f);
        float textY = std::round(pY + (pH - fontSizePx) * 0.5f);

        if (!isIconEmpty(icon)) {
            renderer.drawIcon(currentX, iconY, iconSizePx, icon, fg);
            currentX += iconSizePx + gap;
        }

        renderer.drawText(text, currentX, textY, fontSizePx, fg);
    }
    else {
        if (!isIconEmpty(icon)) {
            float drawX = std::round(pX + (pW - iconSizePx) * 0.5f);
            float drawY = std::round(pY + (pH - iconSizePx) * 0.5f);
            renderer.drawIcon(drawX, drawY, iconSizePx, icon, fg);
        }
    }
}

bool FloatingActionButton::handleMouseMove(float mx, float my) {
    bool handled = View::handleMouseMove(mx, my);
    isHovered = isInside(mx, my);
    isPressed = (state == UIState::Pressed);

    if (m_expandProgress > 0.5f && !speedDialItems.empty()) {
        float actualW = width;
        float actualH = height;
        if (fabSize != FabSize::Extended) {
            actualW = std::min(width, height);
            actualH = actualW;
        }

        for (size_t i = 0; i < speedDialItems.size(); ++i) {
            float itemX = 0.0f, itemY = 0.0f, itemW = 0.0f, itemH = 0.0f;
            getSubItemBounds(i, actualW, actualH, 1.0f, itemX, itemY, itemW, itemH, nullptr);

            bool subInside = (mx >= itemX && mx <= itemX + itemW && my >= itemY && my <= itemY + itemH);
            speedDialItems[i].isHovered = subInside;
            if (subInside) handled = true;
        }
    }

    return handled;
}

bool FloatingActionButton::handleMouseButton(int button, int action, float mx, float my) {
    if (button == 0 && action == 1 && m_expandProgress > 0.5f && !speedDialItems.empty()) {
        float actualW = width;
        float actualH = height;
        if (fabSize != FabSize::Extended) {
            actualW = std::min(width, height);
            actualH = actualW;
        }

        for (size_t i = 0; i < speedDialItems.size(); ++i) {
            float itemX = 0.0f, itemY = 0.0f, itemW = 0.0f, itemH = 0.0f;
            getSubItemBounds(i, actualW, actualH, 1.0f, itemX, itemY, itemW, itemH, nullptr);

            if (mx >= itemX && mx <= itemX + itemW && my >= itemY && my <= itemY + itemH) {
                if (speedDialItems[i].onClick) {
                    speedDialItems[i].onClick();
                }
                collapse();
                return true;
            }
        }
    }

    bool handled = View::handleMouseButton(button, action, mx, my);
    isHovered = isInside(mx, my);
    isPressed = (state == UIState::Pressed);
    return handled;
}

void FloatingActionButton::onClick() {
    if (!speedDialItems.empty()) {
        toggleExpand();
    }
    if (onClickListener) {
        onClickListener();
    }
}

FloatingActionButtonBuilder::FloatingActionButtonBuilder() {}
FloatingActionButtonBuilder& FloatingActionButtonBuilder::text(const std::string& t) { m_text = t; return *this; }
FloatingActionButtonBuilder& FloatingActionButtonBuilder::icon(const Icon& i) { m_icon = i; return *this; }
FloatingActionButtonBuilder& FloatingActionButtonBuilder::icon(const std::string& i) { m_icon = iconFromString(i); return *this; }
FloatingActionButtonBuilder& FloatingActionButtonBuilder::size(FabSize s) { m_size = s; return *this; }
FloatingActionButtonBuilder& FloatingActionButtonBuilder::type(FabType t) { m_type = t; return *this; }
FloatingActionButtonBuilder& FloatingActionButtonBuilder::shape(FabShape s) { m_shape = s; return *this; }
FloatingActionButtonBuilder& FloatingActionButtonBuilder::expandDirection(ExpandDirection dir) { m_expandDirection = dir; return *this; }
FloatingActionButtonBuilder& FloatingActionButtonBuilder::menuAlignment(MenuAlignment align) { m_menuAlignment = align; return *this; }
FloatingActionButtonBuilder& FloatingActionButtonBuilder::showTooltips(bool enable) { m_showTooltips = enable; return *this; }
FloatingActionButtonBuilder& FloatingActionButtonBuilder::iconSize(float sizeDp) { m_iconSizeDp = sizeDp; return *this; }
FloatingActionButtonBuilder& FloatingActionButtonBuilder::position(float customX, float customY) {
    m_hasCustomPosition = true;
    m_customX = customX;
    m_customY = customY;
    return *this;
}
FloatingActionButtonBuilder& FloatingActionButtonBuilder::margins(float left, float top, float right, float bottom) {
    m_marginLeft = left; m_marginTop = top; m_marginRight = right; m_marginBottom = bottom;
    return *this;
}
FloatingActionButtonBuilder& FloatingActionButtonBuilder::onClick(std::function<void()> callback) { m_onClick = callback; return *this; }

FloatingActionButtonBuilder& FloatingActionButtonBuilder::addSubItem(const std::string& label, const Icon& icon, std::function<void()> callback, bool showTextInButton, FabType type) {
    m_subItems.push_back({ label, icon, callback, false, type, showTextInButton, 0.0f, 0.0f });
    return *this;
}

FloatingActionButtonBuilder& FloatingActionButtonBuilder::addSubItem(const std::string& label, const std::string& iconStr, std::function<void()> callback, bool showTextInButton, FabType type) {
    m_subItems.push_back({ label, iconFromString(iconStr), callback, false, type, showTextInButton, 0.0f, 0.0f });
    return *this;
}

void FloatingActionButton::setFloatingAnchorBottomRight(float marginRightDp, float marginBottomDp) {
    m_alignment = FabAlignment::BottomRight;
    m_anchorMarginRight = marginRightDp;
    m_anchorMarginBottom = marginBottomDp;
}

void FloatingActionButton::doLayout(float parentX, float parentY, float parentW, float parentH) {
    if (m_alignment == FabAlignment::BottomRight) {
        float btnW = dp(getPreferredWidth());
        float btnH = dp(getPreferredHeight());

        this->width = btnW;
        this->height = btnH;

        this->x = parentX + parentW - btnW - dp(m_anchorMarginRight);
        this->y = parentY + parentH - btnH - dp(m_anchorMarginBottom);
        return;
    }

    if (m_hasCustomPosition) {
        this->width = dp(getPreferredWidth());
        this->height = dp(getPreferredHeight());
        this->x = m_customX;
        this->y = m_customY;
        return;
    }

    View::doLayout(parentX, parentY, parentW, parentH);
}

FloatingActionButtonBuilder& FloatingActionButtonBuilder::anchorBottomRight(float marginRightDp, float marginBottomDp) {
    m_alignment = FabAlignment::BottomRight;
    m_anchorMarginRight = marginRightDp;
    m_anchorMarginBottom = marginBottomDp;
    return *this;
}

FloatingActionButton* FloatingActionButtonBuilder::build() {
    FloatingActionButton* fab = new FloatingActionButton();
    fab->text = m_text;
    fab->icon = m_icon;
    fab->fabSize = m_size;
    fab->fabType = m_type;
    fab->fabShape = m_shape;
    fab->expandDirection = m_expandDirection;
    fab->menuAlignment = m_menuAlignment;
    fab->showTooltips = m_showTooltips;
    fab->iconSizeDp = m_iconSizeDp;
    
    if (m_alignment == FabAlignment::BottomRight) {
        fab->setFloatingAnchorBottomRight(m_anchorMarginRight, m_anchorMarginBottom);
    } else if (m_hasCustomPosition) {
        fab->setFloatingPosition(m_customX, m_customY);
    }

    fab->margin_left = m_marginLeft;
    fab->margin_top = m_marginTop;
    fab->margin_right = m_marginRight;
    fab->margin_bottom = m_marginBottom;
    fab->onClickListener = m_onClick;
    fab->speedDialItems = m_subItems;
    return fab;
}