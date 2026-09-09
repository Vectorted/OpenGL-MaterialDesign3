/**
 * @file MediaSlider.cpp
 * @brief Implementation of Material 3 Capsule and Discrete Ticked Slider.
 * 
 * Implements multiple slider styles, discrete stepping with tick marks,
 * time display, value tooltip, and exclusive drag capture with smooth
 * hover/drag animations.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "MediaSlider.hpp"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <GLFW/glfw3.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern void requestUIWakeup(double seconds);

/** Global pointer to the slider currently being dragged (used for event conflict avoidance). */
MediaSlider* g_activeDraggingSlider = nullptr;

// --- Constructor ---

MediaSlider::MediaSlider(float initialValue) : m_value(initialValue) {
    layout_width = WRAP_CONTENT;
    layout_height = WRAP_CONTENT;
    m_value = std::clamp(initialValue, 0.0f, 1.0f);
}

float MediaSlider::getValue() const { return m_value; }

float MediaSlider::snapValue(float rawValue) const {
    if (m_discrete && m_steps > 0) {
        float stepSize = 1.0f / static_cast<float>(m_steps);
        float snapped = std::round(rawValue / stepSize) * stepSize;
        return std::clamp(snapped, 0.0f, 1.0f);
    }
    return std::clamp(rawValue, 0.0f, 1.0f);
}

MediaSlider* MediaSlider::setValue(float value) {
    m_value = snapValue(value);
    if (m_totalSeconds > 0.0f) {
        m_currentSeconds = m_value * m_totalSeconds;
    }
    return this;
}

MediaSlider* MediaSlider::setEnabled(bool enabled) { m_enabled = enabled; return this; }
MediaSlider* MediaSlider::setPlaying(bool playing) { m_isPlaying = playing; return this; }
MediaSlider* MediaSlider::setStyle(SliderStyle style) { m_style = style; return this; }
MediaSlider* MediaSlider::setShowTime(bool show) { m_showTime = show; return this; }

MediaSlider* MediaSlider::setDiscreteConfig(int steps, bool discrete, bool showTicks, bool showValueIndicator) {
    m_steps = std::max(0, steps);
    m_discrete = discrete;
    m_showTicks = showTicks;
    m_showValueIndicator = showValueIndicator;
    if (m_discrete && m_steps > 0) {
        m_value = snapValue(m_value);
    }
    return this;
}

MediaSlider* MediaSlider::setTime(float currentSec, float totalSec) {
    m_currentSeconds = std::max(0.0f, currentSec);
    m_totalSeconds = std::max(0.001f, totalSec);
    m_value = std::clamp(m_currentSeconds / m_totalSeconds, 0.0f, 1.0f);
    return this;
}

MediaSlider* MediaSlider::setOnValueChanged(std::function<void(float)> listener) {
    m_onValueChanged = listener;
    return this;
}

std::string MediaSlider::formatTime(float seconds) {
    int totalSec = static_cast<int>(std::round(seconds));
    int mins = totalSec / 60;
    int secs = totalSec % 60;
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%d:%02d", mins, secs);
    return std::string(buf);
}

float MediaSlider::getPreferredWidth() { return 260.0f; }
float MediaSlider::getPreferredHeight() { return 44.0f; }

void MediaSlider::doLayout(float parentX, float parentY, float parentW, float parentH) {
    x = std::round(parentX + dp(margin_left));
    y = std::round(parentY + dp(margin_top));

    if (layout_width == WRAP_CONTENT || width <= 0.0f) {
        width = std::round(dp(getPreferredWidth()));
    } else if (layout_width == MATCH_PARENT) {
        width = std::round(parentW - dp(margin_left) - dp(margin_right));
    } else if (layout_width >= 0.0f) {
        width = std::round(dp(layout_width));
    }

    height = std::round(dp(getPreferredHeight()));
}

