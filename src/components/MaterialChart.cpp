/**
 * @file MaterialChart.cpp
 * @brief Implementation of MaterialChart rendering, kinetic tooltips, anti-aliased line drawing, and data animation.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "MaterialChart.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

extern void requestUIWakeup(double seconds);

/**
 * @brief Renders a line segment using a two-pass subpixel anti-aliasing technique.
 * 
 * @param renderer MaterialShader renderer instance.
 * @param x1 Start coordinate X.
 * @param y1 Start coordinate Y.
 * @param x2 End coordinate X.
 * @param y2 End coordinate Y.
 * @param thickness Line thickness in pixels.
 * @param color Line stroke color.
 */
static void drawSmoothLineSegment(MaterialShader& renderer, float x1, float y1, float x2, float y2, float thickness, const M3Color& color) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 0.001f) return;

    float lineAngle = std::atan2(dy, dx);
    float cx = (x1 + x2) * 0.5f;
    float cy = (y1 + y2) * 0.5f;

    M3Color zeroCol;
    zeroCol.r = 0.0f; zeroCol.g = 0.0f; zeroCol.b = 0.0f; zeroCol.a = 0.0f;

    /** Pass 1: Subpixel Anti-Aliasing Fringe */
    float aaThick = thickness + 1.2f;
    float aaR = aaThick * 0.5f;
    float aaW = dist + 1.0f;
    M3Color aaColor = color;
    aaColor.a *= 0.32f;

    renderer.drawM3UI(
        cx - aaW * 0.5f, cy - aaThick * 0.5f, aaW, aaThick,
        aaR, aaR, aaR, aaR, aaColor,
        0.0f, 0.0f, 0.0f, 0.0f, zeroCol,
        0, 0.0f, zeroCol, zeroCol, 0.0f,
        0, false, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        lineAngle, cx, cy
    );

    /** Pass 2: Solid Opaque Core Line */
    float coreThick = std::max(0.6f, thickness - 0.4f);
    float coreR = coreThick * 0.5f;
    float coreW = dist;

    renderer.drawM3UI(
        cx - coreW * 0.5f, cy - coreThick * 0.5f, coreW, coreThick,
        coreR, coreR, coreR, coreR, color,
        0.0f, 0.0f, 0.0f, 0.0f, zeroCol,
        0, 0.0f, zeroCol, zeroCol, 0.0f,
        0, false, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        lineAngle, cx, cy
    );
}

MaterialChart::MaterialChart(ChartType type) : m_type(type), m_chartType(type) {
    layout_width = MATCH_PARENT;
    layout_height = dp(240.0f);
}

void MaterialChart::setXAxisLabels(const std::vector<std::string>& labels) {
    m_labels = labels;
    m_xLabels = labels;
}

void MaterialChart::setData(const std::vector<float>& values, const std::vector<std::string>& labels) {
    m_values = values;
    m_labels = labels;
    m_xLabels = labels;

    m_seriesList.clear();
    ChartSeries s;
    s.name = "Series 1";
    s.data = values;
    m_seriesList.push_back(s);

    m_animProgress.assign(values.size(), 0.0f);
    m_hoverAnims.assign(values.size(), 0.0f);
    
    m_maxValue = 0.1f; 
    for (float v : values) {
        if (v > m_maxValue) m_maxValue = v;
    }
    m_masterAnim = 0.0f;
    m_hasTipInit = false;
}

void MaterialChart::setMultiSeries(const std::vector<ChartSeries>& seriesList, const std::vector<std::string>& xLabels) {
    m_seriesList = seriesList;
    m_xLabels = xLabels;
    m_labels = xLabels;

    size_t count = xLabels.size();
    if (!seriesList.empty()) {
        m_values = seriesList[0].data;
    } else {
        m_values.clear();
    }

    m_animProgress.assign(count, 0.0f);
    m_hoverAnims.assign(count, 0.0f);

    m_maxValue = 0.1f;
    for (const auto& s : m_seriesList) {
        for (float v : s.data) {
            if (v > m_maxValue) m_maxValue = v;
        }
    }
    m_masterAnim = 0.0f;
    m_hasTipInit = false;
}

