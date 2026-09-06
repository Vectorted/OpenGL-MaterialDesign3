/**
 * @file MaterialSkeletonText.cpp
 * @brief Implementation of placeholder skeleton text loading lines.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "SkeletonText.hpp"

SkeletonText::SkeletonText(float w, float h)
    : View(0.0f, 0.0f, w, h) {
}

void SkeletonText::render(MaterialShader& renderer, MaterialTheme& theme) {
    float currentW = std::min(width, dp(layout_width));
    
    // Primary placeholder bar
    M3Color mainCol = lerpColor(theme.onSurface, M3Color{ 0,0,0,0 }, 0.5f);
    renderer.drawM3UI(x, y, currentW, dp(8.0f), dp(4.0f), dp(4.0f), dp(4.0f), dp(4.0f), mainCol);

    // Secondary sub-line bar
    M3Color subCol = lerpColor(theme.onSurfaceVariant, M3Color{ 0,0,0,0 }, 0.7f);
    renderer.drawM3UI(x, y + dp(12.0f), currentW * 0.6f, dp(4.0f), dp(2.0f), dp(2.0f), dp(2.0f), dp(2.0f), subCol);
}

SkeletonTextBuilder& SkeletonTextBuilder::size(float w, float h) { 
    m_w = w; 
    m_h = h; 
    return *this; 
}

SkeletonText* SkeletonTextBuilder::build() {
    return new SkeletonText(m_w, m_h);
}