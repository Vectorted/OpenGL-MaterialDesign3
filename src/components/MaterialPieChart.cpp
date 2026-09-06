/**
 * @file MaterialPieChart.cpp
 * @brief High-precision hardware-accelerated polar bar chart with range slices, average lines, and custom units.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "MaterialPieChart.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

extern void requestUIWakeup(double seconds); 

static GLuint g_realEstateProgram = 0;
static GLuint g_realEstateVAO = 0;
static GLuint g_realEstateVBO = 0;

M3Color MaterialPieChart::hexToColor(uint32_t hex, float alpha) {
    float r = ((hex >> 16) & 0xFF) / 255.0f;
    float g = ((hex >> 8) & 0xFF) / 255.0f;
    float b = (hex & 0xFF) / 255.0f;
    return { r, g, b, alpha };
}

M3Color MaterialPieChart::parseColor(const std::string& str, float defaultAlpha) {
    if (str.empty()) return { 0.0f, 0.0f, 0.0f, 0.0f };

    std::string s = str;
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(s.begin());
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();

    /** Parse rgb(...) and rgba(...) color definitions */
    if (s.rfind("rgb", 0) == 0) {
        int r = 0, g = 0, b = 0;
        float a = defaultAlpha;
        if (s.find("rgba") == 0) {
            if (sscanf(s.c_str(), "rgba(%d,%d,%d,%f)", &r, &g, &b, &a) >= 3 ||
                sscanf(s.c_str(), "rgba(%d, %d, %d, %f)", &r, &g, &b, &a) >= 3) {
                return { r / 255.0f, g / 255.0f, b / 255.0f, a };
            }
        } else {
            if (sscanf(s.c_str(), "rgb(%d,%d,%d)", &r, &g, &b) == 3 ||
                sscanf(s.c_str(), "rgb(%d, %d, %d)", &r, &g, &b) == 3) {
                return { r / 255.0f, g / 255.0f, b / 255.0f, defaultAlpha };
            }
        }
    }

    /** Parse Hex color format variations */
    if (s.rfind("#", 0) == 0) s = s.substr(1);
    else if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) s = s.substr(2);

    bool isHex = true;
    for (char c : s) {
        if (!isxdigit(c)) { isHex = false; break; }
    }

    if (isHex) {
        if (s.length() == 6) {
            unsigned int num = 0;
            std::stringstream ss; ss << std::hex << s; ss >> num;
            return hexToColor(num, defaultAlpha);
        } else if (s.length() == 8) {
            unsigned int num = 0;
            std::stringstream ss; ss << std::hex << s; ss >> num;
            float r = ((num >> 24) & 0xFF) / 255.0f;
            float g = ((num >> 16) & 0xFF) / 255.0f;
            float b = ((num >> 8) & 0xFF) / 255.0f;
            float a = (num & 0xFF) / 255.0f;
            return { r, g, b, a };
        } else if (s.length() == 3) {
            int r = 0, g = 0, b = 0;
            sscanf(s.c_str(), "%1x%1x%1x", &r, &g, &b);
            return { (r * 17) / 255.0f, (g * 17) / 255.0f, (b * 17) / 255.0f, defaultAlpha };
        }
    }

    return { 0.0f, 0.0f, 0.0f, 0.0f };
}

void MaterialPieChart::setColors(const std::string& barHex, const std::string& avgHex) {
    m_barColor = parseColor(barHex, 0.88f);
    m_avgColor = parseColor(avgHex, 1.00f);
}

void MaterialPieChart::setBarColor(const std::string& barHex) {
    m_barColor = parseColor(barHex, 0.88f);
}

void MaterialPieChart::setAvgColor(const std::string& avgHex) {
    m_avgColor = parseColor(avgHex, 1.00f);
}

void MaterialPieChart::setColors(uint32_t barHex, uint32_t avgHex, float alpha) {
    m_barColor = hexToColor(barHex, alpha);
    m_avgColor = hexToColor(avgHex, 1.0f);
}

void MaterialPieChart::setPalette(const std::vector<std::string>& hexColors) {
    m_palette.clear();
    for (const auto& hex : hexColors) {
        M3Color c = parseColor(hex, 0.88f);
        if (c.a > 0.0f) m_palette.push_back(c);
    }
}

MaterialPieChart::MaterialPieChart() {
    layout_width = MATCH_PARENT;
    layout_height = MATCH_PARENT;
}

