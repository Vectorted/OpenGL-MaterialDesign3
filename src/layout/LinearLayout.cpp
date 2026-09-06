/**
 * @file LinearLayout.cpp
 * @brief Algorithmic implementation of linear flex container layout positioning.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "LinearLayout.hpp"
#include <algorithm>
#include <vector>

LinearLayout::LinearLayout(Orientation orient) : orientation(orient) {
    layout_width = MATCH_PARENT;
    layout_height = MATCH_PARENT;
}

float LinearLayout::getPreferredWidth() {
    if (orientation == Orientation::VERTICAL) {
        float maxW = 0.0f;
        for (auto* child : children) {
            if (!child || child->isGone()) continue; 
            float cw = child->getPreferredWidth() + child->margin_left + child->margin_right;
            if (cw > maxW) maxW = cw;
        }
        return maxW + padding_left + padding_right;
    } else {
        float w = padding_left + padding_right;
        int validCount = 0;
        for (auto* child : children) {
            if (!child || child->isGone()) continue;
            validCount++;
            w += child->getPreferredWidth() + child->margin_left + child->margin_right;
        }
        if (validCount > 1) w += (validCount - 1) * gapDp;
        return w;
    }
}

float LinearLayout::getPreferredHeight() {
    if (orientation == Orientation::VERTICAL) {
        float h = padding_top + padding_bottom;
        int validCount = 0;
        for (auto* child : children) {
            if (!child || child->isGone()) continue;
            validCount++;
            h += child->getPreferredHeight() + child->margin_top + child->margin_bottom;
        }
        if (validCount > 1) h += (validCount - 1) * gapDp;
        return h;
    } else {
        float maxH = 0.0f;
        for (auto* child : children) {
            if (!child || child->isGone()) continue;
            float ch = child->getPreferredHeight() + child->margin_top + child->margin_bottom;
            if (ch > maxH) maxH = ch;
        }
        return maxH + padding_top + padding_bottom;
    }
}

void LinearLayout::doLayout(float parentX, float parentY, float parentW, float parentH) {
    View::doLayout(parentX, parentY, parentW, parentH);

    std::vector<View*> validChildren;
    validChildren.reserve(children.size());
    for (auto* child : children) {
        if (child && !child->isGone()) validChildren.push_back(child);
    }

    if (validChildren.empty()) return;

    if (orientation == Orientation::VERTICAL) {
        float totalGaps = (validChildren.size() > 1) ? (validChildren.size() - 1) * dp(gapDp) : 0.0f;
        float availableHeight = height - dp(padding_top + padding_bottom);

        float totalDesiredFixedH = 0.0f;
        int fillHeightCount = 0;
        for (auto* child : validChildren) {
            if (child->layout_height == MATCH_PARENT) {
                fillHeightCount++;
            } else {
                totalDesiredFixedH += dp(child->getPreferredHeight()) + dp(child->margin_top) + dp(child->margin_bottom);
            }
        }

        float scaleFactor = 1.0f;
        float totalNeededWithoutFill = totalDesiredFixedH + totalGaps;
        if (availableHeight < totalNeededWithoutFill && totalNeededWithoutFill > 0.0f) {
            scaleFactor = availableHeight / totalNeededWithoutFill;
        }

        float remainingH = availableHeight - (totalDesiredFixedH * scaleFactor) - (totalGaps * scaleFactor);
        float distH = (fillHeightCount > 0) ? (remainingH / fillHeightCount) : 0.0f;
        if (distH < 0.0f) distH = 0.0f;

        float currentY = y + dp(padding_top);
        
        if (fillHeightCount == 0 && remainingH > 0.0f) {
            if (gravity & Gravity::CENTER_VERTICAL) {
                currentY += remainingH / 2.0f;
            } else if (gravity & Gravity::BOTTOM) {
                currentY += remainingH;
            }
        }

        float innerWidth = width - dp(padding_left + padding_right);

        for (auto* child : validChildren) {
            float maxAllowedChildW = innerWidth - dp(child->margin_left) - dp(child->margin_right);
            if (maxAllowedChildW < 0.0f) maxAllowedChildW = 0.0f;

            float childW = (child->layout_width == MATCH_PARENT) ? maxAllowedChildW : 
                           (child->layout_width >= 0.0f ? std::min(dp(child->layout_width), maxAllowedChildW) : 
                           std::min(dp(child->getPreferredWidth()), maxAllowedChildW));

            float childH = 0.0f;
            if (child->layout_height == MATCH_PARENT) {
                childH = distH;
            } else {
                childH = dp(child->getPreferredHeight()) * scaleFactor;
            }

            int crossGrav = (child->layout_gravity != Gravity::NONE) ? child->layout_gravity : gravity;
            float childX = x + dp(padding_left) + dp(child->margin_left);

            if (crossGrav & Gravity::CENTER_HORIZONTAL) {
                childX = x + dp(padding_left) + (innerWidth - childW) / 2.0f;
            } else if (crossGrav & Gravity::RIGHT) {
                childX = x + dp(padding_left) + innerWidth - childW - dp(child->margin_right);
            }

            float childY = currentY;
            child->doLayout(childX, childY, childW, childH);

            currentY += (dp(child->margin_top) + childH + dp(child->margin_bottom) + dp(gapDp)) * scaleFactor;
        }
    } else {
        float totalGaps = (validChildren.size() > 1) ? (validChildren.size() - 1) * dp(gapDp) : 0.0f;
        float availableWidth = width - dp(padding_left + padding_right);

        float totalDesiredFixedW = 0.0f;
        int fillWidthCount = 0;
        for (auto* child : validChildren) {
            if (child->layout_width == MATCH_PARENT) {
                fillWidthCount++;
            } else {
                totalDesiredFixedW += dp(child->getPreferredWidth()) + dp(child->margin_left) + dp(child->margin_right);
            }
        }

        float scaleFactor = 1.0f;
        float totalNeededWithoutFill = totalDesiredFixedW + totalGaps;
        if (availableWidth < totalNeededWithoutFill && totalNeededWithoutFill > 0.0f) {
            scaleFactor = availableWidth / totalNeededWithoutFill;
        }

        float remainingW = availableWidth - (totalDesiredFixedW * scaleFactor) - (totalGaps * scaleFactor);
        float distW = (fillWidthCount > 0) ? (remainingW / fillWidthCount) : 0.0f;
        if (distW < 0.0f) distW = 0.0f;

        float currentX = x + dp(padding_left);

        if (fillWidthCount == 0 && remainingW > 0.0f) {
            if (gravity & Gravity::CENTER_HORIZONTAL) {
                currentX += remainingW / 2.0f;
            } else if (gravity & Gravity::RIGHT) {
                currentX += remainingW;
            }
        }

        float innerHeight = height - dp(padding_top + padding_bottom);

        for (auto* child : validChildren) {
            float maxAllowedChildH = innerHeight - dp(child->margin_top) - dp(child->margin_bottom);
            if (maxAllowedChildH < 0.0f) maxAllowedChildH = 0.0f;

            float childH = (child->layout_height == MATCH_PARENT) ? maxAllowedChildH : 
                           (child->layout_height >= 0.0f ? std::min(dp(child->layout_height), maxAllowedChildH) : 
                           std::min(dp(child->getPreferredHeight()), maxAllowedChildH));

            float childW = 0.0f;
            if (child->layout_width == MATCH_PARENT) {
                childW = distW;
            } else {
                childW = dp(child->getPreferredWidth()) * scaleFactor;
            }

            int crossGrav = (child->layout_gravity != Gravity::NONE) ? child->layout_gravity : gravity;
            float childY = y + dp(padding_top) + (innerHeight - childH) / 2.0f;

            if (crossGrav & Gravity::TOP) {
                childY = y + dp(padding_top) + dp(child->margin_top);
            } else if (crossGrav & Gravity::BOTTOM) {
                childY = y + dp(padding_top) + innerHeight - childH - dp(child->margin_bottom);
            }

            float childX = currentX;
            child->doLayout(childX, childY, childW, childH);

            currentX += (dp(child->margin_left) + childW + dp(child->margin_right) + dp(gapDp)) * scaleFactor;
        }
    }
}