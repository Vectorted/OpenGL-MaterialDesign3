/**
 * @file MediaSlider.cpp
 * @brief Implementation of MediaSlider sine wave math, tracking capture, and rendering.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "MediaSlider.hpp"
#include <cmath>
#include <algorithm>
#include <GLFW/glfw3.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Global pointer tracking the active dragging slider instance to guarantee exclusive input capture.
 */
static MediaSlider* g_activeDraggingSlider = nullptr;

MediaSlider::MediaSlider(float initialValue) : m_value(initialValue) {
    layout_width = WRAP_CONTENT;
    layout_height = WRAP_CONTENT;

    if (m_value < 0.0f) m_value = 0.0f;
    else if (m_value > 1.0f) m_value = 1.0f;
}

float MediaSlider::getValue() const {
    return m_value;
}

MediaSlider* MediaSlider::setValue(float value) {
    if (value < 0.0f) m_value = 0.0f;
    else if (value > 1.0f) m_value = 1.0f;
    else m_value = value;
    return this;
}

MediaSlider* MediaSlider::setEnabled(bool enabled) { m_enabled = enabled; return this; }
MediaSlider* MediaSlider::setPlaying(bool playing) { m_isPlaying = playing; return this; }
MediaSlider* MediaSlider::setStyle(SliderStyle style) { m_style = style; return this; }
MediaSlider* MediaSlider::setOnValueChanged(std::function<void(float)> listener) {
    m_onValueChanged = listener;
    return this;
}

float MediaSlider::getPreferredWidth() { return 200.0f; }
float MediaSlider::getPreferredHeight() { return 40.0f; }

void MediaSlider::doLayout(float parentX, float parentY, float parentW, float parentH) {
    x = parentX + dp(margin_left);
    y = parentY + dp(margin_top);

    if (layout_width == WRAP_CONTENT || width <= 0.0f) {
        width = dp(getPreferredWidth());
    }
    else if (layout_width == MATCH_PARENT) {
        width = parentW - dp(margin_left) - dp(margin_right);
    }
    else if (layout_width >= 0.0f) {
        width = dp(layout_width);
    }

    height = dp(getPreferredHeight());
}

void MediaSlider::update(float dt) {
    View::update(dt);
    if (!m_enabled) return;

    if (m_isDragging) {
        GLFWwindow* win = glfwGetCurrentContext();
        if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            m_isDragging = false;
            
            if (g_activeDraggingSlider == this) {
                g_activeDraggingSlider = nullptr;
            }
        }
    }

    if (m_isPlaying || m_isDragging) {
        m_phase += dt * 12.0f;
        if (m_phase > static_cast<float>(M_PI) * 2.0f) m_phase -= static_cast<float>(M_PI) * 2.0f;
    }
}

