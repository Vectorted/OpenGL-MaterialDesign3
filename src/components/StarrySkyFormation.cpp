/**
 * @file StarrySkyFormation.cpp
 * @brief Implementation of dynamic geometric magic array formation, Greek runes, and kinetic gesture handling.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "StarrySkyFormation.hpp"
#include <cmath>
#include <algorithm>

extern void requestUIWakeup(double seconds);

/**
 * @brief Renders a stroked circle using subpixel interpolated line segments.
 */
static void drawStrokedCircle(MaterialShader& renderer, float cx, float cy, float r, float thickness, const M3Color& col, bool dashed = false) {
    if (r <= 0.5f) return;
    int segments = std::max(16, static_cast<int>(r * 1.5f));
    float stepAngle = 2.0f * 3.14159265f / segments;
    for (int i = 0; i < segments; ++i) {
        if (dashed && (i % 2 == 1)) continue; 
        float a1 = i * stepAngle;
        float a2 = (i + 1) * stepAngle;
        float x1 = cx + r * std::cos(a1);
        float y1 = cy + r * std::sin(a1);
        float x2 = cx + r * std::cos(a2);
        float y2 = cy + r * std::sin(a2);

        float dx = x2 - x1, dy = y2 - y1;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f) continue;

        float subSteps = std::max(1.0f, std::floor(len / 1.2f));
        for (float s = 0; s <= subSteps; s += 1.0f) {
            float tNorm = s / subSteps;
            float px = x1 + dx * tNorm;
            float py = y1 + dy * tNorm;
            renderer.drawM3UI(px - thickness * 0.5f, py - thickness * 0.5f, thickness, thickness, thickness * 0.5f, thickness * 0.5f, thickness * 0.5f, thickness * 0.5f, col);
        }
    }
}

/**
 * @brief Renders an anti-aliased rotated square boundary.
 */
static void drawStrokedSquare(MaterialShader& renderer, float cx, float cy, float halfSize, float angleRad, float thickness, const M3Color& col) {
    float corners[4][2] = {
        { -halfSize, -halfSize },
        {  halfSize, -halfSize },
        {  halfSize,  halfSize },
        { -halfSize,  halfSize }
    };

    float cosA = std::cos(angleRad);
    float sinA = std::sin(angleRad);

    float rotCorners[4][2];
    for (int i = 0; i < 4; ++i) {
        rotCorners[i][0] = cx + corners[i][0] * cosA - corners[i][1] * sinA;
        rotCorners[i][1] = cy + corners[i][0] * sinA + corners[i][1] * cosA;
    }

    for (int i = 0; i < 4; ++i) {
        int next = (i + 1) % 4;
        float x1 = rotCorners[i][0], y1 = rotCorners[i][1];
        float x2 = rotCorners[next][0], y2 = rotCorners[next][1];

        float dx = x2 - x1, dy = y2 - y1;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f) continue;

        float subSteps = std::max(1.0f, std::floor(len / 1.2f));
        for (float s = 0; s <= subSteps; s += 1.0f) {
            float tNorm = s / subSteps;
            float px = x1 + dx * tNorm;
            float py = y1 + dy * tNorm;
            renderer.drawM3UI(px - thickness * 0.5f, py - thickness * 0.5f, thickness, thickness, thickness * 0.5f, thickness * 0.5f, thickness * 0.5f, thickness * 0.5f, col);
        }
    }
}

StarrySkyFormation::StarrySkyFormation(float size) : m_size(size) {
    layout_width = size;
    layout_height = size;
    width = size;
    height = size;
    x = 60.0f;
    y = 60.0f;
}

void StarrySkyFormation::setLevel(int level) {
    if (level < 0) level = 0;
    if (level > 2) level = 2;
    setLevel(static_cast<FormationLevel>(level));
}

void StarrySkyFormation::setLevel(FormationLevel level) {
    m_level = level;
    if (m_level == FormationLevel::Inactive) m_targetScale = 0.80f;
    else if (m_level == FormationLevel::Active) m_targetScale = 1.15f;
    else if (m_level == FormationLevel::Alert) m_targetScale = 1.30f;
}

void StarrySkyFormation::setSize(float size) {
    m_size = size;
    layout_width = size;
    layout_height = size;
    width = size;
    height = size;
}

void StarrySkyFormation::setMove(bool isZoomIn) {
    m_scale = isZoomIn ? 1.3f : 0.7f;
}

void StarrySkyFormation::update(float dt) {
    View::update(dt);
    float dtSafe = std::min(dt, 0.033f);

    m_scale += (m_targetScale - m_scale) * 10.0f * dtSafe;

    float spinSpeed = 24.0f;
    float greekSpeed = -18.0f;
    float pulseSpeed = 0.25f;

    if (m_level == FormationLevel::Active) {
        spinSpeed = 60.0f;
        greekSpeed = -45.0f;
        pulseSpeed = 0.50f;
    } else if (m_level == FormationLevel::Alert) {
        spinSpeed = 240.0f;
        greekSpeed = -180.0f;
        pulseSpeed = 1.0f;
    }

    m_outerSpinAngle += spinSpeed * dtSafe;
    m_greekRotateAngle += greekSpeed * dtSafe;

    m_pulseTimer += pulseSpeed * dtSafe;
    if (m_pulseTimer > 1.0f) m_pulseTimer -= 1.0f;

    // Long press detection
    if (m_isPressed && !m_hasFiredLongClick) {
        m_pressDuration += dtSafe;
        if (m_pressDuration >= 0.6f) {
            float moveDist = std::hypot(x + m_dragOffsetX - m_downX, y + m_dragOffsetY - m_downY);
            if (moveDist < dp(6.0f)) {
                m_hasFiredLongClick = true;
                if (m_onLongClick) m_onLongClick();
            }
        }
    }

    if (m_clickCount > 0) {
        m_clickTimer -= dtSafe;
        if (m_clickTimer <= 0.0f) {
            if (m_clickCount == 1 && m_onClick) {
                m_onClick();
            }
            m_clickCount = 0;
        }
    }

    requestUIWakeup(0.016);
}