void MediaSlider::update(float dt) {
    View::update(dt);
    if (!m_enabled) return;

    float dtSafe = std::min(dt, 0.033f);

    // Handle hover/drag scale animation
    float targetScale = (m_isDragging || m_isHovered) ? 1.0f : 0.0f;
    float scaleDiff = targetScale - m_handleScaleAnim;
    if (std::abs(scaleDiff) > 0.005f) {
        m_handleScaleAnim += scaleDiff * (1.0f - std::exp(-18.0f * dtSafe));
        requestUIWakeup(0.016);
    } else {
        m_handleScaleAnim = targetScale;
    }

    // Tooltip popup animation
    float targetTooltip = m_isDragging ? 1.0f : 0.0f;
    float tooltipDiff = targetTooltip - m_tooltipAnim;
    if (std::abs(tooltipDiff) > 0.005f) {
        m_tooltipAnim += tooltipDiff * (1.0f - std::exp(-22.0f * dtSafe));
        requestUIWakeup(0.016);
    } else {
        m_tooltipAnim = targetTooltip;
    }

    // Global drag capture with continuous position update
    if (m_isDragging) {
        GLFWwindow* win = glfwGetCurrentContext();
        if (win) {
            if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
                m_isDragging = false;
                if (g_activeDraggingSlider == this) {
                    g_activeDraggingSlider = nullptr;
                }
            } else {
                double curX, curY;
                glfwGetCursorPos(win, &curX, &curY);

                float trackLeft = x;
                float trackW = width;
                if (m_showTime) {
                    float timeTextMargin = dp(44.0f);
                    trackLeft += timeTextMargin;
                    trackW -= timeTextMargin * 2.0f;
                }

                if (trackW > 1.0f) {
                    float rawValue = (static_cast<float>(curX) - trackLeft) / trackW;
                    float newValue = snapValue(rawValue);
                    if (std::abs(newValue - m_value) > 0.0001f) {
                        m_value = newValue;
                        m_currentSeconds = m_value * m_totalSeconds;
                        if (m_onValueChanged) m_onValueChanged(m_value);
                    }
                }
            }
        }
    }

    if (m_isPlaying || m_isDragging) {
        m_phase += dt * 12.0f;
        if (m_phase > static_cast<float>(M_PI) * 2.0f) {
            m_phase -= static_cast<float>(M_PI) * 2.0f;
        }
    }
}