void MaterialPieChart::initPolarShader() {
    if (g_realEstateProgram != 0) return;

    const char* vsCode = R"(#version 300 es
        layout(location = 0) in vec2 aPos;
        out vec2 v_uv;
        uniform vec2 u_offset;
        uniform vec2 u_scale;
        uniform vec2 u_resolution; 
        void main() {
            v_uv = aPos * 0.5 + 0.5; 
            vec2 screenPos = u_offset + (aPos * 0.5 + 0.5) * u_scale;
            vec2 ndc = (screenPos / u_resolution) * 2.0 - 1.0;
            ndc.y = -ndc.y; 
            gl_Position = vec4(ndc, 0.0, 1.0);
        }
    )";

    const char* fsCode = R"(#version 300 es
        precision highp float;
        in vec2 v_uv;
        out vec4 FragColor;

        uniform float u_AnimSweep;
        uniform int u_ItemCount;
        uniform vec3 u_Ranges[32];      // x: normMin, y: normMax, z: normAvg
        uniform vec4 u_BarColors[32];
        uniform float u_HoverOffsets[32];
        uniform vec4 u_AvgLineColor;
        uniform vec4 u_GridLineColor;
        uniform vec4 u_AxisLineColor;
        uniform vec2 u_scale; 

        const float PI = 3.141592653589793;
        const float TAU = 6.283185307179586;

        void main() {
            vec2 p = v_uv * 2.0 - 1.0; 
            float r = length(p);
            
            float pixelRadius = 0.5 * min(u_scale.x, u_scale.y);
            float invPixel = 1.0 / max(pixelRadius, 1.0);

            vec4 finalColor = vec4(0.0);

            float rInner = 0.12;
            float rOuter = 0.85;

            // 1. Concentric grid rings
            for (int i = 0; i <= 5; ++i) {
                float ringR = rInner + (rOuter - rInner) * (float(i) / 5.0);
                float dRing = abs(r - ringR);
                float ringAlpha = clamp((1.0 * invPixel - dRing) * pixelRadius, 0.0, 1.0);
                if (ringAlpha > 0.0) {
                    finalColor = mix(finalColor, u_GridLineColor, ringAlpha * 0.75 * u_AnimSweep);
                }
            }

            // 2. 12 o'clock radial axis line
            if (p.y <= 0.0 && r >= rInner - invPixel && r <= rOuter + invPixel) {
                float dVert = abs(p.x);
                float vertAlpha = clamp((1.0 * invPixel - dVert) * pixelRadius, 0.0, 1.0);
                if (vertAlpha > 0.0) {
                    finalColor = mix(finalColor, u_AxisLineColor, vertAlpha * 0.85 * u_AnimSweep);
                }
            }

            // 3. Polar angle calculation
            float angle = atan(p.x, -p.y);
            if (angle < 0.0) angle += TAU;

            float sectorAngle = TAU / float(max(u_ItemCount, 1));
            float barAngleWidth = sectorAngle * 0.82; 

            // 4. Polar bar slice rendering
            float cornerR = 1.5 * invPixel;

            for (int i = 0; i < 32; ++i) {
                if (i >= u_ItemCount) break;

                float midAng = float(i) * sectorAngle + sectorAngle * 0.5;
                float da = angle - midAng;
                if (da < -PI) da += TAU;
                if (da > PI) da -= TAU;

                float arcDist = (abs(da) - barAngleWidth * 0.5) * max(r, 0.06);

                vec3 data = u_Ranges[i];
                float normMin = data.x;
                float normMax = data.y;
                float normAvg = data.z;
                float hover = u_HoverOffsets[i];

                float currentNormMax = normMin + (normMax - normMin) * u_AnimSweep;
                float targetNormMax = min(currentNormMax + hover * 0.045, 1.0);

                float barRMin = rInner + (rOuter - rInner) * normMin;
                float barRMax = min(rInner + (rOuter - rInner) * targetNormMax, rOuter);

                float radDist = max(barRMin - r, r - barRMax);

                float d_arc = arcDist - cornerR;
                float d_rad = radDist - cornerR;
                float barSDF = (d_arc > 0.0 && d_rad > 0.0) ? (length(vec2(d_arc, d_rad)) - cornerR) : (max(d_arc, d_rad) - cornerR);

                float barAlpha = clamp((-barSDF) * pixelRadius, 0.0, 1.0);
                float outerClip = clamp((rOuter - r) * pixelRadius, 0.0, 1.0);
                barAlpha *= outerClip;

                if (barAlpha > 0.0) {
                    vec4 col = u_BarColors[i];
                    if (hover > 0.0) {
                        col.rgb = mix(col.rgb, vec3(1.0), hover * 0.25);
                    }
                    finalColor = mix(finalColor, col, barAlpha);
                }

                // Average value arc indicator
                if (u_AnimSweep > 0.25 && normAvg > 0.0) {
                    float avgR = min(rInner + (rOuter - rInner) * normAvg, rOuter - invPixel);
                    float lineHalfW = 1.5 * invPixel;
                    float avgRadDist = abs(r - avgR) - lineHalfW;
                    float avgSDF = max(arcDist, avgRadDist);
                    float avgAlpha = clamp((-avgSDF) * pixelRadius, 0.0, 1.0) * outerClip;
                    if (avgAlpha > 0.0 && r <= barRMax && r >= barRMin) {
                        finalColor = mix(finalColor, u_AvgLineColor, avgAlpha);
                    }
                }
            }

            // 5. Sector boundary tick marks
            float daTick = mod(angle, sectorAngle);
            if (daTick > sectorAngle * 0.5) daTick -= sectorAngle;
            float tickArcDist = abs(daTick) * r;
            
            float tickLen = 0.028; 
            float tickRadDist = max(rOuter - r, r - (rOuter + tickLen));
            float tickSDF = max(tickArcDist - 0.65 * invPixel, tickRadDist);
            float tickAlpha = clamp((-tickSDF) * pixelRadius, 0.0, 1.0);
            if (tickAlpha > 0.0) {
                finalColor = mix(finalColor, u_AxisLineColor, tickAlpha * 0.90 * u_AnimSweep);
            }

            // 6. Outer circular boundary stroke
            float dOuter = abs(r - rOuter);
            float outerAlpha = clamp((1.0 * invPixel - dOuter) * pixelRadius, 0.0, 1.0);
            if (outerAlpha > 0.0) {
                finalColor = mix(finalColor, u_AxisLineColor, outerAlpha * 0.85 * u_AnimSweep);
            }

            if (finalColor.a <= 0.001) {
                discard;
            }
            FragColor = finalColor;
        }
    )";

    auto compileShader = [](GLuint type, const char* src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        return s;
    };

    GLuint vs = compileShader(GL_VERTEX_SHADER, vsCode);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsCode);
    g_realEstateProgram = glCreateProgram();
    glAttachShader(g_realEstateProgram, vs);
    glAttachShader(g_realEstateProgram, fs);
    glLinkProgram(g_realEstateProgram);
    
    glDeleteShader(vs); 
    glDeleteShader(fs);

    float vertices[] = { -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, -1.0, 1.0 };
    glGenVertexArrays(1, &g_realEstateVAO);
    glGenBuffers(1, &g_realEstateVBO);
    glBindVertexArray(g_realEstateVAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_realEstateVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void MaterialPieChart::setData(const std::vector<PolarBarItem>& items) {
    m_items = items;
    m_hoverAnims.assign(items.size(), 0.0f);
    m_hoveredIndex = -1;
    m_animSweep = 0.0f; 

    if (items.empty()) {
        m_maxScale = 100.0f;
        return;
    }

    float maxVal = 0.0001f;
    for (const auto& item : items) {
        if (item.maxVal > maxVal) maxVal = item.maxVal;
    }

    float exponent = std::pow(10.0f, std::floor(std::log10(maxVal)));
    float normalized = maxVal / exponent;
    float niceScale = 1.0f;
    if (normalized <= 1.0f) niceScale = 1.0f;
    else if (normalized <= 2.0f) niceScale = 2.0f;
    else if (normalized <= 5.0f) niceScale = 5.0f;
    else niceScale = 10.0f;

    m_maxScale = niceScale * exponent;
}

void MaterialPieChart::clearData() {
    m_items.clear();
    m_hoverAnims.clear();
    m_hoveredIndex = -1;
}

void MaterialPieChart::update(float dt) {
    View::update(dt);
    const float dtSafe = std::min(dt, 0.033f);

    m_animSweep += (1.0f - m_animSweep) * 5.0f * dtSafe;
    if (m_animSweep < 0.99f) requestUIWakeup(0.1); 

    for (size_t i = 0; i < m_hoverAnims.size(); ++i) {
        float target = (i == (size_t)m_hoveredIndex) ? 1.0f : 0.0f;
        m_hoverAnims[i] += (target - m_hoverAnims[i]) * 14.0f * dtSafe;
        if (std::abs(target - m_hoverAnims[i]) > 0.005f) requestUIWakeup(0.05);
    }

    const float tAlpha = (m_hoveredIndex != -1) ? 1.0f : 0.0f;
    m_tipAlpha += (tAlpha - m_tipAlpha) * 15.0f * dtSafe;

    if (m_hoveredIndex != -1 && m_hoveredIndex < (int)m_items.size()) {
        if (m_tipAlpha < 0.05f) { 
            m_tipRelX = m_targetTipRelX; 
            m_tipRelY = m_targetTipRelY; 
        } else {
            float dx = m_targetTipRelX - m_tipRelX;
            float dy = m_targetTipRelY - m_tipRelY;
            
            /** Anti-jitter: Snap directly to target position within deadzone threshold */
            if (std::abs(dx) < 0.2f && std::abs(dy) < 0.2f) {
                m_tipRelX = m_targetTipRelX;
                m_tipRelY = m_targetTipRelY;
            } else {
                m_tipRelX += dx * 20.0f * dtSafe;
                m_tipRelY += dy * 20.0f * dtSafe;
                requestUIWakeup(0.02);
            }
        }
    }
}

bool MaterialPieChart::handleMouseMove(float mx, float my) {
    bool inside = isInside(mx, my);
    m_hoveredIndex = -1;

    if (inside && !m_items.empty()) {
        /** Accurately track mouse in local coordinates directly from DPI-scaled frame coordinates */
        m_targetTipRelX = mx - x;
        m_targetTipRelY = my - y;

        float availW = width - dp(84.0f);
        float availH = height - dp(48.0f);
        float chartSize = std::max(dp(120.0f), std::min(availW, availH));

        float centerX = x + width / 2.0f;
        float centerY = y + dp(8.0f) + availH / 2.0f;

        float rx = mx - centerX;
        float ry = my - centerY;
        float radius = std::sqrt(rx * rx + ry * ry);
        
        float rInnerPx = (chartSize / 2.0f) * 0.12f;
        float rOuterPx = (chartSize / 2.0f) * 0.85f;

        if (radius >= rInnerPx && radius <= rOuterPx + dp(14.0f)) { 
            float angle = std::atan2(rx, -ry);
            if (angle < 0.0f) angle += 2.0f * 3.141592653589793f;
            
            int count = (int)m_items.size();
            float sectorAngle = (2.0f * 3.141592653589793f) / count;
            int idx = (int)(angle / sectorAngle);
            if (idx >= 0 && idx < count) {
                m_hoveredIndex = idx;
            }
        }
    }
    return inside;
}

void MaterialPieChart::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (m_items.empty()) return;
    if (!m_shaderReady) { initPolarShader(); m_shaderReady = true; }

    int fbW, fbH; 
    glfwGetFramebufferSize(glfwGetCurrentContext(), &fbW, &fbH); 

    const float availW = width - dp(84.0f); 
    const float availH = height - dp(48.0f);
    const float chartSize = std::max(dp(120.0f), std::min(availW, availH));

    const float centerX = std::round(x + width / 2.0f);
    const float centerY = std::round(y + dp(10.0f) + availH / 2.0f);
    const float drawX = std::round(centerX - chartSize / 2.0f);
    const float drawY = std::round(centerY - chartSize / 2.0f);

    renderer.end(); 
    
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean cullFaceEnabled  = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glUseProgram(g_realEstateProgram);
    glBindVertexArray(g_realEstateVAO);

    glUniform1f(glGetUniformLocation(g_realEstateProgram, "u_AnimSweep"), 1.0f - std::pow(1.0f - m_animSweep, 3.0f)); 
    glUniform1i(glGetUniformLocation(g_realEstateProgram, "u_ItemCount"), (int)m_items.size());

    float ranges[32 * 3] = {0.0f};
    float hovers[32] = {0.0f};
    float barColors[32 * 4] = {0.0f}; 

    for (size_t i = 0; i < m_items.size() && i < 32; ++i) {
        ranges[i * 3 + 0] = m_items[i].minVal / m_maxScale;
        ranges[i * 3 + 1] = m_items[i].maxVal / m_maxScale;
        ranges[i * 3 + 2] = m_items[i].avgVal / m_maxScale;
        hovers[i] = m_hoverAnims[i];

        M3Color itemCol = { 0.0f, 0.0f, 0.0f, 0.0f };
        if (!m_items[i].color.empty()) {
            itemCol = parseColor(m_items[i].color, 0.88f);
        }
        if (itemCol.a <= 0.001f) {
            if (!m_palette.empty()) {
                itemCol = m_palette[i % m_palette.size()];
            } else {
                itemCol = m_barColor;
            }
        }
        barColors[i * 4 + 0] = itemCol.r;
        barColors[i * 4 + 1] = itemCol.g;
        barColors[i * 4 + 2] = itemCol.b;
        barColors[i * 4 + 3] = itemCol.a;
    }
    
    M3Color gridColor = theme.onSurfaceVariant; 
    gridColor.a = 0.16f;
    M3Color axisColor = theme.onSurface; 
    axisColor.a = 0.45f;

    glUniform3fv(glGetUniformLocation(g_realEstateProgram, "u_Ranges[0]"), 32, ranges);
    glUniform4fv(glGetUniformLocation(g_realEstateProgram, "u_BarColors[0]"), 32, barColors); 
    glUniform1fv(glGetUniformLocation(g_realEstateProgram, "u_HoverOffsets[0]"), 32, hovers);
    glUniform4f(glGetUniformLocation(g_realEstateProgram, "u_AvgLineColor"), m_avgColor.r, m_avgColor.g, m_avgColor.b, m_avgColor.a);
    glUniform4f(glGetUniformLocation(g_realEstateProgram, "u_GridLineColor"), gridColor.r, gridColor.g, gridColor.b, gridColor.a);
    glUniform4f(glGetUniformLocation(g_realEstateProgram, "u_AxisLineColor"), axisColor.r, axisColor.g, axisColor.b, axisColor.a);

    glUniform2f(glGetUniformLocation(g_realEstateProgram, "u_offset"), drawX, drawY);
    glUniform2f(glGetUniformLocation(g_realEstateProgram, "u_scale"), chartSize, chartSize);
    glUniform2f(glGetUniformLocation(g_realEstateProgram, "u_resolution"), (float)fbW, (float)fbH);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
    if (cullFaceEnabled)  glEnable(GL_CULL_FACE);

    renderer.begin(fbW, fbH);

    /** 2. Radial axis scale tick numbers (Cleanly hugging the top edge of each grid ring line) */
    const float rInnerPx = (chartSize / 2.0f) * 0.12f;
    const float rOuterPx = (chartSize / 2.0f) * 0.85f;
    const float tickFontSize = dp(8.5f);
    
    M3Color tickTextColor = theme.onSurfaceVariant; 
    tickTextColor.a *= 0.85f;

    for (int i = 0; i <= 5; ++i) {
        float val = m_maxScale * (float(i) / 5.0f);
        float tickR = rInnerPx + (rOuterPx - rInnerPx) * (float(i) / 5.0f);

        std::stringstream ss;
        if (std::abs(val - std::round(val)) < 0.01f) ss << (int)val;
        else ss << std::fixed << std::setprecision(1) << val;
        std::string tickStr = ss.str();

        float tW = renderer.getTextWidth(tickStr, tickFontSize);
        
        /** Hug closely above the circular grid ring with 1dp boundary gap */
        float tickDrawX = std::round(centerX - tW - dp(4.0f));
        float tickDrawY = std::round(centerY - tickR - tickFontSize + dp(0.5f));

        renderer.drawText(tickStr, tickDrawX, tickDrawY, tickFontSize, tickTextColor);
    }

    /** 3. Outer category text labels */
    const float sectorAngle = (2.0f * 3.141592653589793f) / (float)m_items.size();
    for (size_t i = 0; i < m_items.size(); ++i) {
        float midAng = float(i) * sectorAngle + sectorAngle * 0.5f;

        float sinA = std::sin(midAng);
        float cosA = -std::cos(midAng);

        const std::string& itemLabel = m_items[i].label;
        float cW = renderer.getTextWidth(itemLabel, dp(10.0f));
        float cH = dp(10.0f);

        float textR = rOuterPx + dp(10.0f); 
        float tx = centerX + sinA * textR;
        float ty = centerY + cosA * textR;

        if (sinA > 0.25f) {
            tx += dp(2.0f);
            ty -= cH * 0.5f;
        } else if (sinA < -0.25f) {
            tx -= (cW + dp(2.0f));
            ty -= cH * 0.5f;
        } else {
            tx -= cW * 0.5f;
            if (cosA < 0.0f) ty -= (cH + dp(1.0f));
            else ty += dp(2.0f);
        }

        M3Color labelColor = (int(i) == m_hoveredIndex) ? theme.primary : theme.onSurface;
        renderer.drawText(itemLabel, std::round(tx), std::round(ty), dp(10.0f), labelColor);
    }

    /** 4. Bottom legend items */
    const float legendRowCenterY = std::round(y + height - dp(18.0f)); 

    const float rangeW = renderer.getTextWidth(m_rangeLegend, dp(11.0f));
    const float avgW = renderer.getTextWidth(m_avgLegend, dp(11.0f));

    const float iconW = dp(14.0f);
    const float iconH = dp(8.0f);       
    const float iconR = dp(2.0f);       
    const float gapIconText = dp(6.0f);
    const float gapItem = dp(24.0f);

    const float item1W = iconW + gapIconText + rangeW;
    const float item2W = iconW + gapIconText + avgW;
    const float totalLegendW = item1W + gapItem + item2W;

    const float startLX = std::round(centerX - totalLegendW / 2.0f);

    renderer.drawM3UI(startLX, std::round(legendRowCenterY - iconH * 0.5f), iconW, iconH, iconR, iconR, iconR, iconR, m_barColor);
    renderer.drawText(m_rangeLegend, std::round(startLX + iconW + gapIconText), std::round(legendRowCenterY - dp(5.5f)), dp(11.0f), theme.onSurface);

    const float avgItemStartX = std::round(startLX + item1W + gapItem);
    renderer.drawM3UI(avgItemStartX, std::round(legendRowCenterY - iconH * 0.5f), iconW, iconH, iconR, iconR, iconR, iconR, m_avgColor);
    renderer.drawText(m_avgLegend, std::round(avgItemStartX + iconW + gapIconText), std::round(legendRowCenterY - dp(5.5f)), dp(11.0f), theme.onSurface);

    /** 5. Hover tooltip popup with pixel-snapping and boundary clamp */
    if (m_tipAlpha > 0.01f && m_hoveredIndex >= 0 && m_hoveredIndex < (int)m_items.size()) {
        const float alpha = m_tipAlpha;
        const auto& it = m_items[m_hoveredIndex];

        std::string l1 = it.label;
        std::stringstream ss2, ss3;
        ss2 << m_rangeLegend << ": " << (int)it.minVal << " ~ " << (int)it.maxVal;
        if (!m_unit.empty()) ss2 << " " << m_unit;
        
        ss3 << m_avgLegend << ": " << std::fixed << std::setprecision(1) << it.avgVal;
        if (!m_unit.empty()) ss3 << " " << m_unit;

        std::string l2 = ss2.str();
        std::string l3 = ss3.str();

        float w1 = renderer.getTextWidth(l1, dp(12.0f));
        float w2 = renderer.getTextWidth(l2, dp(11.0f));
        float w3 = renderer.getTextWidth(l3, dp(11.0f));
        float tipW = std::round(std::max({ w1, w2, w3 }) + dp(24.0f));
        float tipH = dp(62.0f);

        float finalTipX = x + m_tipRelX + dp(12.0f) + (1.0f - alpha) * dp(10.0f);
        float finalTipY = y + m_tipRelY - tipH / 2.0f;

        finalTipX = std::clamp(finalTipX, x + dp(8.0f), x + width - tipW - dp(8.0f));
        finalTipY = std::clamp(finalTipY, y + dp(8.0f), y + height - tipH - dp(8.0f));

        finalTipX = std::round(finalTipX);
        finalTipY = std::round(finalTipY);

        M3Color tipBg = theme.onSurface; 
        tipBg.a *= alpha * 0.95f;
        M3Color tipTxA = theme.surface; 
        tipTxA.a *= alpha;
        M3Color tipTxB = theme.surfaceVariant; 
        tipTxB.a *= alpha;
        
        renderer.drawM3UI(finalTipX, finalTipY, tipW, tipH, dp(8.0f), dp(8.0f), dp(8.0f), dp(8.0f), tipBg);
        renderer.drawText(l1, std::round(finalTipX + dp(12.0f)), std::round(finalTipY + dp(8.0f)), dp(12.0f), tipTxA);
        renderer.drawText(l2, std::round(finalTipX + dp(12.0f)), std::round(finalTipY + dp(26.0f)), dp(11.0f), tipTxB);
        renderer.drawText(l3, std::round(finalTipX + dp(12.0f)), std::round(finalTipY + dp(43.0f)), dp(11.0f), m_avgColor);
    }
}