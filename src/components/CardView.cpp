/**
 * @file CardView.cpp
 * @brief Implementation of CardView layout calculations, elevation projections, and rendering passes.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "CardView.hpp"
#include <algorithm>

CardView::CardView() {
    layout_width = MATCH_PARENT;
    layout_height = MATCH_PARENT;

    margin_left = 0.0f;
    margin_right = 0.0f;
    margin_top = 0.0f;
    margin_bottom = 0.0f;

    padding_left = 8.0f;
    padding_right = 8.0f;
    padding_top = 8.0f;
    padding_bottom = 8.0f;

    corner_radius = 12.0f;
    interactive = false; 
}

void CardView::addView(View* child) {
    if (child != nullptr) {
        children.push_back(child);
    }
}

void CardView::setCornerRadius(float radius) {
    corner_radius = radius;
}

float CardView::getPreferredWidth() {
    float maxChildW = 0.0f;
    for (auto* child : children) {
        float cw = child->getPreferredWidth() + child->margin_left + child->margin_right;
        if (cw > maxChildW) maxChildW = cw;
    }
    return maxChildW + padding_left + padding_right;
}

float CardView::getPreferredHeight() {
    float maxChildH = 0.0f;
    for (auto* child : children) {
        float ch = child->getPreferredHeight() + child->margin_top + child->margin_bottom;
        if (ch > maxChildH) maxChildH = ch;
    }
    return maxChildH + padding_top + padding_bottom;
}

void CardView::render(MaterialShader& renderer, MaterialTheme& theme) {
    float r = dp(corner_radius);

    M3Color cardColor = theme.surface;
    M3Color borderColor = { 0,0,0,0 };
    float borderW = 0.0f;

    if (card_type == CardType::Filled) {
        cardColor = theme.surfaceVariant;
    }
    else if (card_type == CardType::Outlined) {
        cardColor = theme.surface;
        borderColor = theme.outlineVariant;
        borderW = dp(1.0f);
    }

    if (card_type == CardType::Elevated) {
        bool isPressed = interactive && (state == UIState::Pressed);
        bool isHovered = interactive && (state == UIState::Hover);

        float shadowAlpha = isPressed ? 0.06f : (isHovered ? 0.16f : 0.12f);
        float shadowOffsetY = dp(isPressed ? 1.0f : (isHovered ? 5.0f : 2.0f));
        float shadowBlur = dp(isPressed ? 3.0f : (isHovered ? 12.0f : 6.0f));

        M3Color shadowCol = theme.onSurface;
        shadowCol.a = shadowAlpha;

        float pad = shadowBlur;
        renderer.drawM3UI(
            x - pad, y + shadowOffsetY - pad, width + 2.0f * pad, height + 2.0f * pad,
            r + pad, r + pad, r + pad, r + pad,
            shadowCol,
            0.0f, 0.0f, 0.0f, 0.0f, { 0,0,0,0 },
            0, 0.0f, { 0,0,0,0 }, { 0,0,0,0 }, 0.0f,
            0, false, 0.0f, 0.0f, 1.0f, 1.0f,
            shadowBlur
        );
    }

    renderer.drawM3UI(
        x, y, width, height,
        r, r, r, r,
        cardColor,
        0.0f, 0.0f, 0.0f, 0.0f, { 0,0,0,0 },
        0, 0.0f, { 0,0,0,0 },
        borderColor, borderW
    );

    if (interactive && (state == UIState::Hover || state == UIState::Pressed)) {
        float overlayAlpha = (state == UIState::Pressed) ? 0.12f : 0.08f;
        M3Color overlay = theme.onSurface;
        overlay.a = overlayAlpha;
        renderer.drawM3UI(
            x, y, width, height,
            r, r, r, r,
            overlay
        );
    }

    ViewGroup::render(renderer, theme);
}

CardViewBuilder& CardViewBuilder::layoutWidth(float w) { m_width = w; return *this; }
CardViewBuilder& CardViewBuilder::layoutHeight(float h) { m_height = h; return *this; }
CardViewBuilder& CardViewBuilder::margins(float left, float top, float right, float bottom) {
    m_marginLeft = left; m_marginTop = top; m_marginRight = right; m_marginBottom = bottom;
    return *this;
}
CardViewBuilder& CardViewBuilder::padding(float left, float top, float right, float bottom) {
    m_paddingLeft = left; m_paddingTop = top; m_paddingRight = right; m_paddingBottom = bottom;
    return *this;
}
CardViewBuilder& CardViewBuilder::cornerRadius(float radius) { m_cornerRadius = radius; return *this; }
CardViewBuilder& CardViewBuilder::type(CardType t) { m_type = t; return *this; }
CardViewBuilder& CardViewBuilder::interactive(bool val) { m_interactive = val; return *this; }

CardView* CardViewBuilder::build() {
    CardView* cv = new CardView();
    cv->layout_width = m_width;
    cv->layout_height = m_height;
    cv->margin_left = m_marginLeft;
    cv->margin_top = m_marginTop;
    cv->margin_right = m_marginRight;
    cv->margin_bottom = m_marginBottom;
    cv->padding_left = m_paddingLeft;
    cv->padding_top = m_paddingTop;
    cv->padding_right = m_paddingRight;
    cv->padding_bottom = m_paddingBottom;
    cv->corner_radius = m_cornerRadius;
    cv->card_type = m_type;
    cv->interactive = m_interactive; 
    return cv;
}