void MediaSlider::render(MaterialShader& renderer, MaterialTheme& theme) {
    float h = height > 0.0f ? height : dp(44.0f);
    float trackCenterY = std::round(y + h * 0.5f);

    float trackX = x;
    float trackW = width;

    // 1. Render time labels at ends (if enabled)
    if (m_showTime) {
        float timeTextWidth = dp(36.0f);
        float timeMargin = dp(8.0f);
        float totalOffset = timeTextWidth + timeMargin;

        trackX = x + totalOffset;
        trackW = width - totalOffset * 2.0f;

        M3Color timeColor = theme.onSurfaceVariant;
        timeColor.a *= (m_enabled ? 0.75f : 0.38f);
        float fontSize = dp(11.0f);

        std::string curTimeStr = formatTime(m_currentSeconds);
        renderer.drawText(curTimeStr, x, trackCenterY - fontSize * 0.5f, fontSize, timeColor);

        std::string totalTimeStr = formatTime(m_totalSeconds);
        renderer.drawText(totalTimeStr, x + width - timeTextWidth, trackCenterY - fontSize * 0.5f, fontSize, timeColor);
    }

    float thumbX = std::round(trackX + trackW * m_value);

    // M3 color scheme
    M3Color activeCol = m_enabled ? theme.primary : M3Color{ theme.onSurface.r, theme.onSurface.g, theme.onSurface.b, 0.38f };
    M3Color inactiveCol = m_enabled ? theme.surfaceContainerHigh : M3Color{ theme.onSurface.r, theme.onSurface.g, theme.onSurface.b, 0.12f };

    if (m_style == SliderStyle::Thick) {
        float trackH = std::round(dp(16.0f));
        float trackRadius = trackH * 0.5f;
        float gap = std::round(dp(6.0f)); 

        float handleW = std::round(dp(4.0f) + dp(2.0f) * m_handleScaleAnim);
        float handleH = std::round(dp(28.0f) + dp(4.0f) * m_handleScaleAnim);
        float handleRadius = handleW * 0.5f;

        // 1.1 Active capsule segment (left of thumb)
        float activeRight = thumbX - gap * 0.5f;
        float activeW = std::max(0.0f, activeRight - trackX);
        if (activeW > 0.5f) {
            float rLeft = trackRadius;
            float rRight = (activeW < trackRadius) ? activeW * 0.5f : trackRadius;
            renderer.drawM3UI(trackX, trackCenterY - trackRadius, activeW, trackH, rLeft, rRight, rRight, rLeft, activeCol);
        }

        // 1.2 Inactive capsule segment (right of thumb)
        float inactLeft = thumbX + gap * 0.5f;
        float inactW = std::max(0.0f, (trackX + trackW) - inactLeft);
        if (inactW > 0.5f) {
            float rRight = trackRadius;
            float rLeft = (inactW < trackRadius) ? inactW * 0.5f : trackRadius;
            renderer.drawM3UI(inactLeft, trackCenterY - trackRadius, inactW, trackH, rLeft, rRight, rRight, rLeft, inactiveCol);
        }

        // 1.3 Tick marks (skip endpoints and area near thumb)
        if (m_showTicks && m_steps > 1) {
            float tickRadius = dp(2.0f);
            float stepDistance = trackW / static_cast<float>(m_steps);

            for (int i = 1; i < m_steps; ++i) {
                float tickX = trackX + i * stepDistance;

                // Skip ticks that would overlap the handle
                if (std::abs(tickX - thumbX) <= (gap * 0.5f + tickRadius + dp(2.0f))) {
                    continue;
                }

                M3Color tickColor;
                if (tickX < thumbX) {
                    tickColor = theme.onPrimary;
                    tickColor.a = 0.65f;
                } else {
                    tickColor = theme.onSurfaceVariant;
                    tickColor.a = 0.45f;
                }

                renderer.drawM3UI(
                    std::round(tickX - tickRadius),
                    std::round(trackCenterY - tickRadius),
                    std::round(tickRadius * 2.0f),
                    std::round(tickRadius * 2.0f),
                    tickRadius, tickRadius, tickRadius, tickRadius,
                    tickColor
                );
            }
        }

        // 1.4 Handle state layer (splash)
        if (m_handleScaleAnim > 0.01f && m_enabled) {
            float splashW = std::round(dp(16.0f));
            float splashH = std::round(dp(36.0f));
            float splashR = splashW * 0.5f;
            M3Color splashCol = { activeCol.r, activeCol.g, activeCol.b, 0.16f * m_handleScaleAnim };
            renderer.drawM3UI(thumbX - splashW * 0.5f, trackCenterY - splashH * 0.5f, splashW, splashH, splashR, splashR, splashR, splashR, splashCol);
        }

        // 1.5 Vertical handle bar
        renderer.drawM3UI(thumbX - handleW * 0.5f, trackCenterY - handleH * 0.5f, handleW, handleH, handleRadius, handleRadius, handleRadius, handleRadius, activeCol);

        // 1.6 Value tooltip (Snackbar-like style with exact getTextWidth alignment)
        if (m_showValueIndicator && m_tooltipAnim > 0.01f) {
            char valStr[16];
            if (m_steps > 0) {
                int curStep = static_cast<int>(std::round(m_value * m_steps));
                std::snprintf(valStr, sizeof(valStr), "%d", curStep);
            } else {
                std::snprintf(valStr, sizeof(valStr), "%d%%", static_cast<int>(std::round(m_value * 100.0f)));
            }

            std::string text(valStr);
            const float fontSize = dp(13.0f);
            const float textW = renderer.getTextWidth(text, fontSize);

            // Adaptive bubble width based on text length
            const float paddingX = dp(12.0f);
            const float tipW = std::max(dp(38.0f), std::round(textW + paddingX * 2.0f));
            const float tipH = dp(32.0f);
            const float cornerRadius = dp(12.0f);

            // Vertical offset with breathing room
            const float tipOffset = std::round(dp(44.0f) + dp(10.0f) * m_tooltipAnim);
            const float tipX = std::round(thumbX - tipW * 0.5f);
            const float tipY = std::round(trackCenterY - tipOffset);

            // 6.1 Background (onSurface) and text (surface) colors, with opacity
            M3Color bgColor = theme.onSurface;
            bgColor.a *= m_tooltipAnim;

            M3Color textColor = theme.surface;
            textColor.a *= m_tooltipAnim;

            // 6.2 Soft shadow
            M3Color shadowCol = { 0.0f, 0.0f, 0.0f, 0.20f * m_tooltipAnim };
            renderer.drawM3UI(tipX, tipY + dp(2.0f), tipW, tipH, cornerRadius, cornerRadius, cornerRadius, cornerRadius, shadowCol);

            // 6.3 Snackbar container
            renderer.drawM3UI(tipX, tipY, tipW, tipH, cornerRadius, cornerRadius, cornerRadius, cornerRadius, bgColor);

            // 6.4 Centered text using exact width
            const float textX = std::round(tipX + (tipW - textW) * 0.5f);
            const float textY = std::round(tipY + (tipH - fontSize) * 0.5f);

            renderer.drawText(text, textX, textY, fontSize, textColor);
        }
    }
    else if (m_style == SliderStyle::Standard) {
        float hAct = dp(6.0f), hInact = dp(4.0f);
        float activeW = thumbX - trackX;
        if (activeW > 0) renderer.drawM3UI(trackX, trackCenterY - hAct * 0.5f, activeW, hAct, hAct * 0.5f, hAct * 0.5f, hAct * 0.5f, hAct * 0.5f, activeCol);
        if (trackW - activeW > 0) renderer.drawM3UI(thumbX, trackCenterY - hInact * 0.5f, trackW - activeW, hInact, hInact * 0.5f, hInact * 0.5f, hInact * 0.5f, hInact * 0.5f, inactiveCol);

        if (m_showTicks && m_steps > 1) {
            float tickRadius = dp(2.0f);
            float stepDistance = trackW / static_cast<float>(m_steps);

            for (int i = 1; i < m_steps; ++i) {
                float tickX = trackX + i * stepDistance;
                if (std::abs(tickX - thumbX) <= dp(8.0f)) continue;

                M3Color tickColor = (tickX < thumbX) ? theme.onPrimary : theme.onSurfaceVariant;
                tickColor.a = 0.5f;
                renderer.drawM3UI(tickX - tickRadius, trackCenterY - tickRadius, tickRadius * 2.0f, tickRadius * 2.0f, tickRadius, tickRadius, tickRadius, tickRadius, tickColor);
            }
        }

        float thumbR = dp(10.0f);
        if ((m_isHovered || m_isDragging) && m_enabled) {
            float splashR = dp(20.0f);
            M3Color splashCol = { activeCol.r, activeCol.g, activeCol.b, 0.16f };
            renderer.drawM3UI(thumbX - splashR, trackCenterY - splashR, splashR * 2.0f, splashR * 2.0f, splashR, splashR, splashR, splashR, splashCol);
        }
        renderer.drawM3UI(thumbX - thumbR, trackCenterY - thumbR, thumbR * 2.0f, thumbR * 2.0f, thumbR, thumbR, thumbR, thumbR, activeCol);
    }
    else if (m_style == SliderStyle::Squiggly) {
        float thickness = dp(3.0f);
        float amplitude = dp(5.0f);
        float freq = (2.0f * static_cast<float>(M_PI)) / dp(36.0f);
        float step = dp(1.0f);
        float r = thickness * 0.5f;

        float activeW = thumbX - trackX;
        for (float curX = trackX; curX <= thumbX; curX += step) {
            float waveY = trackCenterY + std::sin((curX - trackX) * freq - m_phase) * amplitude;
            renderer.drawM3UI(curX - r, waveY - r, thickness, thickness, r, r, r, r, activeCol);
        }

        if (trackW - activeW > 0) {
            renderer.drawM3UI(thumbX, trackCenterY - r, trackW - activeW, thickness, 0, r, r, 0, inactiveCol);
        }

        float thumbW = dp(4.0f), thumbH = dp(24.0f);
        if ((m_isHovered || m_isDragging) && m_enabled) {
            float splashR = dp(20.0f);
            M3Color splashCol = { activeCol.r, activeCol.g, activeCol.b, 0.16f };
            renderer.drawM3UI(thumbX - splashR, trackCenterY - splashR, splashR * 2.0f, splashR * 2.0f, splashR, splashR, splashR, splashR, splashCol);
        }
        renderer.drawM3UI(thumbX - thumbW * 0.5f, trackCenterY - thumbH * 0.5f, thumbW, thumbH, thumbW * 0.5f, thumbW * 0.5f, thumbW * 0.5f, thumbW * 0.5f, activeCol);
    }
    else if (m_style == SliderStyle::LinearProgress) {
        float ptH = dp(4.0f); float r = ptH * 0.5f;
        float activeW = thumbX - trackX;
        if (activeW > 0) renderer.drawM3UI(trackX, trackCenterY - r, activeW, ptH, r, r, r, r, activeCol);
        if (trackW - activeW > 0) renderer.drawM3UI(thumbX, trackCenterY - r, trackW - activeW, ptH, r, r, r, r, inactiveCol);
    }
}

