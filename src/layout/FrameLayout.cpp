/**
 * @file FrameLayout.cpp
 * @brief Algorithmic implementation of layered frame layout positioning and gravity resolution.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "FrameLayout.hpp"
#include <algorithm>

FrameLayout::FrameLayout() {
    layout_width = MATCH_PARENT;
    layout_height = MATCH_PARENT;
}

float FrameLayout::getPreferredWidth() {
    if (layout_width > 0.0f) {
        return layout_width;
    }

    float maxLeftW = 0.0f;
    float maxRightW = 0.0f;
    float maxCenterW = 0.0f;

    for (auto* child : children) {
        if (!child || child->isGone()) continue;
        float cw = child->getPreferredWidth() + child->margin_left + child->margin_right;
        int grav = child->layout_gravity;

        if (grav & Gravity::RIGHT) {
            if (cw > maxRightW) maxRightW = cw;
        } else if (grav & Gravity::CENTER_HORIZONTAL) {
            if (cw > maxCenterW) maxCenterW = cw;
        } else {
            if (cw > maxLeftW) maxLeftW = cw;
        }
    }

    float totalContentW = (std::max)({ maxLeftW + maxRightW + 32.0f, maxCenterW, maxLeftW, maxRightW });
    return totalContentW + padding_left + padding_right;
}

float FrameLayout::getPreferredHeight() {
    if (layout_height > 0.0f) {
        return layout_height;
    }

    float maxTopH = 0.0f;
    float maxBottomH = 0.0f;
    float maxCenterH = 0.0f;

    for (auto* child : children) {
        if (!child || child->isGone()) continue;
        float ch = child->getPreferredHeight() + child->margin_top + child->margin_bottom;
        int grav = child->layout_gravity;

        if (grav & Gravity::BOTTOM) {
            if (ch > maxBottomH) maxBottomH = ch;
        } else if (grav & Gravity::CENTER_VERTICAL) {
            if (ch > maxCenterH) maxCenterH = ch;
        } else {
            if (ch > maxTopH) maxTopH = ch;
        }
    }

    float totalContentH = (std::max)({ maxTopH + maxBottomH + 48.0f, maxCenterH, maxTopH, maxBottomH });
    return totalContentH + padding_top + padding_bottom;
}

void FrameLayout::doLayout(float parentX, float parentY, float parentW, float parentH) {
    View::doLayout(parentX, parentY, parentW, parentH);

    float innerX = x + dp(padding_left);
    float innerY = y + dp(padding_top);
    float innerW = (std::max)(0.0f, width - dp(padding_left + padding_right));
    float innerH = (std::max)(0.0f, height - dp(padding_top + padding_bottom));

    for (auto* child : children) {
        if (!child || child->isGone()) continue;

        float maxAllowedW = (std::max)(0.0f, innerW - dp(child->margin_left) - dp(child->margin_right));
        float maxAllowedH = (std::max)(0.0f, innerH - dp(child->margin_top) - dp(child->margin_bottom));

        float childW = (child->layout_width == MATCH_PARENT) ? maxAllowedW :
                       (child->layout_width >= 0.0f ? (std::min)(dp(child->layout_width), maxAllowedW) :
                       (std::min)(dp(child->getPreferredWidth()), maxAllowedW));

        float childH = (child->layout_height == MATCH_PARENT) ? maxAllowedH :
                       (child->layout_height >= 0.0f ? (std::min)(dp(child->layout_height), maxAllowedH) :
                       (std::min)(dp(child->getPreferredHeight()), maxAllowedH));

        int grav = child->layout_gravity;
        if (grav == Gravity::NONE) {
            grav = Gravity::LEFT | Gravity::TOP;
        }

        // Horizontal axis alignment
        float childX = innerX + dp(child->margin_left);
        if (grav & Gravity::CENTER_HORIZONTAL) {
            childX = innerX + dp(child->margin_left) + (maxAllowedW - childW) / 2.0f;
        } else if (grav & Gravity::RIGHT) {
            childX = innerX + innerW - childW - dp(child->margin_right);
        }

        // Vertical axis alignment
        float childY = innerY + dp(child->margin_top);
        if (grav & Gravity::CENTER_VERTICAL) {
            childY = innerY + dp(child->margin_top) + (maxAllowedH - childH) / 2.0f;
        } else if (grav & Gravity::BOTTOM) {
            childY = innerY + innerH - childH - dp(child->margin_bottom);
        }

        child->doLayout(childX, childY, childW, childH);
    }
}