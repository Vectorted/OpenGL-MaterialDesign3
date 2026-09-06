/**
 * @file RelativeLayout.cpp
 * @brief Algorithmic implementation of two-pass dependency resolution for relative positioning.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "RelativeLayout.hpp"
#include <algorithm>

RelativeLayout::RelativeLayout() {
    layout_width = MATCH_PARENT;
    layout_height = MATCH_PARENT;
}

RelativeLayout* RelativeLayout::addRule(View* child, RelativeRule rule) {
    return addRule(child, rule, nullptr);
}

RelativeLayout* RelativeLayout::addRule(View* child, RelativeRule rule, View* anchor) {
    if (child) {
        m_childParams[child].rules[rule] = anchor;
    }
    return this;
}

float RelativeLayout::getPreferredWidth() {
    float maxRight = 0.0f;
    for (auto* child : children) {
        if (!child || child->isGone()) continue;
        float cw = child->getPreferredWidth() + child->margin_left + child->margin_right;
        if (cw > maxRight) maxRight = cw;
    }
    return maxRight + padding_left + padding_right;
}

float RelativeLayout::getPreferredHeight() {
    float maxBottom = 0.0f;
    for (auto* child : children) {
        if (!child || child->isGone()) continue;
        float ch = child->getPreferredHeight() + child->margin_top + child->margin_bottom;
        if (ch > maxBottom) maxBottom = ch;
    }
    return maxBottom + padding_top + padding_bottom;
}

void RelativeLayout::doLayout(float parentX, float parentY, float parentW, float parentH) {
    View::doLayout(parentX, parentY, parentW, parentH);

    float innerX = x + dp(padding_left);
    float innerY = y + dp(padding_top);
    float innerW = (std::max)(0.0f, width - dp(padding_left + padding_right));
    float innerH = (std::max)(0.0f, height - dp(padding_top + padding_bottom));

    // Two-pass solver to ensure sibling dependencies (e.g. Below, RightOf) are satisfied
    for (int pass = 0; pass < 2; ++pass) {
        for (auto* child : children) {
            if (!child || child->isGone()) continue;

            float childW = (child->layout_width == MATCH_PARENT) ? innerW :
                           (child->layout_width >= 0.0f ? dp(child->layout_width) : dp(child->getPreferredWidth()));
            float childH = (child->layout_height == MATCH_PARENT) ? innerH :
                           (child->layout_height >= 0.0f ? dp(child->layout_height) : dp(child->getPreferredHeight()));

            float childX = innerX + dp(child->margin_left);
            float childY = innerY + dp(child->margin_top);

            auto it = m_childParams.find(child);
            if (it != m_childParams.end()) {
                const auto& rules = it->second.rules;

                // Parent alignment rules
                if (rules.find(RelativeRule::AlignParentRight) != rules.end()) {
                    childX = innerX + innerW - childW - dp(child->margin_right);
                }
                if (rules.find(RelativeRule::AlignParentBottom) != rules.end()) {
                    childY = innerY + innerH - childH - dp(child->margin_bottom);
                }
                if (rules.find(RelativeRule::CenterHorizontal) != rules.end() || rules.find(RelativeRule::CenterInParent) != rules.end()) {
                    childX = innerX + (innerW - childW) / 2.0f;
                }
                if (rules.find(RelativeRule::CenterVertical) != rules.end() || rules.find(RelativeRule::CenterInParent) != rules.end()) {
                    childY = innerY + (innerH - childH) / 2.0f;
                }

                // Sibling relationship rules
                auto rBelow = rules.find(RelativeRule::Below);
                if (rBelow != rules.end() && rBelow->second) {
                    childY = rBelow->second->y + rBelow->second->height + dp(child->margin_top);
                }

                auto rAbove = rules.find(RelativeRule::Above);
                if (rAbove != rules.end() && rAbove->second) {
                    childY = rAbove->second->y - childH - dp(child->margin_bottom);
                }

                auto rRightOf = rules.find(RelativeRule::RightOf);
                if (rRightOf != rules.end() && rRightOf->second) {
                    childX = rRightOf->second->x + rRightOf->second->width + dp(child->margin_left);
                }

                auto rLeftOf = rules.find(RelativeRule::LeftOf);
                if (rLeftOf != rules.end() && rLeftOf->second) {
                    childX = rLeftOf->second->x - childW - dp(child->margin_right);
                }

                auto rAlignLeft = rules.find(RelativeRule::AlignLeft);
                if (rAlignLeft != rules.end() && rAlignLeft->second) {
                    childX = rAlignLeft->second->x + dp(child->margin_left);
                }

                auto rAlignTop = rules.find(RelativeRule::AlignTop);
                if (rAlignTop != rules.end() && rAlignTop->second) {
                    childY = rAlignTop->second->y + dp(child->margin_top);
                }
            }

            child->doLayout(childX, childY, childW, childH);
        }
    }
}