std::vector<M3Color> MaterialChart::getM3Palette(const MaterialTheme& theme) {
    return {
        theme.primary,
        theme.tertiary,
        theme.secondary,
        { 0.98f, 0.52f, 0.32f, 1.0f },
        { 0.23f, 0.64f, 0.45f, 1.0f }
    };
}

void MaterialChart::drawDashedLine(MaterialShader& renderer, float x1, float y1, float x2, float y2, float dashLen, float gapLen, const M3Color& color, float thickness) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 0.001f) return;

    float nx = dx / dist;
    float ny = dy / dist;
    float cur = 0.0f;

    while (cur < dist) {
        float drawLen = std::min(dashLen, dist - cur);
        float cx1 = x1 + nx * cur;
        float cy1 = y1 + ny * cur;
        float cx2 = cx1 + nx * drawLen;
        float cy2 = cy1 + ny * drawLen;

        drawSmoothLineSegment(renderer, cx1, cy1, cx2, cy2, thickness, color);
        cur += dashLen + gapLen;
    }
}

void MaterialChart::update(float dt) {
    View::update(dt);
    const float dtSafe = std::min(dt, 0.033f);
    bool isAnimating = false;
    
    if (m_masterAnim < 0.999f) {
        m_masterAnim += (1.0f - m_masterAnim) * 10.0f * dtSafe;
        if (m_masterAnim >= 0.999f) m_masterAnim = 1.0f;
        isAnimating = true;
    }

    const float staggerDelay = 0.05f; 
    for (size_t i = 0; i < m_animProgress.size(); ++i) {
        float startDelay = i * staggerDelay;
        if (m_masterAnim > startDelay && m_animProgress[i] < 0.999f) {
            m_animProgress[i] += (1.0f - m_animProgress[i]) * 12.0f * dtSafe;
            if (m_animProgress[i] >= 0.999f) m_animProgress[i] = 1.0f;
            isAnimating = true;
        }

        float targetHover = (i == (size_t)m_hoveredIndex) ? 1.0f : 0.0f;
        float diffHover = targetHover - m_hoverAnims[i];
        if (std::abs(diffHover) > 0.002f) {
            m_hoverAnims[i] += diffHover * 16.0f * dtSafe;
            if (std::abs(targetHover - m_hoverAnims[i]) <= 0.002f) {
                m_hoverAnims[i] = targetHover;
            }
            isAnimating = true;
        }
    }

    const float targetTipAlpha = (m_hoveredIndex != -1) ? 1.0f : 0.0f;
    const float diffAlpha = targetTipAlpha - m_animTipAlpha;
    if (std::abs(diffAlpha) > 0.002f) {
        m_animTipAlpha += diffAlpha * 16.0f * dtSafe;
        if (std::abs(targetTipAlpha - m_animTipAlpha) <= 0.002f) {
            m_animTipAlpha = targetTipAlpha;
        }
        isAnimating = true;
    }

    size_t count = m_labels.size();
    if (count == 0 && !m_seriesList.empty()) count = m_seriesList[0].data.size();

    if (m_hoveredIndex != -1 && m_hoveredIndex < (int)count) {
        const float pL = dp(48.0f), pb = dp(32.0f), pt = dp(24.0f); 
        const float graphW = width - pL - dp(16.0f);
        const float graphH = height - pt - pb;
        const float step = graphW / count;
        
        const float animP = (m_hoveredIndex < (int)m_animProgress.size()) ? m_animProgress[m_hoveredIndex] : 1.0f;
        const float springEase = 1.0f - std::pow(1.0f - animP, 3.0f); 
        const float desiredTargetX = pL + step * m_hoveredIndex + step / 2.0f;
        
        const float curVal = (!m_values.empty() && m_hoveredIndex < (int)m_values.size()) ? m_values[m_hoveredIndex] : 
                             (!m_seriesList.empty() && m_hoveredIndex < (int)m_seriesList[0].data.size()) ? m_seriesList[0].data[m_hoveredIndex] : 0.0f;

        const float desiredTargetY = pt + graphH - ((curVal / m_maxValue) * graphH * springEase);
        const float desiredTargetVal = curVal;
        
        /** Fixed-time cubic transition setup */
        if (!m_hasTipInit || m_animTipAlpha < 0.05f) {
            m_hasTipInit = true;
            m_tipStartX = desiredTargetX;
            m_tipStartY = desiredTargetY;
            m_tipStartVal = desiredTargetVal;
            m_tipTargetX = desiredTargetX;
            m_tipTargetY = desiredTargetY;
            m_tipTargetVal = desiredTargetVal;
            m_tipProgress = 1.0f;

            m_animPointerX = desiredTargetX;
            m_animTipRelX = desiredTargetX;
            m_animTipRelY = desiredTargetY;
            m_animTipValue = desiredTargetVal;
        } else if (std::abs(desiredTargetX - m_tipTargetX) > 0.5f || std::abs(desiredTargetY - m_tipTargetY) > 0.5f) {
            m_tipStartX = m_animTipRelX;
            m_tipStartY = m_animTipRelY;
            m_tipStartVal = m_animTipValue;
            m_tipTargetX = desiredTargetX;
            m_tipTargetY = desiredTargetY;
            m_tipTargetVal = desiredTargetVal;
            m_tipProgress = 0.0f;
        }

        if (m_tipProgress < 1.0f) {
            m_tipProgress += dtSafe / 0.22f; // Smooth, deterministic 220ms ease-out transition
            if (m_tipProgress >= 1.0f) {
                m_tipProgress = 1.0f;
                m_animPointerX = m_tipTargetX;
                m_animTipRelX = m_tipTargetX;
                m_animTipRelY = m_tipTargetY;
                m_animTipValue = m_tipTargetVal;
            } else {
                float p = m_tipProgress;
                float easeOut = 1.0f - std::pow(1.0f - p, 3.0f);
                m_animPointerX = m_tipStartX + (m_tipTargetX - m_tipStartX) * easeOut;
                m_animTipRelX = m_animPointerX;
                m_animTipRelY = m_tipStartY + (m_tipTargetY - m_tipStartY) * easeOut;
                m_animTipValue = m_tipStartVal + (m_tipTargetVal - m_tipStartVal) * easeOut;
                isAnimating = true;
            }
        } else {
            m_animPointerX = m_tipTargetX;
            m_animTipRelX = m_tipTargetX;
            m_animTipRelY = m_tipTargetY;
            m_animTipValue = m_tipTargetVal;
        }
    } else {
        m_hasTipInit = false;
    }

    if (isAnimating) {
        requestUIWakeup(0.016);
    }
}

