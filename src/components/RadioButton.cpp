/**
 * @file RadioButton.cpp
 * @brief Implementation of RadioButton selection kinetics, ring rendering, and input dispatching.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "RadioButton.hpp"
#include <cmath>
#include <GLFW/glfw3.h>

/**
 * @brief Helper function to linearly interpolate between two M3Colors.
 */
static M3Color mixColor(const M3Color& a, const M3Color& b, float t) {
    return {
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t
    };
}

RadioButton::RadioButton(std::string text, bool act) : label(text), checked(act) {
    checkAnim = act ? 1.0f : 0.0f;
    layout_height = 40.0f;
}

float RadioButton::getPreferredWidth() {
    return layout_width > 0 ? layout_width : 48.0f + (label.empty() ? 0.0f : label.length() * 9.0f);
}

void RadioButton::update(float dt) {
    View::update(dt);
    checkAnim += ((checked ? 1.0f : 0.0f) - checkAnim) * (1.0f - std::exp(-18.0f * dt));
}

void RadioButton::render(MaterialShader& renderer, MaterialTheme& theme) {
    float outer = dp(10.0f);
    float cx = x + dp(12.0f) + outer;
    float cy = y + height / 2.0f;

    // Render hover halo
    if (state == UIState::Hover || hoverAnim > 0.01f) {
        float hr = dp(20.0f) * hoverAnim;
        renderer.drawM3UI(cx - hr, cy - hr, hr * 2, hr * 2, hr, hr, hr, hr, theme.surfaceVariant);
    }
    
    // Render ripple animation
    if (ripple.active) {
        renderer.drawM3UI(cx - ripple.radius, cy - ripple.radius, ripple.radius * 2, ripple.radius * 2,
            ripple.radius, ripple.radius, ripple.radius, ripple.radius, theme.primaryContainer);
    }

    M3Color ringColor = mixColor(theme.onSurfaceVariant, theme.primary, checkAnim);

    // Outer circle
    renderer.drawM3UI(cx - outer, cy - outer, outer * 2, outer * 2, outer, outer, outer, outer, ringColor);
    
    // Inner surface fill
    float innerR = outer - dp(2.0f);
    renderer.drawM3UI(cx - innerR, cy - innerR, innerR * 2, innerR * 2, innerR, innerR, innerR, innerR, theme.surface);

    // Active inner dot
    float dotR = dp(5.0f) * checkAnim;
    if (dotR > 0.1f) {
        renderer.drawM3UI(cx - dotR, cy - dotR, dotR * 2, dotR * 2, dotR, dotR, dotR, dotR, theme.primary);
    }

    // Label text
    if (!label.empty()) {
        renderer.drawText(label, cx + outer + dp(12.0f), cy - dp(14.0f) * 0.45f, dp(14.0f), theme.onSurface);
    }
}

bool RadioButton::handleMouseButton(int button, int action, float mx, float my) {
    if (!isVisible()) return false;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && isInside(mx, my)) {
            state = UIState::Pressed;
            ripple.active = true; 
            ripple.radius = 0.0f; 
            ripple.maxRadius = dp(24.0f);
            return true;
        }
        else if (action == GLFW_RELEASE && state == UIState::Pressed) {
            state = isInside(mx, my) ? UIState::Hover : UIState::Normal;
            
            if (isInside(mx, my) && !checked) {
                checked = true;
                if (onCheckedChange) {
                    onCheckedChange(checked);
                }
            }
            return true;
        }
    }
    return false;
}