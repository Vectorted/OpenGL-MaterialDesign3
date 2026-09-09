/**
 * @file View.cpp
 * @brief Implementation of fundamental UI view hierarchy with continuous subpixel layout.
 * 
 * Implements the View and ViewGroup classes, including layout calculations,
 * event handling, animation updates, and rendering dispatch with viewport culling.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "View.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath> 

float g_dpiScale = 1.0f;

// --- View implementation ---

View::View(float x, float y, float w, float h)
    : x(x), y(y), width(w), height(h), layout_width(w), layout_height(h) {
}

void View::setLayoutParams(float w, float h) { 
    layout_width = w; 
    layout_height = h; 
}

void View::setMargins(float left, float top, float right, float bottom) { 
    margin_left = left; 
    margin_top = top; 
    margin_right = right; 
    margin_bottom = bottom; 
}

void View::setPadding(float left, float top, float right, float bottom) { 
    padding_left = left; 
    padding_top = top; 
    padding_right = right; 
    padding_bottom = bottom; 
}

float View::getPreferredWidth() { 
    return layout_width >= 0.0f ? layout_width : 0.0f; 
}

float View::getPreferredHeight() { 
    return layout_height >= 0.0f ? layout_height : 0.0f; 
}

void View::update(float dt) {
    float targetHover = (state == UIState::Hover) ? 1.0f : 0.0f;
    float targetPress = (state == UIState::Pressed) ? 1.0f : 0.0f;

    float hoverBlend = 1.0f - std::exp(-16.0f * dt);
    float pressBlend = 1.0f - std::exp(-16.0f * dt);

    hoverAnim += (targetHover - hoverAnim) * hoverBlend;
    pressAnim += (targetPress - pressAnim) * pressBlend;

    if (ripple.active) {
        ripple.radius += ripple.maxRadius * 4.5f * dt;
        ripple.alpha -= 2.0f * dt;
        if (ripple.alpha <= 0.001f || ripple.radius >= ripple.maxRadius) {
            ripple.active = false;
        }
    }
}

void View::doLayout(float parentX, float parentY, float parentW, float parentH) {
    if (isGone()) return;

    float wpx = 0.0f;
    if (layout_width == View::MATCH_PARENT) { 
        wpx = std::max(0.0f, parentW - dp(margin_left) - dp(margin_right)); 
    } else if (layout_width == View::WRAP_CONTENT) { 
        wpx = dp(getPreferredWidth()); 
    } else if (layout_width >= 0.0f) { 
        wpx = dp(layout_width); 
    }

    float hpx = 0.0f;
    if (layout_height == View::MATCH_PARENT) { 
        hpx = std::max(0.0f, parentH - dp(margin_top) - dp(margin_bottom)); 
    } else if (layout_height == View::WRAP_CONTENT) { 
        hpx = dp(getPreferredHeight()); 
    } else if (layout_height >= 0.0f) { 
        hpx = dp(layout_height); 
    }

    width = std::min(wpx, std::max(0.0f, parentW - dp(margin_left) - dp(margin_right)));
    height = std::min(hpx, std::max(0.0f, parentH - dp(margin_top) - dp(margin_bottom)));

    x = parentX + dp(margin_left);
    y = parentY + dp(margin_top);
}

void View::render(MaterialShader& renderer, MaterialTheme& theme) {}

bool View::handleMouseMove(float mx, float my) {
    if (!isVisible()) return false;
    if (isInside(mx, my)) {
        if (state != UIState::Pressed) state = UIState::Hover;
        return true;
    } else {
        state = UIState::Normal; 
        return false;
    }
}

bool View::handleMouseButton(int button, int action, float mx, float my) {
    if (!isVisible()) return false;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && isInside(mx, my)) {
            state = UIState::Pressed; 
            ripple.active = true;
            ripple.localX = mx - x - width / 2.0f; 
            ripple.localY = my - y - height / 2.0f;
            ripple.radius = 0.0f; 
            ripple.alpha = 0.12f; 
            ripple.maxRadius = std::max(width, height) * 1.5f;
            return true;
        } else if (action == GLFW_RELEASE && state == UIState::Pressed) {
            bool releasedInside = isInside(mx, my);
            state = releasedInside ? UIState::Hover : UIState::Normal;
            if (releasedInside) onClick();
            return true;
        }
    }
    return false;
}

bool View::handleScroll(float mx, float my, float ox, float oy) { return false; }
bool View::handleKey(int key, int action) { return false; }
bool View::handleChar(unsigned int codepoint) { return false; }
bool View::isInside(float px, float py) { return px >= x && px <= x + width && py >= y && py <= y + height; }

// --- ViewGroup implementation ---

ViewGroup::~ViewGroup() { 
    for (auto* c : children) {
        delete c; 
    }
}

void ViewGroup::addView(View* child) { 
    children.push_back(child); 
}

void ViewGroup::removeView(View* child) { 
    if (child) child->isDestroyed = true; 
}

void ViewGroup::removeAllViews() { 
    for (auto* c : children) {
        c->isDestroyed = true; 
    }
}

void ViewGroup::update(float dt) {
    View::update(dt);
    for (auto it = children.begin(); it != children.end(); ) {
        if ((*it)->isDestroyed) {
            delete *it;
            it = children.erase(it);
        } else {
            (*it)->update(dt);
            ++it;
        }
    }
}

void ViewGroup::doLayout(float parentX, float parentY, float parentW, float parentH) {
    View::doLayout(parentX, parentY, parentW, parentH);
    
    float innerX = x + dp(padding_left);
    float innerY = y + dp(padding_top);
    float innerW = std::max(0.0f, width - dp(padding_left + padding_right));
    float innerH = std::max(0.0f, height - dp(padding_top + padding_bottom));
    
    for (auto* child : children) {
        if (!child || child->isGone()) continue;

        int grav = child->layout_gravity;
        if (grav != Gravity::NONE) {
            float maxAllowedW = std::max(0.0f, innerW - dp(child->margin_left) - dp(child->margin_right));
            float maxAllowedH = std::max(0.0f, innerH - dp(child->margin_top) - dp(child->margin_bottom));

            float childW = (child->layout_width == View::MATCH_PARENT) ? maxAllowedW : 
                           (child->layout_width >= 0.0f ? std::min(dp(child->layout_width), maxAllowedW) : 
                           std::min(dp(child->getPreferredWidth()), maxAllowedW));

            float childH = (child->layout_height == View::MATCH_PARENT) ? maxAllowedH : 
                           (child->layout_height >= 0.0f ? std::min(dp(child->layout_height), maxAllowedH) : 
                           std::min(dp(child->getPreferredHeight()), maxAllowedH));

            float childX = innerX;
            float childY = innerY;

            if (grav & Gravity::CENTER_HORIZONTAL) {
                childX = innerX + (innerW - childW) / 2.0f;
            } else if (grav & Gravity::RIGHT) {
                childX = innerX + innerW - childW - dp(child->margin_right);
            }

            if (grav & Gravity::CENTER_VERTICAL) {
                childY = innerY + (innerH - childH) / 2.0f;
            } else if (grav & Gravity::BOTTOM) {
                childY = innerY + innerH - childH - dp(child->margin_bottom);
            }

            child->doLayout(childX, childY, innerW, innerH);
        } else {
            child->doLayout(innerX, innerY, innerW, innerH);
        }
    }
}

void ViewGroup::render(MaterialShader& renderer, MaterialTheme& theme) {
    View::render(renderer, theme);

    int fbW = 0, fbH = 0;
    GLFWwindow* win = glfwGetCurrentContext();
    if (!win) return;
    glfwGetFramebufferSize(win, &fbW, &fbH);

    // Viewport culling: skip children outside the visible area with a safety margin.
    float safeMargin = dp(150.0f);
    float viewTop = -safeMargin;
    float viewBottom = (float)fbH + safeMargin;
    float viewLeft = -safeMargin;
    float viewRight = (float)fbW + safeMargin;

    for (auto* child : children) {
        if (!child || !child->isVisible()) continue;

        float absoluteChildTop = child->y;
        float absoluteChildBottom = child->y + child->height;
        float absoluteChildLeft = child->x;
        float absoluteChildRight = child->x + child->width;

        if (absoluteChildBottom < viewTop || absoluteChildTop > viewBottom ||
            absoluteChildRight < viewLeft || absoluteChildLeft > viewRight) {
            continue;
        }

        child->render(renderer, theme);
    }
}

bool ViewGroup::handleMouseMove(float mx, float my) {
    if (!isVisible()) return false;
    bool handled = false;
    for (int i = (int)children.size() - 1; i >= 0; --i) {
        if (!children[i]->isVisible()) { 
            children[i]->state = UIState::Normal; 
            continue; 
        }
        if (handled) {
            children[i]->handleMouseMove(-9999.0f, -9999.0f);
        } else if (children[i]->handleMouseMove(mx, my)) {
            handled = true;
        }
    }
    if (isInside(mx, my)) { 
        if (state != UIState::Pressed) state = UIState::Hover; 
    } else { 
        state = UIState::Normal; 
    }
    return handled || (state == UIState::Hover);
}

bool ViewGroup::handleMouseButton(int button, int action, float mx, float my) {
    if (!isVisible()) return false;
    bool handled = false;
    for (int i = (int)children.size() - 1; i >= 0; --i) {
        if (!children[i]->isVisible()) continue;
        if (handled) {
            children[i]->handleMouseButton(button, action, -9999.0f, -9999.0f);
        } else if (children[i]->handleMouseButton(button, action, mx, my)) {
            handled = true;
        }
    }
    if (!handled && button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && isInside(mx, my)) {
            state = UIState::Pressed; 
            ripple.active = true;
            ripple.localX = mx - x - width / 2.0f; 
            ripple.localY = my - y - height / 2.0f;
            ripple.radius = 0.0f; 
            ripple.alpha = 0.12f; 
            ripple.maxRadius = std::max(width, height) * 1.5f;
            handled = true;
        } else if (action == GLFW_RELEASE && state == UIState::Pressed) {
            bool releasedInside = isInside(mx, my); 
            state = releasedInside ? UIState::Hover : UIState::Normal;
            if (releasedInside) onClick(); 
            handled = true;
        }
    }
    return handled;
}

bool ViewGroup::handleScroll(float mx, float my, float ox, float oy) {
    if (!isVisible()) return false;
    for (int i = (int)children.size() - 1; i >= 0; --i) {
        if (children[i]->isVisible() && children[i]->handleScroll(mx, my, ox, oy)) {
            return true;
        }
    } 
    return false;
}

bool ViewGroup::handleKey(int key, int action) {
    if (!isVisible()) return false;
    for (int i = (int)children.size() - 1; i >= 0; --i) {
        if (children[i]->isVisible() && children[i]->handleKey(key, action)) {
            return true;
        }
    } 
    return false;
}

bool ViewGroup::handleChar(unsigned int codepoint) {
    if (!isVisible()) return false;
    for (int i = (int)children.size() - 1; i >= 0; --i) {
        if (children[i]->isVisible() && children[i]->handleChar(codepoint)) {
            return true;
        }
    } 
    return false;
}