void MediaSlider::render(MaterialShader& renderer, MaterialTheme& theme) {
    float h = height > 0.0f ? height : dp(40.0f);
    float w = width;
    float trackY = y + h * 0.5f;
    float thumbX = x + w * m_value;
    float activeW = thumbX - x;

    M3Color activeCol = m_enabled ? theme.primary : M3Color{ theme.onSurface.r, theme.onSurface.g, theme.onSurface.b, 0.38f };
    M3Color inactiveCol = m_enabled ? M3Color{ theme.primary.r, theme.primary.g, theme.primary.b, 0.24f } :
        M3Color{ theme.onSurface.r, theme.onSurface.g, theme.onSurface.b, 0.12f };

    if (m_style == SliderStyle::Squiggly) {
        float thickness = dp(3.0f);
        float amplitude = dp(5.0f);
        
        float freq = (2.0f * static_cast<float>(M_PI)) / dp(36.0f);
        float step = dp(1.0f);
        float r = thickness * 0.5f;

        for (float curX = x; curX <= thumbX; curX += step) {
            float waveY = trackY + std::sin((curX - x) * freq - m_phase) * amplitude;
            renderer.drawM3UI(curX - r, waveY - r, thickness, thickness, r, r, r, r, activeCol);
        }
        float lastWaveY = trackY + std::sin(activeW * freq - m_phase) * amplitude;
        renderer.drawM3UI(thumbX - r, lastWaveY - r, thickness, thickness, r, r, r, r, activeCol);

        if (w - activeW > 0) {
            renderer.drawM3UI(thumbX, trackY - r, w - activeW, thickness, 0, r, r, 0, inactiveCol);
        }

        float thumbW = dp(4.0f), thumbH = dp(24.0f);
        if ((m_isHovered || m_isDragging) && m_enabled) {
            float splashR = dp(20.0f);
            M3Color splashCol = { activeCol.r, activeCol.g, activeCol.b, 0.16f };
            renderer.drawM3UI(thumbX - splashR, trackY - splashR, splashR * 2.0f, splashR * 2.0f, splashR, splashR, splashR, splashR, splashCol);
        }
        renderer.drawM3UI(thumbX - thumbW * 0.5f, trackY - thumbH * 0.5f, thumbW, thumbH, thumbW * 0.5f, thumbW * 0.5f, thumbW * 0.5f, thumbW * 0.5f, activeCol);
    }
    else if (m_style == SliderStyle::Standard) {
        float hAct = dp(6.0f), hInact = dp(4.0f);
        if (activeW > 0) renderer.drawM3UI(x, trackY - hAct * 0.5f, activeW, hAct, hAct * 0.5f, hAct * 0.5f, hAct * 0.5f, hAct * 0.5f, activeCol);
        if (w - activeW > 0) renderer.drawM3UI(thumbX, trackY - hInact * 0.5f, w - activeW, hInact, hInact * 0.5f, hInact * 0.5f, hInact * 0.5f, hInact * 0.5f, inactiveCol);

        float thumbR = dp(10.0f);
        if ((m_isHovered || m_isDragging) && m_enabled) {
            float splashR = dp(20.0f);
            M3Color splashCol = { activeCol.r, activeCol.g, activeCol.b, 0.16f };
            renderer.drawM3UI(thumbX - splashR, trackY - splashR, splashR * 2.0f, splashR * 2.0f, splashR, splashR, splashR, splashR, splashCol);
        }
        renderer.drawM3UI(thumbX - thumbR, trackY - thumbR, thumbR * 2.0f, thumbR * 2.0f, thumbR, thumbR, thumbR, thumbR, activeCol);
    }
    else if (m_style == SliderStyle::Thick) {
        float trackH = dp(16.0f); float r = trackH * 0.5f;
        renderer.drawM3UI(x, trackY - r, w, trackH, r, r, r, r, inactiveCol);
        if (activeW > 0.1f) {
            float cornerRight = (activeW > w - 0.2f) ? r : 0.0f;
            renderer.drawM3UI(x, trackY - r, activeW, trackH, r, cornerRight, cornerRight, r, activeCol);
        }
        float innerLineW = dp(4.0f), innerLineH = dp(12.0f);
        M3Color innerCol = { theme.surface.r, theme.surface.g, theme.surface.b, 0.9f };
        if (activeW < innerLineW) innerCol = activeCol;
        renderer.drawM3UI(thumbX - innerLineW * 0.5f, trackY - innerLineH * 0.5f, innerLineW, innerLineH, innerLineW * 0.5f, innerLineW * 0.5f, innerLineW * 0.5f, innerLineW * 0.5f, innerCol);
    }
    else if (m_style == SliderStyle::LinearProgress) {
        float ptH = dp(4.0f); float r = ptH * 0.5f;
        if (activeW > 0) renderer.drawM3UI(x, trackY - r, activeW, ptH, r, r, r, r, activeCol);
        if (w - activeW > 0) renderer.drawM3UI(thumbX, trackY - r, w - activeW, ptH, r, r, r, r, inactiveCol);
    }
}

bool MediaSlider::handleMouseMove(float mx, float my) {
    if (!m_enabled || !isVisible() || m_style == SliderStyle::LinearProgress) return false;

    if (g_activeDraggingSlider != nullptr && g_activeDraggingSlider != this) {
        m_isHovered = false;
        m_isDragging = false;
        return false;
    }

    m_isHovered = (mx >= x - dp(10) && mx <= x + width + dp(10) && my >= y - dp(10) && my <= y + height + dp(10));

    if (m_isDragging) {
        float rawValue = (mx - x) / width;
        if (rawValue < 0.0f) m_value = 0.0f; else if (rawValue > 1.0f) m_value = 1.0f; else m_value = rawValue;
        if (m_onValueChanged) m_onValueChanged(m_value);

        return true;
    }

    return m_isHovered;
}

bool MediaSlider::handleMouseButton(int button, int action, float mx, float my) {
    if (!m_enabled || !isVisible() || button != GLFW_MOUSE_BUTTON_LEFT || m_style == SliderStyle::LinearProgress) return false;

    if (action == GLFW_PRESS) {
        if (m_isHovered) {
            m_isDragging = true;
            g_activeDraggingSlider = this; 

            float rawValue = (mx - x) / width;
            if (rawValue < 0.0f) m_value = 0.0f; else if (rawValue > 1.0f) m_value = 1.0f; else m_value = rawValue;
            if (m_onValueChanged) m_onValueChanged(m_value);
            return true;
        }
    }
    else if (action == GLFW_RELEASE) {
        if (m_isDragging) {
            m_isDragging = false;
            
            if (g_activeDraggingSlider == this) {
                g_activeDraggingSlider = nullptr; 
            }
            return true;
        }
    }
    return false;
}