bool MaterialChart::handleMouseMove(float mx, float my) {
    bool inside = isInside(mx, my);
    int prevHover = m_hoveredIndex;
    m_hoveredIndex = -1;
    
    size_t count = m_labels.size();
    if (count == 0 && !m_seriesList.empty()) count = m_seriesList[0].data.size();

    if (inside && count > 0) {
        float paddingL = dp(48.0f), paddingR = dp(16.0f);
        float graphW = width - paddingL - paddingR;
        float step = graphW / count;
        
        int index = static_cast<int>((mx - (x + paddingL)) / step);
        if (index >= 0 && index < (int)count) {
            m_hoveredIndex = index;
        }
    }

    if (m_hoveredIndex != prevHover) {
        requestUIWakeup(0.016);
    }
    return inside;
}

void MaterialChart::render(MaterialShader& renderer, MaterialTheme& theme) {
    size_t count = m_labels.size();
    if (count == 0 && !m_seriesList.empty()) count = m_seriesList[0].data.size();
    if (count == 0) return;

    const float pL = dp(48.0f), pb = dp(32.0f), pt = dp(24.0f); 
    const float graphW = width - pL - dp(16.0f);
    const float graphH = height - pt - pb;

    auto palette = getM3Palette(theme);

    M3Color gridColor = theme.outlineVariant; 
    gridColor.a = 0.3f * m_masterAnim;
    M3Color axisColor = theme.onSurfaceVariant; 
    axisColor.a *= m_masterAnim;

    /** 1. Grid and Y Axis Labels */
    for (int i = 0; i <= 2; ++i) {
        float rowY = std::round(y + pt + graphH - (graphH * (i / 2.0f)));
        renderer.drawM3UI(std::round(x + pL), rowY, std::round(graphW), std::floor(dp(1.0f)), 0, 0, 0, 0, gridColor);

        std::stringstream ss; 
        ss << std::fixed << std::setprecision(0) << (m_maxValue * (i / 2.0f));
        float txtW = renderer.getTextWidth(ss.str(), dp(10.0f));
        renderer.drawText(ss.str(), std::round(x + pL - txtW - dp(8.0f)), std::round(rowY - dp(10.0f) * 0.45f), dp(10.0f), axisColor);
    }

    const float step = graphW / count;
    const float barWidth = std::round(std::min(step * 0.6f, dp(32.0f))); 

    /** 2. Axis Pointer Line */
    if (m_animTipAlpha > 0.01f && m_pointerStyle != AxisPointerStyle::None) {
        float px = std::round(x + m_animPointerX);
        M3Color pointerCol = theme.primary; 
        pointerCol.a = 0.55f * m_animTipAlpha;

        if (m_pointerStyle == AxisPointerStyle::Dashed) {
            drawDashedLine(renderer, px, y + pt, px, y + pt + graphH, dp(3.5f), dp(2.5f), pointerCol, dp(1.5f));
        } else if (m_pointerStyle == AxisPointerStyle::Solid) {
            renderer.drawM3UI(px - dp(0.75f), y + pt, dp(1.5f), graphH, 0, 0, 0, 0, pointerCol);
        }
    }

    std::vector<float> pointXs(count);
    std::vector<float> pointYs(count);

    for (size_t i = 0; i < count; ++i) {
        float animP = (i < m_animProgress.size()) ? m_animProgress[i] : 1.0f;
        float springEase = 1.0f - std::pow(1.0f - animP, 3.0f); 
        pointXs[i] = std::round(x + pL + step * i + step / 2.0f);

        float curV = (!m_values.empty() && i < m_values.size()) ? m_values[i] : 
                     (!m_seriesList.empty() && i < m_seriesList[0].data.size()) ? m_seriesList[0].data[i] : 0.0f;
        pointYs[i] = std::round(y + pt + graphH - ((curV / m_maxValue) * graphH * springEase));

        if (i < m_labels.size()) {
            float txtW = renderer.getTextWidth(m_labels[i], dp(11.0f));
            M3Color labelCol = theme.onSurfaceVariant;
            if (i < m_hoverAnims.size() && m_hoverAnims[i] > 0.01f) {
                labelCol = lerpColor(labelCol, theme.primary, m_hoverAnims[i]);
            }
            renderer.drawText(m_labels[i], std::round(pointXs[i] - txtW / 2.0f), std::round(y + height - dp(16.0f)), dp(11.0f), labelCol);
        }
    }

    /** 3. Bar Charts Rendering */
    if (m_type == ChartType::Bar || m_chartType == ChartType::Bar || m_chartType == ChartType::PillBar || m_chartType == ChartType::GroupedBar) {
        float exactBaseHeight = std::round(y + pt + graphH); 

        if (m_seriesList.size() <= 1) {
            for (size_t i = 0; i < count; ++i) {
                float valH = exactBaseHeight - pointYs[i] - std::floor(dp(1.0f));
                if (valH < 0.1f) valH = 0.1f;

                M3Color bCol = theme.primary;
                if (i < m_hoverAnims.size() && m_hoverAnims[i] > 0.01f) {
                    bCol = lerpColor(bCol, theme.primaryContainer, -m_hoverAnims[i] * 0.3f); 
                }
                
                float topCorner = (m_barStyle == ChartBarStyle::Pill || m_chartType == ChartType::PillBar) ? (barWidth / 2.0f) : dp(4.0f);
                float barStartX = std::round(pointXs[i] - barWidth / 2.0f);
                renderer.drawM3UI(barStartX, pointYs[i], barWidth, valH, topCorner, topCorner, 0, 0, bCol);
            }
        } else {
            float totalSeries = (float)m_seriesList.size();
            float groupWidth = step * 0.75f;
            float singleBarW = std::min(groupWidth / totalSeries, dp(24.0f));

            for (size_t sIdx = 0; sIdx < m_seriesList.size(); ++sIdx) {
                auto& series = m_seriesList[sIdx];
                M3Color sColor = (series.color.a > 0.01f) ? series.color : palette[sIdx % palette.size()];

                for (size_t i = 0; i < series.data.size(); ++i) {
                    float animP = (i < m_animProgress.size()) ? m_animProgress[i] : 1.0f;
                    float springEase = 1.0f - std::pow(1.0f - animP, 3.0f);
                    float valH = ((series.data[i] / m_maxValue) * graphH * springEase);
                    if (valH < 0.1f) valH = 0.1f;

                    float centerX = pointXs[i];
                    float groupStartX = centerX - (totalSeries * singleBarW) * 0.5f;
                    float bx = groupStartX + sIdx * singleBarW;
                    float by = exactBaseHeight - valH;

                    float topCorner = (m_chartType == ChartType::PillBar || m_barStyle == ChartBarStyle::Pill) ? (singleBarW * 0.5f) : dp(4.0f);
                    renderer.drawM3UI(bx, by, singleBarW - dp(2.0f), valH, topCorner, topCorner, 0, 0, sColor);
                }
            }
        }
    } 
    /** 4. Line Charts Rendering */
    else if (m_type == ChartType::Line || m_chartType == ChartType::Line || m_chartType == ChartType::SmoothLine ||
             m_chartType == ChartType::AreaLine || m_chartType == ChartType::StepLine) {
        
        float lineThick = dp(3.0f);

        for (size_t sIdx = 0; sIdx < m_seriesList.size(); ++sIdx) {
            auto& series = m_seriesList[sIdx];
            M3Color sColor = (series.color.a > 0.01f) ? series.color : (m_seriesList.size() <= 1 ? theme.primary : palette[sIdx % palette.size()]);

            std::vector<std::pair<float, float>> pts(series.data.size());
            for (size_t i = 0; i < series.data.size(); ++i) {
                float animP = (i < m_animProgress.size()) ? m_animProgress[i] : 1.0f;
                float springEase = 1.0f - std::pow(1.0f - animP, 3.0f);
                float px = pointXs[i];
                float py = y + pt + graphH - ((series.data[i] / m_maxValue) * graphH * springEase);
                pts[i] = { px, py };
            }

            /** 4.1. Area Line Fill */
            if (m_chartType == ChartType::AreaLine) {
                float baseY = y + pt + graphH;
                M3Color areaCol = sColor; 
                areaCol.a = 0.22f * m_masterAnim;

                for (size_t i = 0; i < pts.size() - 1; ++i) {
                    float x1 = pts[i].first, y1 = pts[i].second;
                    float x2 = pts[i + 1].first, y2 = pts[i + 1].second;
                    float fillW = x2 - x1;
                    
                    if (fillW > 0.5f) {
                        float sliceSteps = std::max(1.0f, std::round(fillW / dp(1.0f)));
                        float sliceW = fillW / sliceSteps;
                        for (float s = 0; s < sliceSteps; s += 1.0f) {
                            float t1 = s / sliceSteps;
                            float t2 = (s + 1.0f) / sliceSteps;
                            float curY = y1 + (y2 - y1) * (t1 + t2) * 0.5f;
                            float curH = baseY - curY;
                            if (curH > 0.5f) {
                                renderer.drawM3UI(x1 + s * sliceW, baseY - curH, sliceW + dp(0.5f), curH, 0, 0, 0, 0, areaCol);
                            }
                        }
                    }
                }
            }

            /** 4.2. Line segments rendering */
            for (size_t i = 0; i < pts.size() - 1; ++i) {
                float animP_next = (i + 1 < m_animProgress.size()) ? m_animProgress[i + 1] : 1.0f;
                if (animP_next > 0.01f) {
                    float x1 = pts[i].first, y1 = pts[i].second;
                    float x2 = pts[i + 1].first, y2 = pts[i + 1].second;

                    if (m_chartType == ChartType::StepLine) {
                        drawSmoothLineSegment(renderer, x1, y1 - lineThick * 0.5f, x2, y1 - lineThick * 0.5f, lineThick, sColor);
                        drawSmoothLineSegment(renderer, x2, y1, x2, y2, lineThick, sColor);
                    } else {
                        float dx = x2 - x1; 
                        float dy = y2 - y1;
                        float dist = std::sqrt(dx * dx + dy * dy);

                        if (dist > 0.1f) {
                            float gap = (m_lineStyle == ChartLineStyle::Gapped) ? dp(12.0f) : 0.0f;
                            float tStart = (gap > 0.0f) ? (gap / dist) : 0.0f;
                            float tEnd = (gap > 0.0f) ? (1.0f - (gap / dist)) : 1.0f;

                            if (tStart < tEnd) {
                                float sx1 = x1 + dx * tStart;
                                float sy1 = y1 + dy * tStart;
                                float sx2 = x1 + dx * tEnd;
                                float sy2 = y1 + dy * tEnd;

                                drawSmoothLineSegment(renderer, sx1, sy1, sx2, sy2, lineThick, sColor);
                            }
                        }
                    }
                }
            }

            /** 4.3. Data Points Nodes */
            for (size_t i = 0; i < pts.size(); ++i) {
                float hAnim = (i < m_hoverAnims.size()) ? m_hoverAnims[i] : 0.0f;
                float dotR = dp(4.0f) + dp(2.5f) * hAnim;
                if (m_lineStyle == ChartLineStyle::Seamless) {
                    renderer.drawM3UI(pts[i].first - dotR, pts[i].second - dotR, dotR * 2.0f, dotR * 2.0f, dotR, dotR, dotR, dotR, theme.surface);
                    renderer.drawM3UI(pts[i].first - dotR * 0.8f, pts[i].second - dotR * 0.8f, dotR * 1.6f, dotR * 1.6f, dotR, dotR, dotR, dotR, sColor);
                } else {
                    M3Color nodeCol = theme.primaryContainer;
                    if (hAnim > 0.1f) nodeCol = lerpColor(nodeCol, sColor, hAnim);
                    renderer.drawM3UI(pts[i].first - dotR, pts[i].second - dotR, dotR * 2.0f, dotR * 2.0f, dotR, dotR, dotR, dotR, nodeCol);
                    M3Color innerCol = sColor;
                    if (hAnim > 0.1f) innerCol = lerpColor(innerCol, theme.onPrimary, hAnim);
                    renderer.drawM3UI(pts[i].first - dotR * 0.5f, pts[i].second - dotR * 0.5f, dotR, dotR, dotR * 0.5f, dotR * 0.5f, dotR * 0.5f, dotR * 0.5f, innerCol);
                }
            }
        }
    }

    /** 5. Tooltip Projections with pixel-snapping to prevent text jitter */
    if (m_animTipAlpha > 0.01f) {
        if (m_seriesList.size() <= 1) {
            float alpha = m_animTipAlpha;
            
            float targetVal = (m_hoveredIndex >= 0 && m_hoveredIndex < (int)m_values.size()) ? m_values[m_hoveredIndex] : 
                              (m_hoveredIndex >= 0 && !m_seriesList.empty() && m_hoveredIndex < (int)m_seriesList[0].data.size()) ? m_seriesList[0].data[m_hoveredIndex] : m_animTipValue;

            std::stringstream ss; 
            ss << std::fixed << std::setprecision(1) << targetVal;
            std::string tip = ss.str();

            float tipW = std::round(std::max(dp(36.0f), renderer.getTextWidth(tip, dp(12.0f)) + dp(16.0f)));
            float tipH = std::round(dp(28.0f));

            float curTipX = x + m_animTipRelX;
            float curTipY = y + m_animTipRelY;

            float finalTipX = std::round(curTipX - tipW / 2.0f);
            float finalTipY = std::round(curTipY - tipH - dp(12.0f) + (1.0f - alpha) * dp(10.0f)); 

            M3Color tipBg = theme.onSurface; 
            tipBg.a *= alpha;
            M3Color tipTx = theme.surface; 
            tipTx.a *= alpha;

            renderer.drawM3UI(finalTipX, finalTipY, tipW, tipH, dp(6.0f), dp(6.0f), dp(6.0f), dp(6.0f), tipBg);
            renderer.drawText(tip, std::round(finalTipX + dp(8.0f)), std::round(finalTipY + tipH / 2.0f - dp(12.0f) * 0.45f), dp(12.0f), tipTx);
        } else if (m_hoveredIndex >= 0 && m_hoveredIndex < (int)count) {
            float tipW = std::round(dp(150.0f));
            float tipH = std::round(dp(28.0f) + m_seriesList.size() * dp(20.0f));
            float destX = std::round(x + m_animTipRelX + dp(12.0f));
            float destY = std::round(y + pt + dp(10.0f));

            if (destX + tipW > x + width) destX -= (tipW + dp(24.0f));

            M3Color tipBg = theme.onSurface; 
            tipBg.a *= m_animTipAlpha;
            M3Color tipTxt = theme.surface; 
            tipTxt.a *= m_animTipAlpha;

            renderer.drawM3UI(destX, destY, tipW, tipH, dp(10.0f), dp(10.0f), dp(10.0f), dp(10.0f), tipBg);
            if (m_hoveredIndex < (int)m_labels.size()) {
                renderer.drawText(m_labels[m_hoveredIndex], std::round(destX + dp(12.0f)), std::round(destY + dp(8.0f)), dp(12.0f), tipTxt);
            }

            for (size_t s = 0; s < m_seriesList.size(); ++s) {
                float rowY = std::round(destY + dp(28.0f) + s * dp(18.0f));
                M3Color sColor = (m_seriesList[s].color.a > 0.01f) ? m_seriesList[s].color : palette[s % palette.size()];
                sColor.a *= m_animTipAlpha;

                renderer.drawM3UI(std::round(destX + dp(12.0f)), std::round(rowY + dp(3.0f)), dp(8.0f), dp(8.0f), dp(4.0f), dp(4.0f), dp(4.0f), dp(4.0f), sColor);

                float val = (m_hoveredIndex < (int)m_seriesList[s].data.size()) ? m_seriesList[s].data[m_hoveredIndex] : 0.0f;
                std::stringstream ssv;
                ssv << std::fixed << std::setprecision(1) << val;
                std::string itemStr = m_seriesList[s].name + ": " + ssv.str();
                renderer.drawText(itemStr, std::round(destX + dp(26.0f)), rowY, dp(11.0f), tipTxt);
            }
        }
    }
}