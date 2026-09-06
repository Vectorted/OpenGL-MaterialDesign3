/**
 * @file Checkbox.cpp
 * @brief Implementation of Checkbox layout, interaction handling, and animated rendering.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "Checkbox.hpp"
#include <cmath>
#include <GLFW/glfw3.h>
#include <algorithm>

/**
 * @brief Linearly interpolates between two RGBA colors.
 * 
 * @param a Start color.
 * @param b Target color.
 * @param t Interpolation progress factor.
 * @return M3Color Interpolated RGBA result.
 */
static M3Color mixColor(const M3Color& a, const M3Color& b, float t) {
    return {
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t
    };
}

Checkbox::Checkbox(std::string text, bool act) : label(std::move(text)), checked(act) {
    checkAnim = act ? 1.0f : 0.0f;
    layout_height = 40.0f;
}

float Checkbox::getPreferredWidth() {
    return layout_width > 0 ? layout_width : 48.0f + (label.empty() ? 0.0f : label.length() * 9.0f);
}

void Checkbox::update(float dt) {
    View::update(dt);
    float target = checked ? 1.0f : 0.0f;
    checkAnim += (target - checkAnim) * (1.0f - std::exp(-18.0f * dt));
}

void Checkbox::render(MaterialShader& renderer, MaterialTheme& theme) {
    float sz = dp(20.0f);
    float boxX = x + dp(12.0f);
    float boxY = y + height / 2.0f - sz / 2.0f;
    float cx = boxX + sz / 2.0f;
    float cy = boxY + sz / 2.0f;

    if (state == UIState::Hover || hoverAnim > 0.01f) {
        float hr = dp(20.0f) * hoverAnim;
        renderer.drawM3UI(cx - hr, cy - hr, hr * 2, hr * 2, hr, hr, hr, hr, theme.surfaceVariant);
    }
    if (ripple.active) {
        renderer.drawM3UI(cx - ripple.radius, cy - ripple.radius, ripple.radius * 2, ripple.radius * 2,
            ripple.radius, ripple.radius, ripple.radius, ripple.radius, theme.primaryContainer);
    }

    M3Color borderColor = mixColor(theme.onSurfaceVariant, theme.primary, checkAnim);
    M3Color innerColor = mixColor(theme.surface, theme.primary, checkAnim);

    float sw = dp(2.0f);
    float outerR = dp(4.0f);
    float innerR = std::max(0.0f, outerR - sw); 

    // Render outer border container
    renderer.drawM3UI(boxX, boxY, sz, sz, outerR, outerR, outerR, outerR, borderColor);
    
    // Render inner fill
    renderer.drawM3UI(boxX + sw, boxY + sw, sz - sw * 2, sz - sw * 2, innerR, innerR, innerR, innerR, innerColor);

    if (checkAnim > 0.02f) {
        float iconSz = dp(16.0f) * (0.6f + 0.4f * checkAnim);
        M3Color checkTint = theme.onPrimary;
        checkTint.a *= std::min(1.0f, checkAnim * 1.5f);
        renderer.drawIcon(cx - iconSz / 2, cy - iconSz / 2, iconSz, "m3:check", checkTint);
    }

    if (!label.empty()) {
        renderer.drawText(label, boxX + sz + dp(12.0f), cy - dp(14.0f) * 0.45f, dp(14.0f), theme.onSurface);
    }
}

bool Checkbox::handleMouseButton(int button, int action, float mx, float my) {
    if (!isVisible()) return false;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && isInside(mx, my)) {
            state = UIState::Pressed;
            ripple.active = true; ripple.radius = 0.0f; ripple.maxRadius = dp(24.0f);
            return true;
        }
        else if (action == GLFW_RELEASE && state == UIState::Pressed) {
            state = isInside(mx, my) ? UIState::Hover : UIState::Normal;
            if (isInside(mx, my)) {
                checked = !checked;
                if (onCheckedChange) onCheckedChange(checked);
            }
            return true;
        }
    }
    return false;
}