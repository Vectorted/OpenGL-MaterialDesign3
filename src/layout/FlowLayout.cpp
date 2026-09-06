/**
 * @file FlowLayout.cpp
 * @brief Algorithmic implementation of multi-row break measurement and alignment in FlowLayout.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "FlowLayout.hpp"
#include <algorithm>
#include <vector>

FlowLayout::FlowLayout(float hGap, float vGap)
    : horizontalGapDp(hGap), verticalGapDp(vGap) {
    layout_width = MATCH_PARENT;
    layout_height = WRAP_CONTENT;
}

float FlowLayout::getPreferredWidth() {
    float totalW = padding_left + padding_right;
    int count = 0;
    for (auto* child : children) {
        if (!child || child->isGone()) continue;
        totalW += child->getPreferredWidth() + child->margin_left + child->margin_right;
        count++;
    }
    if (count > 1) totalW += (count - 1) * horizontalGapDp;
    return totalW;
}

float FlowLayout::getPreferredHeight() {
    float maxLineWidth = (layout_width > 0.0f) ? layout_width : 400.0f;
    float availW = maxLineWidth - padding_left - padding_right;

    float currentLineW = 0.0f;
    float currentLineH = 0.0f;
    float totalH = padding_top + padding_bottom;
    bool hasLine = false;

    for (auto* child : children) {
        if (!child || child->isGone()) continue;

        float cw = child->getPreferredWidth() + child->margin_left + child->margin_right;
        float ch = child->getPreferredHeight() + child->margin_top + child->margin_bottom;

        if (hasLine && currentLineW + horizontalGapDp + cw > availW) {
            totalH += currentLineH + verticalGapDp;
            currentLineW = cw;
            currentLineH = ch;
        } else {
            currentLineW += (hasLine ? horizontalGapDp : 0.0f) + cw;
            currentLineH = (std::max)(currentLineH, ch);
            hasLine = true;
        }
    }

    if (hasLine) totalH += currentLineH;
    return totalH;
}

void FlowLayout::doLayout(float parentX, float parentY, float parentW, float parentH) {
    View::doLayout(parentX, parentY, parentW, parentH);

    float innerX = x + dp(padding_left);
    float innerY = y + dp(padding_top);
    float availW = (std::max)(0.0f, width - dp(padding_left + padding_right));

    struct LineInfo {
        std::vector<View*> items;
        float width = 0.0f;
        float height = 0.0f;
    };

    std::vector<LineInfo> lines;
    LineInfo currentLine;

    for (auto* child : children) {
        if (!child || child->isGone()) continue;

        float cw = dp(child->getPreferredWidth()) + dp(child->margin_left + child->margin_right);
        float ch = dp(child->getPreferredHeight()) + dp(child->margin_top + child->margin_bottom);

        if (!currentLine.items.empty() && currentLine.width + dp(horizontalGapDp) + cw > availW) {
            lines.push_back(currentLine);
            currentLine = LineInfo();
        }

        currentLine.width += (currentLine.items.empty() ? 0.0f : dp(horizontalGapDp)) + cw;
        currentLine.height = (std::max)(currentLine.height, ch);
        currentLine.items.push_back(child);
    }

    if (!currentLine.items.empty()) {
        lines.push_back(currentLine);
    }

    float currentY = innerY;
    for (const auto& line : lines) {
        float startX = innerX;

        if (gravity & Gravity::CENTER_HORIZONTAL) {
            startX = innerX + (availW - line.width) / 2.0f;
        } else if (gravity & Gravity::RIGHT) {
            startX = innerX + availW - line.width;
        }

        float curX = startX;
        for (auto* child : line.items) {
            float childW = dp(child->getPreferredWidth());
            float childH = dp(child->getPreferredHeight());

            float childX = curX + dp(child->margin_left);
            float childY = currentY + dp(child->margin_top) + (line.height - childH - dp(child->margin_top + child->margin_bottom)) / 2.0f;

            child->doLayout(childX, childY, childW, childH);
            curX += dp(child->margin_left) + childW + dp(child->margin_right) + dp(horizontalGapDp);
        }

        currentY += line.height + dp(verticalGapDp);
    }
}