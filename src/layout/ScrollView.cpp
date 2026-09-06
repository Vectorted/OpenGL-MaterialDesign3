/**
 * @file ScrollView.cpp
 * @brief Implementation of hardware-accelerated scissored viewport scrolling with exact integer-pixel stepping.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "ScrollView.hpp"
#include "MaterialShader.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <array>
#include <vector>

/**
 * @brief External event hook to request window frame redraw wakeups during active animations.
 */
extern void requestUIWakeup(double seconds);

/**
 * @brief Hierarchical stack of nested OpenGL scissor clipping rectangles in screen coordinates.
 */
static std::vector<std::array<int, 4>> s_scissorStack;

void ScrollView::clearScissorStack() {
    s_scissorStack.clear();
}

ScrollView::ScrollView() {
    layout_width = MATCH_PARENT; 
    layout_height = MATCH_PARENT;
    padding_left = 0.0f; 
    padding_top = 0.0f; 
    padding_right = 0.0f; 
    padding_bottom = 0.0f;
}

float ScrollView::getPreferredWidth() {
    if (children.empty() || children[0]->isGone()) return 0.0f;
    return children[0]->getPreferredWidth() + padding_left + padding_right;
}

float ScrollView::getPreferredHeight() {
    if (children.empty() || children[0]->isGone()) return 0.0f;
    return children[0]->getPreferredHeight() + padding_top + padding_bottom;
}

void ScrollView::syncChildrenScrollPositions() {
    if (children.empty() || children[0]->isGone()) return;

    View* child = children[0];
    float innerWidth = std::round(width - dp(padding_left + padding_right));
    float innerHeight = std::round(height - dp(padding_top + padding_bottom));

    float childW = (child->layout_width == MATCH_PARENT) ? innerWidth : std::min(std::round(dp(child->getPreferredWidth())), innerWidth);
    float childH = std::round(dp(child->getPreferredHeight()));
    if (child->layout_height == MATCH_PARENT) childH = std::max(childH, innerHeight);

    maxScrollY = std::max(0.0f, std::round(childH - innerHeight));
    targetScrollY = std::clamp(targetScrollY, 0.0f, maxScrollY);

    float effectiveScrollY = std::round(scrollY);
    float childX = std::round(x + dp(padding_left) + dp(child->margin_left));
    float childY = std::round(y + dp(padding_top) + dp(child->margin_top) - effectiveScrollY);

    child->doLayout(childX, childY, childW, childH);
}

void ScrollView::doLayout(float parentX, float parentY, float parentW, float parentH) {
    View::doLayout(parentX, parentY, parentW, parentH);
    syncChildrenScrollPositions();
}

void ScrollView::update(float dt) {
    ViewGroup::update(dt);

    const float dt_safe = std::min(dt, 0.033f);
    const float diff = targetScrollY - scrollY;

    if (std::abs(diff) >= 0.05f) {
        const float factor = 1.0f - std::exp(-24.0f * dt_safe);
        scrollY += diff * factor;
        
        syncChildrenScrollPositions();
        requestUIWakeup(0.016);
    } else if (scrollY != targetScrollY) {
        scrollY = targetScrollY;
        syncChildrenScrollPositions();
    }
}

void ScrollView::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (width <= 0.0f || height <= 0.0f) {
        ViewGroup::render(renderer, theme);
        return;
    }

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    GLFWwindow* win = glfwGetCurrentContext();
    if (!win) return;
    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(win, &fbW, &fbH);

    int ry = (int)std::round(this->y);
    int rh = (int)std::round(this->height);
    int sx = (int)std::round(this->x);
    int sy = fbH - (ry + rh); 
    int sw = (int)std::round(this->width);
    int sh = rh;

    std::array<int, 4> new_box = { sx, sy, sw, sh };

    glEnable(GL_SCISSOR_TEST);

    if (!s_scissorStack.empty()) {
        const auto& parent_box = s_scissorStack.back();
        int cx = std::max((int)new_box[0], (int)parent_box[0]);
        int cy = std::max((int)new_box[1], (int)parent_box[1]);
        int cw = std::max(0, std::min((int)(new_box[0] + new_box[2]), (int)(parent_box[0] + parent_box[2])) - cx);
        int ch = std::max(0, std::min((int)(new_box[1] + new_box[3]), (int)(parent_box[1] + parent_box[3])) - cy);
        new_box = { cx, cy, cw, ch };
    }

    s_scissorStack.push_back(new_box);
    glScissor(new_box[0], new_box[1], new_box[2], new_box[3]);

    renderer.drawM3UI(std::round(this->x), std::round(this->y), std::round(this->width), std::round(this->height), 0.0f, 0.0f, 0.0f, 0.0f, theme.surface);

    ViewGroup::render(renderer, theme);

    s_scissorStack.pop_back();

    if (!s_scissorStack.empty()) {
        const auto& parent_box = s_scissorStack.back();
        glScissor(parent_box[0], parent_box[1], parent_box[2], parent_box[3]);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}

bool ScrollView::handleMouseMove(float mx, float my) {
    if (!isVisible()) return false;

    GLFWwindow* win = glfwGetCurrentContext();
    bool isMouseDown = (win && glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

    if (!isInside(mx, my) && !isMouseDown) {
        for (auto* child : children) {
            if (child) child->handleMouseMove(-9999.0f, -9999.0f);
        }
        state = UIState::Normal;
        return false;
    }
    return ViewGroup::handleMouseMove(mx, my);
}

bool ScrollView::handleMouseButton(int button, int action, float mx, float my) {
    if (!isVisible()) return false;
    if (action == GLFW_RELEASE) {
        return ViewGroup::handleMouseButton(button, action, mx, my);
    }
    if (!isInside(mx, my)) return false;
    return ViewGroup::handleMouseButton(button, action, mx, my);
}

bool ScrollView::handleScroll(float mx, float my, float ox, float oy) {
    if (!isVisible() || !isInside(mx, my)) return false;
    if (ViewGroup::handleScroll(mx, my, ox, oy)) return true;

    if (maxScrollY > 0.0f) {
        targetScrollY -= oy * dp(56.0f);
        targetScrollY = std::clamp(targetScrollY, 0.0f, maxScrollY);
        requestUIWakeup(0.016);
        return true;
    }
    return false;
}