void StarrySkyFormation::drawMagicArray(MaterialShader& renderer, float cx, float cy, float radius,
                                        FormationLevel level, float outerAngle, float greekAngle,
                                        float pulseProgress, const M3Color& baseColor) {
    float scale = radius / 50.0f;
    float lineThick = std::max(1.0f, dp(1.2f) * scale);

    // 1. Dual rotating octagonal square arrays
    float starHalfSize = 29.0f * scale;
    float outerRad1 = outerAngle * 3.14159265f / 180.0f;
    float outerRad2 = (outerAngle + 45.0f) * 3.14159265f / 180.0f;

    drawStrokedSquare(renderer, cx, cy, starHalfSize, outerRad1, lineThick * 1.1f, baseColor);
    drawStrokedSquare(renderer, cx, cy, starHalfSize, outerRad2, lineThick * 1.1f, baseColor);

    // 2. Dual concentric circles
    float r1 = 23.0f * scale;
    drawStrokedCircle(renderer, cx, cy, r1, lineThick, baseColor, false);

    float r2 = 16.0f * scale;
    drawStrokedCircle(renderer, cx, cy, r2, lineThick, baseColor, false);

    // 3. Core dashed micro ring
    float r3 = 6.0f * scale;
    drawStrokedCircle(renderer, cx, cy, r3, lineThick * 0.8f, baseColor, true);

    // 4. Energy pulse wave ring
    if (pulseProgress > 0.001f) {
        float spreadR = (6.0f + 25.2f * pulseProgress) * scale;
        M3Color spreadCol = baseColor;
        spreadCol.a *= (1.0f - pulseProgress) * 0.8f;
        float spreadThick = (1.0f - 0.7f * pulseProgress) * lineThick;
        drawStrokedCircle(renderer, cx, cy, spreadR, spreadThick, spreadCol, true);
    }

    // 5. Twelve Greek glyph runes ring
    static const char* greeks[12] = {
        "\xCF\x80", "\xCE\xBB", "\xCE\xB8", "\xCF\x89",
        "\xCE\xB4", "\xCF\x86", "\xCF\x88", "\xCF\x83",
        "\xCE\xB1", "\xCE\xB2", "\xCE\xB3", "\xCE\xB7"
    };

    float greekR = 19.5f * scale;
    float fontSize = std::max(5.0f, 6.8f * scale);

    for (int i = 0; i < 12; ++i) {
        float angDeg = greekAngle + i * 30.0f;
        float rad = angDeg * 3.14159265f / 180.0f;
        float gx = cx + greekR * std::sin(rad);
        float gy = cy - greekR * std::cos(rad);

        std::string txt = greeks[i];
        float tw = renderer.getTextWidth(txt, fontSize);
        renderer.drawText(txt, gx - tw * 0.5f, gy - fontSize * 0.40f, fontSize, baseColor);
    }
}

void StarrySkyFormation::render(MaterialShader& renderer, MaterialTheme& theme) {
    float cx = x + width * 0.5f;
    float cy = y + height * 0.5f;
    float radius = (m_size * 0.5f) * m_scale;

    M3Color baseColor;
    if (m_level == FormationLevel::Active) {
        baseColor = { 26.0f / 255.0f, 115.0f / 255.0f, 232.0f / 255.0f, 1.0f };
    } else if (m_level == FormationLevel::Alert) {
        baseColor = { 255.0f / 255.0f, 128.0f / 255.0f, 171.0f / 255.0f, 1.0f };
    } else {
        baseColor = theme.onSurfaceVariant;
        baseColor.a = 0.45f;
    }

    drawMagicArray(renderer, cx, cy, radius, m_level, m_outerSpinAngle, m_greekRotateAngle, m_pulseTimer, baseColor);
}

bool StarrySkyFormation::handleMouseMove(float mx, float my) {
    if (m_isPressed || m_isDragging) {
        x = mx - m_dragOffsetX;
        y = my - m_dragOffsetY;
        requestUIWakeup(0.016);
        return true;
    }
    return isInside(mx, my);
}

bool StarrySkyFormation::handleMouseButton(int button, int action, float mx, float my) {
    if (button != 0) return false;

    if (action == 1) { 
        if (isInside(mx, my)) {
            m_isPressed = true;
            m_isDragging = true;
            m_downX = mx;
            m_downY = my;
            m_dragOffsetX = mx - x;
            m_dragOffsetY = my - y;
            m_pressDuration = 0.0f;
            m_hasFiredLongClick = false;
            return true;
        }
    } else if (action == 0 && m_isPressed) { 
        float totalMoveDist = std::hypot(mx - m_downX, my - m_downY);

        if (totalMoveDist < dp(6.0f) && !m_hasFiredLongClick) {
            m_clickCount++;
            if (m_clickCount == 1) {
                m_clickTimer = 0.26f;
            } else if (m_clickCount == 2) {
                m_clickTimer = 0.0f;
                m_clickCount = 0;
                if (m_onDoubleClick) m_onDoubleClick();
            }
        }
        m_isPressed = false;
        m_isDragging = false;
        return true;
    }
    return false;
}