/**
 * @file TimePickerDial.cpp
 * @brief Implementation of TimePickerDial polar coordinate physics, clock hand rendering, and digital input cards.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "TimePickerDial.hpp"
#include <cmath>
#include <algorithm>
#include <string>
#include <GLFW/glfw3.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

TimePickerDial::TimePickerDial(float w, float h) : View(0, 0, w, h) {
    layout_width = w; 
    layout_height = h;
    currentAngle = getTargetAngleFromValue();
    targetAngle = currentAngle;
}

float TimePickerDial::getTargetAngleFromValue() {
    int val = (mode == TimePickerMode::Hour) ? selectedHour : selectedMinute;
    if (mode == TimePickerMode::Hour) {
        return (val / 12.0f) * (2.0f * M_PI) - (M_PI / 2.0f);
    }
    else {
        return (val / 60.0f) * (2.0f * M_PI) - (M_PI / 2.0f);
    }
}

int TimePickerDial::calculateValueFromPosition(float mx, float my) {
    float clockAreaHeight = height - dp(40.0f);
    float cx = x + width / 2.0f;
    float cy = y + dp(40.0f) + clockAreaHeight / 2.0f;
    
    float angle = std::atan2(my - cy, mx - cx);
    angle += (M_PI / 2.0f);
    if (angle < 0.0f) {
        angle += 2.0f * M_PI;
    }

    if (mode == TimePickerMode::Hour) {
        int val = (int)std::round((angle / (2.0f * M_PI)) * 12.0f);
        return (val == 0 || val == 12) ? 12 : val;
    }
    else {
        int val = (int)std::round((angle / (2.0f * M_PI)) * 60.0f);
        return (val == 60) ? 0 : val;
    }
}

bool TimePickerDial::isInsideIcon(float mx, float my, float ix, float iy, float size) {
    return mx >= ix - size && 
           mx <= ix + size * 2.0f && 
           my >= iy - size && 
           my <= iy + size * 2.0f;
}

void TimePickerDial::update(float dt) {
    View::update(dt);
    if (style != TimePickerStyle::Dial) return;

    targetAngle = getTargetAngleFromValue();
    
    float diff = targetAngle - currentAngle;
    while (diff <= -M_PI) diff += 2.0f * M_PI;
    while (diff > M_PI) diff -= 2.0f * M_PI;

    float error = -diff;
    float dt_safe = std::min(dt, 0.033f);
    float stiff = 25.0f;

    float temp = (angleVelocity + stiff * error) * dt_safe;
    float decay = std::exp(-stiff * dt_safe);
    float new_error = (error + temp) * decay;
    
    angleVelocity = (angleVelocity - stiff * temp) * decay;
    currentAngle = (currentAngle + diff) + new_error;

    while (currentAngle <= -M_PI) currentAngle += 2.0f * M_PI;
    while (currentAngle > M_PI) currentAngle -= 2.0f * M_PI;
}

void TimePickerDial::render(MaterialShader& renderer, MaterialTheme& theme) {
    float clockAreaHeight = height - dp(40.0f);
    float cx = x + width / 2.0f;
    float cy = y + dp(40.0f) + clockAreaHeight / 2.0f;

    renderer.drawText("Enter time", x + dp(24.0f), y + dp(20.0f) - dp(13.0f) * 0.45f, dp(13.0f), theme.onSurfaceVariant);

    if (style == TimePickerStyle::Dial) {
        float radius = std::min(width, clockAreaHeight) / 2.0f - dp(24.0f);

        // Analog clock face surface
        float bgR = radius + dp(16.0f);
        renderer.drawM3UI(cx - bgR, cy - bgR, bgR * 2, bgR * 2, bgR, bgR, bgR, bgR, theme.surfaceContainerHigh);

        float bubbleR = dp(20.0f);
        float lineLength = radius - bubbleR;
        float lineThickness = dp(3.0f); 

        // Rotating hand
        renderer.drawM3UI(
            cx, cy - lineThickness / 2.0f, lineLength + dp(4.0f), lineThickness,
            lineThickness / 2.0f, lineThickness / 2.0f, lineThickness / 2.0f, lineThickness / 2.0f, theme.primary,
            0, 0, 0, 0, { 0,0,0,0 }, 0, 0, { 0,0,0,0 }, { 0,0,0,0 }, 0, 0, false, 0, 0, 0, 0, 0,
            currentAngle, cx, cy
        );

        // Central anchor pivot
        float dotR = dp(4.0f);
        renderer.drawM3UI(cx - dotR, cy - dotR, dotR * 2, dotR * 2, dotR, dotR, dotR, dotR, theme.primary);

        // Target bubble
        float bubbleX = cx + std::cos(currentAngle) * radius;
        float bubbleY = cy + std::sin(currentAngle) * radius;
        renderer.drawM3UI(bubbleX - bubbleR, bubbleY - bubbleR, bubbleR * 2, bubbleR * 2, bubbleR, bubbleR, bubbleR, bubbleR, theme.primary);

        // Dial numbers
        int count = (mode == TimePickerMode::Hour) ? 12 : 60;
        int stepSize = (mode == TimePickerMode::Hour) ? 1 : 5;
        
        for (int i = (mode == TimePickerMode::Hour ? 1 : 0); i <= count; i += stepSize) {
            if (mode == TimePickerMode::Hour && i == 0) continue;
            if (mode != TimePickerMode::Hour && i == 60) continue;

            float a = (i / (float)count) * (2.0f * M_PI) - (M_PI / 2.0f);
            float numX = cx + std::cos(a) * radius;
            float numY = cy + std::sin(a) * radius;
            std::string txt = std::to_string(i);
            float textSize = dp(14.0f);
            float txtW = renderer.getTextWidth(txt, textSize);
            
            M3Color txtColor = (std::abs(a - currentAngle) < 0.2f) ? theme.onPrimary : theme.onSurface;

            renderer.drawText(txt, numX - txtW / 2.0f, numY - textSize * 0.45f, textSize, txtColor);
        }
    }
    else {
        // Dual digital input blocks
        float boxW = dp(112.0f); 
        float boxH = dp(80.0f);
        float gap = dp(16.0f);
        float startX = cx - boxW - gap / 2.0f;
        float startY = cy - boxH / 2.0f;

        // Hours container
        M3Color hBg = (mode == TimePickerMode::Hour) ? theme.primaryContainer : theme.surfaceContainerHigh;
        M3Color hTx = (mode == TimePickerMode::Hour) ? theme.onPrimaryContainer : theme.onSurface;
        renderer.drawM3UI(startX, startY, boxW, boxH, dp(12), dp(12), dp(12), dp(12), hBg);

        std::string hStr = (selectedHour < 10 ? "0" : "") + std::to_string(selectedHour);
        float hw = renderer.getTextWidth(hStr, dp(50.0f));
        renderer.drawText(hStr, startX + boxW / 2 - hw / 2, startY + boxH / 2.0f - dp(50.0f) * 0.45f, dp(50.0f), hTx);
        renderer.drawText("Hour", startX + dp(4), startY + boxH + dp(8), dp(12.0f), theme.onSurfaceVariant);

        // Center colon separator
        renderer.drawText(":", cx - renderer.getTextWidth(":", dp(50.0f)) / 2, startY + boxH / 2.0f - dp(50.0f) * 0.45f - dp(4.0f), dp(50.0f), theme.onSurface);

        // Minutes container
        M3Color mBg = (mode == TimePickerMode::Minute) ? theme.primaryContainer : theme.surfaceContainerHigh;
        M3Color mTx = (mode == TimePickerMode::Minute) ? theme.onPrimaryContainer : theme.onSurface;
        renderer.drawM3UI(startX + boxW + gap, startY, boxW, boxH, dp(12), dp(12), dp(12), dp(12), mBg);

        std::string mStr = (selectedMinute < 10 ? "0" : "") + std::to_string(selectedMinute);
        float mw = renderer.getTextWidth(mStr, dp(50.0f));
        renderer.drawText(mStr, startX + boxW + gap + boxW / 2 - mw / 2, startY + boxH / 2.0f - dp(50.0f) * 0.45f, dp(50.0f), mTx);
        renderer.drawText("Minute", startX + boxW + gap + dp(4), startY + boxH + dp(8), dp(12.0f), theme.onSurfaceVariant);
    }

    // Input mode toggle icon
    std::string iconName = (style == TimePickerStyle::Dial) ? "m3:edit" : "m3:home";
    renderer.drawIcon(x + dp(16.0f), y + height - dp(32.0f), dp(22.0f), iconName, theme.onSurfaceVariant);
}

bool TimePickerDial::handleMouseButton(int button, int action, float mx, float my) {
    if (!isVisible()) return false;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            if (isInsideIcon(mx, my, x + dp(16.0f), y + height - dp(32.0f), dp(22.0f))) {
                style = (style == TimePickerStyle::Dial) ? TimePickerStyle::Input : TimePickerStyle::Dial;
                return true;
            }
            
            if (style == TimePickerStyle::Dial && isInside(mx, my)) {
                isDragging = true;
                int val = calculateValueFromPosition(mx, my);
                if (mode == TimePickerMode::Hour) {
                    selectedHour = val; 
                } else {
                    selectedMinute = val;
                }
                return true;
            }
            else if (style == TimePickerStyle::Input && isInside(mx, my)) {
                float cx = x + width / 2.0f;
                if (mx < cx) {
                    mode = TimePickerMode::Hour;
                } else {
                    mode = TimePickerMode::Minute;
                }
                return true;
            }
        }
        else if (action == GLFW_RELEASE && isDragging) {
            isDragging = false;
            if (onTimeSelected) {
                onTimeSelected(selectedHour, selectedMinute);
            }
            return true;
        }
    }
    return View::handleMouseButton(button, action, mx, my);
}

bool TimePickerDial::handleMouseMove(float mx, float my) {
    if (!isVisible()) return false;
    
    bool mouseBtn = (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    
    if (isDragging && style == TimePickerStyle::Dial) {
        if (!mouseBtn) {
            isDragging = false;
        } else {
            int val = calculateValueFromPosition(mx, my);
            if (mode == TimePickerMode::Hour) {
                selectedHour = val; 
            } else {
                selectedMinute = val;
            }
        }
        return true;
    }
    return View::handleMouseMove(mx, my);
}