bool MediaSlider::handleMouseMove(float mx, float my) {
    if (!m_enabled || !isVisible() || m_style == SliderStyle::LinearProgress) return false;

    if (g_activeDraggingSlider != nullptr && g_activeDraggingSlider != this) {
        m_isHovered = false;
        m_isDragging = false;
        return false;
    }

    m_isHovered = isInside(mx, my);

    if (m_isDragging) {
        float trackLeft = x;
        float trackW = width;
        if (m_showTime) {
            float timeTextMargin = dp(44.0f);
            trackLeft += timeTextMargin;
            trackW -= timeTextMargin * 2.0f;
        }

        if (trackW > 1.0f) {
            float rawValue = (mx - trackLeft) / trackW;
            float newValue = snapValue(rawValue);
            if (std::abs(newValue - m_value) > 0.0001f) {
                m_value = newValue;
                m_currentSeconds = m_value * m_totalSeconds;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
        }
        return true;
    }

    return false;
}

bool MediaSlider::handleMouseButton(int button, int action, float mx, float my) {
    if (!m_enabled || !isVisible() || button != GLFW_MOUSE_BUTTON_LEFT || m_style == SliderStyle::LinearProgress) return false;

    if (action == GLFW_PRESS) {
        if (isInside(mx, my)) {
            m_isDragging = true;
            g_activeDraggingSlider = this;

            float trackLeft = x;
            float trackW = width;
            if (m_showTime) {
                float timeTextMargin = dp(44.0f);
                trackLeft += timeTextMargin;
                trackW -= timeTextMargin * 2.0f;
            }

            if (trackW > 1.0f) {
                float rawValue = (mx - trackLeft) / trackW;
                float newValue = snapValue(rawValue);
                m_value = newValue;
                m_currentSeconds = m_value * m_totalSeconds;
                if (m_onValueChanged) m_onValueChanged(m_value);
            }
            return true;
        }
    } else if (action == GLFW_RELEASE) {
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
