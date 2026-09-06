/**
 * @file MaterialShader.cpp
 * @brief Material Design 3 multi-icon and FreeType text renderer with dynamic font metric normalization.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include <glad/glad.h>
#include "MaterialShader.hpp"
#include "TextureLoader.hpp"

#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <variant>
#include <iostream>

/**
 * @brief OpenGL ES 3.0 compatible UI vertex shader source.
 */
const char* ui_vertex_shader = R"(#version 300 es
precision highp float;

layout (location = 0) in vec2 aPos;
uniform mat4 projection;
uniform vec2 u_position;
uniform vec2 u_size;
uniform vec4 u_texRange;

uniform float u_rotation;
uniform vec2 u_rotationCenter;

out vec2 fragLocalPos;
out vec2 TexCoords;

void main() {
    vec2 pixelPos = u_position + aPos * u_size;
    if (abs(u_rotation) > 0.0001) {
        vec2 p = pixelPos - u_rotationCenter;
        float s = sin(u_rotation);
        float c = cos(u_rotation);
        pixelPos = vec2(p.x * c - p.y * s, p.x * s + p.y * c) + u_rotationCenter;
    }
    
    gl_Position = projection * vec4(pixelPos, 0.0, 1.0);
    fragLocalPos = (aPos - 0.5) * u_size;
    TexCoords = vec2(
        mix(u_texRange.x, u_texRange.z, aPos.x),
        mix(u_texRange.y, u_texRange.w, aPos.y)
    );
}
)";

/**
 * @brief OpenGL ES 3.0 compatible UI fragment shader source implementing SDF rendering and ripples.
 */
const char* ui_fragment_shader = R"(#version 300 es
precision highp float;

out vec4 FragColor;
in vec2 fragLocalPos;
in vec2 TexCoords;

uniform vec2 u_size;
uniform vec4 u_borderRadius; 
uniform vec4 u_color;
uniform vec4 u_borderColor;
uniform float u_borderWidth;

uniform vec2 u_ripplePos;   
uniform float u_rippleRadius;
uniform float u_rippleAlpha;
uniform vec4 u_rippleColor;

uniform int u_iconType;
uniform float u_iconSize;
uniform vec4 u_iconColor;

uniform sampler2D u_customTexture;
uniform int u_useTexture; 
uniform float u_blur;

float sdRoundedBox(in vec2 p, in vec2 b, in vec4 r) {
    float sx = step(0.0, p.x);
    float sy = step(0.0, p.y);
    float leftR = mix(r.x, r.w, sy);
    float rightR = mix(r.y, r.z, sy);
    float radius = mix(leftR, rightR, sx);
    
    vec2 q = abs(p) - b + vec2(radius);
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - radius;
}

float sdSegment(in vec2 p, in vec2 a, in vec2 b) {
    vec2 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}

float getAAAlpha(float dist) {
    if (u_blur > 0.0) return 1.0 - smoothstep(-u_blur, u_blur, dist);
    float afwidth = fwidth(dist);
    float aa = max(afwidth * 0.75, 0.75);
    return 1.0 - smoothstep(-aa, aa, dist);
}

void main() {
    if (u_useTexture == 2) {
        float charAlpha = texture(u_customTexture, TexCoords).r;
        if (charAlpha <= 0.005) discard;
        FragColor = vec4(u_color.rgb, u_color.a * charAlpha);
        return;
    }

    float dist = -10.0;
    float fillMask = 1.0;
    bool isPlainRect = (u_borderRadius == vec4(0.0) && u_blur == 0.0 && u_borderWidth == 0.0);
    
    if (!isPlainRect) {
        vec2 sdfB = u_size * 0.5;
        vec4 sdfR = u_borderRadius;
        if (u_blur > 0.0) {
            sdfB -= vec2(u_blur);
            sdfR = max(vec4(0.0), sdfR - vec4(u_blur));
        }
        dist = sdRoundedBox(fragLocalPos, sdfB, sdfR);
        fillMask = getAAAlpha(dist);
    }

    if (fillMask <= 0.001) discard;

    vec4 fillCol = u_color; 

    if (u_rippleRadius > 0.0 && u_rippleAlpha > 0.0) {
        float rippleDist = distance(fragLocalPos, u_ripplePos);
        float rippleFactor = 1.0 - smoothstep(u_rippleRadius - 3.0, u_rippleRadius + 3.0, rippleDist);
        float rA = u_rippleColor.a * u_rippleAlpha * rippleFactor;
        float nextA = rA + fillCol.a * (1.0 - rA);
        if (nextA > 0.0) {
            fillCol.rgb = mix(fillCol.rgb, u_rippleColor.rgb, rA / nextA);
            fillCol.a = nextA;
        }
    }

    if (u_useTexture == 1) {
        vec4 texCol = texture(u_customTexture, vec2(TexCoords.x, 1.0 - TexCoords.y));
        float nextA = texCol.a + fillCol.a * (1.0 - texCol.a);
        if (nextA > 0.0) {
            fillCol.rgb = mix(fillCol.rgb, texCol.rgb, texCol.a / nextA);
            fillCol.a = nextA;
        }
    }

    if (u_iconType != 0) {
        vec2 p = vec2(fragLocalPos.x, -fragLocalPos.y); 
        float iconDist = 1e5;
        switch(u_iconType) {
            case 1: { // Check
                vec2 p1 = vec2(-u_iconSize * 0.26, -u_iconSize * 0.02); 
                vec2 p2 = vec2(-u_iconSize * 0.08, -u_iconSize * 0.20); 
                vec2 p3 = vec2(u_iconSize * 0.26, u_iconSize * 0.20); 
                iconDist = min(sdSegment(p, p1, p2), sdSegment(p, p2, p3)) - (u_iconSize * 0.045); 
                break; 
            }
            case 2: { // Sun (Material Design 3 Light Mode: Solid central core with 8 detached floating rays)
                float centerCircle = length(p) - u_iconSize * 0.19; 
                vec2 ap = abs(p);
                // 4 orthogonal cardinal rays
                float rayH = sdSegment(ap, vec2(u_iconSize * 0.28, 0.0), vec2(u_iconSize * 0.40, 0.0));
                float rayV = sdSegment(ap, vec2(0.0, u_iconSize * 0.28), vec2(0.0, u_iconSize * 0.40));
                // 4 diagonal 45-degree rays
                float rayD = sdSegment(ap, vec2(u_iconSize * 0.20, u_iconSize * 0.20), vec2(u_iconSize * 0.285, u_iconSize * 0.285));
                float rays = min(min(rayH, rayV), rayD) - (u_iconSize * 0.036);
                iconDist = min(centerCircle, rays); 
                break; 
            }
            case 3: { // Moon (Material Design 3 Dark Mode: Standard right-facing crescent, plump & optically centered)
                vec2 pMoon = p + vec2(-u_iconSize * 0.03, -u_iconSize * 0.02);
                float dOuter = length(pMoon) - u_iconSize * 0.34;
                float dCutter = length(pMoon - vec2(u_iconSize * 0.16, u_iconSize * 0.12)) - u_iconSize * 0.29;
                iconDist = max(dOuter, -dCutter);
                break; 
            }
            case 4: { // Diamond
                vec2 q = abs(p); 
                iconDist = (q.x + q.y - u_iconSize * 0.34) * 0.7071 - (u_iconSize * 0.02); 
                break; 
            }
            case 5: { // Close / Cross
                iconDist = min(sdSegment(p, vec2(-u_iconSize * 0.24, -u_iconSize * 0.24), vec2(u_iconSize * 0.24, u_iconSize * 0.24)), 
                               sdSegment(p, vec2(-u_iconSize * 0.24, u_iconSize * 0.24), vec2(u_iconSize * 0.24, -u_iconSize * 0.24))) - (u_iconSize * 0.035); 
                break; 
            }
            case 6: { // Add
                iconDist = min(sdSegment(p, vec2(0.0, -u_iconSize * 0.26), vec2(0.0, u_iconSize * 0.26)), 
                               sdSegment(p, vec2(-u_iconSize * 0.26, 0.0), vec2(u_iconSize * 0.26, 0.0))) - (u_iconSize * 0.035); 
                break; 
            }
            case 7: { // Search
                float circle = abs(length(p - vec2(-u_iconSize * 0.06, u_iconSize * 0.06)) - u_iconSize * 0.17) - (u_iconSize * 0.035);
                float handle = sdSegment(p, vec2(u_iconSize * 0.06, -u_iconSize * 0.06), vec2(u_iconSize * 0.24, -u_iconSize * 0.24)) - (u_iconSize * 0.035);
                iconDist = min(circle, handle); 
                break; 
            }
            case 8: { // Settings
                iconDist = max(min(length(p) - u_iconSize * 0.32, min(sdSegment(p, vec2(0.0, -u_iconSize * 0.40), vec2(0.0, u_iconSize * 0.40)), min(sdSegment(p, vec2(-u_iconSize * 0.35, -u_iconSize * 0.20), vec2(u_iconSize * 0.35, u_iconSize * 0.20)), sdSegment(p, vec2(-u_iconSize * 0.35, u_iconSize * 0.20), vec2(u_iconSize * 0.35, -u_iconSize * 0.20)))) - (u_iconSize * 0.045)), -(length(p) - u_iconSize * 0.14)); 
                break; 
            }
            case 9: { // Menu
                float b1 = sdSegment(p, vec2(-u_iconSize*0.26, -u_iconSize*0.18), vec2(u_iconSize*0.26, -u_iconSize*0.18));
                float b2 = sdSegment(p, vec2(-u_iconSize*0.26, 0.0), vec2(u_iconSize*0.26, 0.0));
                float b3 = sdSegment(p, vec2(-u_iconSize*0.26, u_iconSize*0.18), vec2(u_iconSize*0.26, u_iconSize*0.18));
                iconDist = min(min(b1, b2), b3) - (u_iconSize * 0.032); 
                break; 
            }
            case 10: { // Arrow Back
                float bar = sdSegment(p, vec2(-u_iconSize*0.26, 0.0), vec2(u_iconSize*0.26, 0.0));
                float a1 = sdSegment(p, vec2(-u_iconSize*0.26, 0.0), vec2(-u_iconSize*0.06, -u_iconSize*0.20));
                float a2 = sdSegment(p, vec2(-u_iconSize*0.26, 0.0), vec2(-u_iconSize*0.06, u_iconSize*0.20));
                iconDist = min(bar, min(a1, a2)) - (u_iconSize * 0.035); 
                break; 
            }
            case 11: { // Arrow Forward
                float bar = sdSegment(p, vec2(-u_iconSize*0.26, 0.0), vec2(u_iconSize*0.26, 0.0));
                float a1 = sdSegment(p, vec2(u_iconSize*0.26, 0.0), vec2(u_iconSize*0.06, -u_iconSize*0.20));
                float a2 = sdSegment(p, vec2(u_iconSize*0.26, 0.0), vec2(u_iconSize*0.06, u_iconSize*0.20));
                iconDist = min(bar, min(a1, a2)) - (u_iconSize * 0.035); 
                break; 
            }
            case 12: { // Edit
                iconDist = min(min(sdSegment(p, vec2(-u_iconSize*0.20, -u_iconSize*0.20), vec2(u_iconSize*0.14, u_iconSize*0.14)) - u_iconSize * 0.055, sdSegment(p, vec2(u_iconSize*0.14, u_iconSize*0.14), vec2(u_iconSize*0.26, u_iconSize*0.26)) - u_iconSize * 0.055), sdSegment(p, vec2(-u_iconSize*0.20, -u_iconSize*0.20), vec2(-u_iconSize*0.32, -u_iconSize*0.32)) - u_iconSize * 0.025); 
                break; 
            }
            case 13: { // Home
                float roof1 = sdSegment(p, vec2(-u_iconSize*0.30, u_iconSize*0.02), vec2(0.0, u_iconSize*0.28));
                float roof2 = sdSegment(p, vec2(u_iconSize*0.30, u_iconSize*0.02), vec2(0.0, u_iconSize*0.28));
                float wall1 = sdSegment(p, vec2(-u_iconSize*0.22, -u_iconSize*0.28), vec2(-u_iconSize*0.22, u_iconSize*0.02));
                float wall2 = sdSegment(p, vec2(u_iconSize*0.22, -u_iconSize*0.28), vec2(u_iconSize*0.22, u_iconSize*0.02));
                float floorB = sdSegment(p, vec2(-u_iconSize*0.22, -u_iconSize*0.28), vec2(u_iconSize*0.22, -u_iconSize*0.28));
                iconDist = min(min(roof1, roof2), min(min(wall1, wall2), floorB)) - (u_iconSize * 0.035); 
                break; 
            }
            case 14: { // Info
                float ring = abs(length(p) - u_iconSize*0.32) - (u_iconSize*0.032);
                float dot = length(p - vec2(0.0, u_iconSize*0.13)) - (u_iconSize*0.038);
                float bar = sdSegment(p, vec2(0.0, -u_iconSize*0.18), vec2(0.0, u_iconSize*0.02)) - (u_iconSize*0.035);
                iconDist = min(ring, min(dot, bar)); 
                break; 
            }
            case 15: { // Chevron Up
                vec2 p1 = vec2(-u_iconSize * 0.18, -u_iconSize * 0.09); 
                vec2 p2 = vec2(0.0, u_iconSize * 0.09); 
                vec2 p3 = vec2(u_iconSize * 0.18, -u_iconSize * 0.09); 
                iconDist = min(sdSegment(p, p1, p2), sdSegment(p, p2, p3)) - (u_iconSize * 0.035); 
                break; 
            }
            case 16: { // Chevron Down
                vec2 p1 = vec2(-u_iconSize * 0.18, u_iconSize * 0.09); 
                vec2 p2 = vec2(0.0, -u_iconSize * 0.09); 
                vec2 p3 = vec2(u_iconSize * 0.18, u_iconSize * 0.09); 
                iconDist = min(sdSegment(p, p1, p2), sdSegment(p, p2, p3)) - (u_iconSize * 0.035); 
                break; 
            }
            case 17: { // Warning
                float ring = abs(length(p) - u_iconSize * 0.34) - (u_iconSize * 0.04);
                float topBar = sdSegment(p, vec2(0.0, u_iconSize * 0.16), vec2(0.0, -u_iconSize * 0.02)) - (u_iconSize * 0.045);
                float dot = length(p - vec2(0.0, -u_iconSize * 0.17)) - (u_iconSize * 0.048);
                iconDist = min(ring, min(topBar, dot)); 
                break; 
            }
        }

        float iconAlpha = getAAAlpha(iconDist);
        float srcA = iconAlpha * u_iconColor.a;
        float nextA = srcA + fillCol.a * (1.0 - srcA);
        if (nextA > 0.0) {
            fillCol.rgb = mix(fillCol.rgb, u_iconColor.rgb, srcA / nextA);
            fillCol.a = nextA;
        }
    }

    if (u_borderWidth > 0.0 && !isPlainRect) {
        float borderDist = abs(dist + u_borderWidth * 0.5) - u_borderWidth * 0.5;
        float borderAlpha = getAAAlpha(borderDist);
        float innerFillAlpha = getAAAlpha(dist + u_borderWidth);
        
        vec4 fillColActive = vec4(fillCol.rgb, fillCol.a * innerFillAlpha);
        vec4 borderColActive = vec4(u_borderColor.rgb, u_borderColor.a * borderAlpha);
        float outAlpha = borderColActive.a + fillColActive.a * (1.0 - borderColActive.a);
        
        if (outAlpha > 0.0) {
            FragColor = vec4(mix(fillColActive.rgb, borderColActive.rgb, borderColActive.a / outAlpha), outAlpha);
        } else {
            discard; 
        }
        return;
    } 
    FragColor = vec4(fillCol.rgb, fillCol.a * fillMask);
}
)";

/**
 * @brief Decodes the next UTF-8 character codepoint from a byte stream.
 * 
 * @param str The UTF-8 source string.
 * @param i Current byte index in the string; updated past the decoded sequence.
 * @return unsigned int Decoded UTF-32 codepoint value, or 0 if malformed/at end.
 */
static unsigned int getNextUtf8Char(const std::string& str, size_t& i) {
    if (i >= str.size()) return 0;
    unsigned char c = str[i];
    if (c < 0x80) { return str[i++]; }
    else if ((c & 0xE0) == 0xC0) {
        if (i + 1 >= str.size()) { i++; return 0; }
        unsigned int val = ((str[i] & 0x1F) << 6) | (str[i + 1] & 0x3F);
        i += 2; return val;
    }
    else if ((c & 0xF0) == 0xE0) {
        if (i + 2 >= str.size()) { i += 2; return 0; }
        unsigned int val = ((str[i] & 0x0F) << 12) | ((str[i + 1] & 0x3F) << 6) | (str[i + 2] & 0x3F);
        i += 3; return val;
    }
    else if ((c & 0xF8) == 0xF0) {
        if (i + 3 >= str.size()) { i += 3; return 0; }
        unsigned int val = ((str[i] & 0x07) << 18) | ((str[i + 1] & 0x3F) << 12) | ((str[i + 2] & 0x3F) << 6) | (str[i + 3] & 0x3F);
        i += 4; return val;
    }
    i++; return 0;
}

/**
 * @brief Builds a 2D orthographic projection matrix for UI pixel rendering.
 * 
 * @param width Viewport width in pixels.
 * @param height Viewport height in pixels.
 * @param m Pointer to an array of 16 floats storing the 4x4 matrix.
 */
static void makeOrthoMatrix(float width, float height, float* m) {
    m[0] = 2.0f / width;  m[1] = 0.0f;           m[2] = 0.0f;   m[3] = 0.0f;
    m[4] = 0.0f;          m[5] = -2.0f / height; m[6] = 0.0f;   m[7] = 0.0f;
    m[8] = 0.0f;          m[9] = 0.0f;           m[10] = -1.0f; m[11] = 0.0f;
    m[12] = -1.0f;        m[13] = 1.0f;          m[14] = 0.0f;  m[15] = 1.0f;
}

/**
 * @brief Destructor for MaterialShader.
 */
MaterialShader::~MaterialShader() {
    cleanup();
}

/**
 * @brief Cleans up and releases all GPU resources, shaders, and FreeType library handles.
 */
void MaterialShader::cleanup() {
    fontAtlases.clear();
    if (m_ftLibrary != nullptr) {
        FT_Done_FreeType(m_ftLibrary);
        m_ftLibrary = nullptr;
    }
    if (quadVAO != 0) {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    if (quadVBO != 0) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }
    if (shaderProgram != 0) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
}

/**
 * @brief Compiles shaders, binds attribute pointers, and sets up uniform locations.
 */
void MaterialShader::init() {
    if (m_ftLibrary == nullptr) {
        if (FT_Init_FreeType(&m_ftLibrary)) {
            std::cerr << "[MaterialShader] Failed to initialize FreeType library!" << std::endl;
        }
    }

    int success;
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &ui_vertex_shader, NULL);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &ui_fragment_shader, NULL);
    glCompileShader(fs);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    glDeleteShader(vs);
    glDeleteShader(fs);

    loc_projection = glGetUniformLocation(shaderProgram, "projection");
    loc_u_position = glGetUniformLocation(shaderProgram, "u_position");
    loc_u_size = glGetUniformLocation(shaderProgram, "u_size");
    loc_u_borderRadius = glGetUniformLocation(shaderProgram, "u_borderRadius");
    loc_u_color = glGetUniformLocation(shaderProgram, "u_color");
    loc_u_blur = glGetUniformLocation(shaderProgram, "u_blur");
    loc_u_ripplePos = glGetUniformLocation(shaderProgram, "u_ripplePos");
    loc_u_rippleRadius = glGetUniformLocation(shaderProgram, "u_rippleRadius");
    loc_u_rippleAlpha = glGetUniformLocation(shaderProgram, "u_rippleAlpha");
    loc_u_rippleColor = glGetUniformLocation(shaderProgram, "u_rippleColor");
    loc_u_iconType = glGetUniformLocation(shaderProgram, "u_iconType");
    loc_u_iconSize = glGetUniformLocation(shaderProgram, "u_iconSize");
    loc_u_iconColor = glGetUniformLocation(shaderProgram, "u_iconColor");
    loc_u_borderColor = glGetUniformLocation(shaderProgram, "u_borderColor");
    loc_u_borderWidth = glGetUniformLocation(shaderProgram, "u_borderWidth");
    loc_u_useTexture = glGetUniformLocation(shaderProgram, "u_useTexture");
    loc_u_customTexture = glGetUniformLocation(shaderProgram, "u_customTexture");
    loc_u_texRange = glGetUniformLocation(shaderProgram, "u_texRange");
    loc_u_rotation = glGetUniformLocation(shaderProgram, "u_rotation");
    loc_u_rotationCenter = glGetUniformLocation(shaderProgram, "u_rotationCenter");

    float vertices[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
    glGenVertexArrays(1, &quadVAO); glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO); glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/**
 * @brief Resolves a font file path and validates its accessibility.
 * 
 * @param fontNameOrPath File path or name to verify.
 * @return std::string Path if found, otherwise an empty string.
 */
std::string MaterialShader::findFontFile(const std::string& fontNameOrPath) {
    bool isAbsolute = false;
#ifdef _WIN32
    if (fontNameOrPath.find(":") != std::string::npos || fontNameOrPath.find("\\") != std::string::npos) isAbsolute = true;
#else
    if (fontNameOrPath.find("/") == 0) isAbsolute = true;
#endif
    if (isAbsolute) { std::ifstream test(fontNameOrPath); if (test.good()) return fontNameOrPath; return ""; }
    return "";
}

/**
 * @brief Reads a font asset from disk and initializes a GPU texture atlas.
 * 
 * @param fontPath Target font file path.
 * @param atlas Destination FontAtlas descriptor.
 * @return true if initialized successfully, false otherwise.
 */
bool MaterialShader::loadFontToAtlas(const std::string& fontPath, FontAtlas& atlas) {
    if (m_ftLibrary == nullptr) {
        if (FT_Init_FreeType(&m_ftLibrary)) return false;
    }

    FILE* f = fopen(fontPath.c_str(), "rb"); 
    if (!f) return false;
    fseek(f, 0, SEEK_END); 
    size_t size = ftell(f); 
    fseek(f, 0, SEEK_SET);
    atlas.fontBuffer.resize(size); 
    fread(atlas.fontBuffer.data(), 1, size, f); 
    fclose(f);

    if (FT_New_Memory_Face(m_ftLibrary, atlas.fontBuffer.data(), static_cast<FT_Long>(atlas.fontBuffer.size()), 0, &atlas.face)) {
        return false;
    }

    atlas.charMap.clear();

    int texSize = 4096;
    glGenTextures(1, &atlas.texture); 
    glBindTexture(GL_TEXTURE_2D, atlas.texture); 
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    std::vector<unsigned char> zero(texSize * texSize, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texSize, texSize, 0, GL_RED, GL_UNSIGNED_BYTE, zero.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    atlas.nextX = 2; 
    atlas.nextY = 2; 
    atlas.rowHeight = 0;
    return true;
}

/**
 * @brief Retrieves cached glyph metrics or rasterizes new glyphs dynamically into the atlas.
 * 
 * @param atlas Target font atlas descriptor.
 * @param cp Character Unicode codepoint.
 * @param sizePx Target rasterization size in pixels.
 * @return CharInfo Containing geometric metrics and UV texture bounds.
 */
CharInfo MaterialShader::getOrBakeChar(FontAtlas& atlas, unsigned int cp, float sizePx) {
    int targetSize = static_cast<int>(std::round(sizePx));
    if (targetSize < 8) targetSize = 8;
    if (targetSize > 256) targetSize = 256;

    uint64_t key = ((uint64_t)cp) | ((uint64_t)targetSize << 32);
    auto it = atlas.charMap.find(key);
    if (it != atlas.charMap.end()) return it->second;

    CharInfo info;
    if (!atlas.face) return info;

    int unitsPerEM = atlas.face->units_per_EM ? atlas.face->units_per_EM : 2048;
    int fontHeight = (atlas.face->ascender > atlas.face->descender) ? (atlas.face->ascender - atlas.face->descender) : unitsPerEM;
    int emPixelSize = static_cast<int>(std::round((float)targetSize * (float)unitsPerEM / (float)fontHeight));
    if (emPixelSize < 1) emPixelSize = 1;

    FT_Set_Pixel_Sizes(atlas.face, 0, emPixelSize);
    FT_UInt glyph_index = FT_Get_Char_Index(atlas.face, cp);

    if (glyph_index == 0 && cp != ' ') {
        info.tx0 = info.ty0 = info.tx1 = info.ty1 = 0.0f;
        atlas.charMap[key] = info;
        return info;
    }

    if (FT_Load_Glyph(atlas.face, glyph_index, FT_LOAD_RENDER)) {
        info.tx0 = info.ty0 = info.tx1 = info.ty1 = 0.0f;
        atlas.charMap[key] = info;
        return info;
    }

    FT_GlyphSlot slot = atlas.face->glyph;
    int w = slot->bitmap.width;
    int h = slot->bitmap.rows;

    info.ax = (float)slot->advance.x / 64.0f;
    info.bw = (float)w;
    info.bh = (float)h;
    info.bl = (float)slot->bitmap_left;
    info.bt = -(float)slot->bitmap_top; 

    if (w == 0 || h == 0) {
        info.tx0 = info.ty0 = info.tx1 = info.ty1 = 0.0f;
        atlas.charMap[key] = info;
        return info;
    }

    int texSize = 4096; int pad = 2;
    if (atlas.nextX + w + pad > texSize) { atlas.nextX = 2; atlas.nextY += atlas.rowHeight + pad; atlas.rowHeight = 0; }
    if (atlas.nextY + h + pad > texSize) {
        atlas.nextX = 2; atlas.nextY = 2; atlas.rowHeight = 0; atlas.charMap.clear();
        glBindTexture(GL_TEXTURE_2D, atlas.texture);
        std::vector<unsigned char> zero(texSize * texSize, 0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texSize, texSize, GL_RED, GL_UNSIGNED_BYTE, zero.data());
    }

    int posX = atlas.nextX; int posY = atlas.nextY;
    glBindTexture(GL_TEXTURE_2D, atlas.texture); 
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, posX, posY, w, h, GL_RED, GL_UNSIGNED_BYTE, slot->bitmap.buffer);

    info.tx0 = (float)posX / (float)texSize; info.ty0 = (float)posY / (float)texSize;
    info.tx1 = (float)(posX + w) / (float)texSize; info.ty1 = (float)(posY + h) / (float)texSize;

    atlas.nextX += w + pad; if (h > atlas.rowHeight) atlas.rowHeight = h;
    atlas.charMap[key] = info;
    return info;
}

/**
 * @brief Calculates optical midline offset using the weighted average of Cap-Height and x-Height.
 * 
 * Ensures mixed-case typography aligns symmetrically with adjacent icons.
 * 
 * @param atlas Target font atlas.
 * @param sizePx Pixel size of the font.
 * @return float Optical midline adjustment offset in pixels.
 */
float MaterialShader::getFontCapHeight(FontAtlas& atlas, float sizePx) {
    float capH = 0.0f;
    float xH = 0.0f;

    const uint32_t capProbes[] = { 'H', 'E', 'D', 'M', 'A', 'B', 'K', 'T', 'X', '0', '8', 0x4E2D };
    for (uint32_t cp : capProbes) {
        CharInfo info = getOrBakeChar(atlas, cp, sizePx);
        if (-info.bt > 0.0f) {
            capH = -info.bt;
            break;
        }
    }
    if (capH <= 0.0f) capH = sizePx * 0.70f;

    const uint32_t xProbes[] = { 'x', 'o', 'a', 'e', 'c', 'v', 'u' };
    for (uint32_t cp : xProbes) {
        CharInfo info = getOrBakeChar(atlas, cp, sizePx);
        if (-info.bt > 0.0f) {
            xH = -info.bt;
            break;
        }
    }
    if (xH <= 0.0f) xH = capH * 0.66f;

    return (capH + xH) * 0.25f;
}

/**
 * @brief Loads a custom font from a file path into the font atlas registry.
 * 
 * @param fontPath File path to the font asset.
 * @param fontName Optional alias for the font.
 * @return true if successfully loaded, false otherwise.
 */
bool MaterialShader::loadFont(const std::string& fontPath, const std::string& fontName) {
    if (m_failedFonts.find(fontPath) != m_failedFonts.end()) return false;
    std::string safePath = findFontFile(fontPath); if (safePath.empty()) safePath = fontPath; 
    std::ifstream test(safePath); if (!test.good()) { m_failedFonts.insert(fontPath); return false; } test.close();

    auto atlas = std::unique_ptr<FontAtlas>(new FontAtlas());
    if (loadFontToAtlas(safePath, *atlas)) {
        std::string name = fontName;
        if (name.empty()) {
            size_t lastSlash = fontPath.find_last_of("/\\");
            if (lastSlash != std::string::npos) name = fontPath.substr(lastSlash + 1); else name = fontPath;
            size_t dot = name.find_last_of('.'); if (dot != std::string::npos) name = name.substr(0, dot);
        }
        fontAtlases[name] = std::move(atlas);
        if (m_customDefaultFont.empty()) { m_customDefaultFont = name; globalFontKey = name; currentFontKey = name; }
        m_failedFonts.erase(fontPath); return true;
    }
    m_failedFonts.insert(fontPath); return false;
}

/**
 * @brief Configures and validates the default fallback font name.
 * 
 * @param fontName Target font name identifier.
 * @return true if available or successfully loaded, false otherwise.
 */
bool MaterialShader::setDefaultFont(const std::string& fontName) {
    if (m_failedFonts.find(fontName) != m_failedFonts.end()) return false;
    if (fontAtlases.find(fontName) != fontAtlases.end() || loadFont(fontName)) {
        m_customDefaultFont = fontName; globalFontKey = fontName; currentFontKey = fontName; return true;
    }
    m_failedFonts.insert(fontName); return false;
}

/**
 * @brief Checks if a font is loaded in the registry.
 * 
 * @param fontName Target font name.
 * @return true if loaded, false otherwise.
 */
bool MaterialShader::isFontLoaded(const std::string& fontName) const {
    return fontAtlases.find(fontName) != fontAtlases.end();
}

/**
 * @brief Sets the global active font identifier.
 * 
 * @param fontNameOrPath Font name or file path.
 * @return true if available or loaded, false otherwise.
 */
bool MaterialShader::setGlobalFont(const std::string& fontNameOrPath) {
    if (fontAtlases.find(fontNameOrPath) != fontAtlases.end()) { globalFontKey = fontNameOrPath; currentFontKey = fontNameOrPath; return true; }
    return loadFont(fontNameOrPath);
}

/**
 * @brief Sets the current font for subsequent text rendering commands.
 * 
 * @param fontNameOrPath Font name or file path.
 */
void MaterialShader::useFont(const std::string& fontNameOrPath) {
    if (fontAtlases.find(fontNameOrPath) != fontAtlases.end()) { currentFontKey = fontNameOrPath; return; }
    if (m_failedFonts.find(fontNameOrPath) == m_failedFonts.end() && loadFont(fontNameOrPath)) currentFontKey = fontNameOrPath;
}

/**
 * @brief Binds the shader program and configures the 2D orthographic projection matrix.
 * 
 * @param width Viewport rendering width.
 * @param height Viewport rendering height.
 */
void MaterialShader::begin(int width, int height) {
    glUseProgram(shaderProgram); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float ortho[16]; makeOrthoMatrix((float)width, (float)height, ortho);

    glUniformMatrix4fv(loc_projection, 1, GL_FALSE, ortho);
    glBindVertexArray(quadVAO);
}

/**
 * @brief Executes a single draw call with the Material UI shader pipeline.
 */
void MaterialShader::drawM3UI(float x, float y, float w, float h, float rTL, float rTR, float rBR, float rBL, M3Color color, float rx, float ry, float rRadius, float rAlpha, M3Color rColor, int iconType, float icSize, M3Color icCol, M3Color borderColor, float borderW, unsigned int customTexture, bool isTextChar, float texUMin, float texVMin, float texUMax, float texVMax, float blur, float rotation, float rotCx, float rotCy) {
    glUniform2f(loc_u_position, x, y);
    glUniform2f(loc_u_size, w, h);
    glUniform4f(loc_u_borderRadius, rTL, rTR, rBR, rBL);
    glUniform4f(loc_u_color, color.r, color.g, color.b, color.a);
    glUniform1f(loc_u_blur, blur);
    glUniform2f(loc_u_ripplePos, rx, ry);
    glUniform1f(loc_u_rippleRadius, rRadius);
    glUniform1f(loc_u_rippleAlpha, rAlpha);
    glUniform4f(loc_u_rippleColor, rColor.r, rColor.g, rColor.b, rColor.a);
    glUniform1i(loc_u_iconType, iconType);
    glUniform1f(loc_u_iconSize, icSize);
    glUniform4f(loc_u_iconColor, icCol.r, icCol.g, icCol.b, icCol.a);
    glUniform4f(loc_u_borderColor, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    glUniform1f(loc_u_borderWidth, borderW);

    glUniform1f(loc_u_rotation, rotation);
    glUniform2f(loc_u_rotationCenter, rotCx, rotCy);

    if (customTexture != 0) {
        glUniform1i(loc_u_useTexture, isTextChar ? 2 : 1);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, customTexture);
        glUniform1i(loc_u_customTexture, 0);
        glUniform4f(loc_u_texRange, texUMin, texVMin, texUMax, texVMax);
    }
    else {
        glUniform1i(loc_u_useTexture, 0);
        glUniform4f(loc_u_texRange, 0.0f, 0.0f, 1.0f, 1.0f);
    }
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

/**
 * @brief Loads a specialized Material Design icon font file.
 * 
 * @param path File system path to the icon font asset.
 * @return true if loaded successfully, false otherwise.
 */
bool MaterialShader::loadIconFont(const std::string& path) {
    if (path.empty()) return false;
    std::string safePath = findFontFile(path);
    if (!safePath.empty()) {
        auto atlas = std::unique_ptr<FontAtlas>(new FontAtlas());
        if (loadFontToAtlas(safePath, *atlas)) {
            fontAtlases["MaterialIcons"] = std::move(atlas); m_iconFontPath = safePath; m_iconFontLoaded = true; return true; }
    }
    return false;
}

/**
 * @brief Renders a polymorphic Icon variant with optional transform adjustments.
 * 
 * @param x Top-left X position.
 * @param y Top-left Y position.
 * @param size Icon bounding dimension.
 * @param icon Icon variant descriptor.
 * @param color Tint color.
 * @param blur Blur radius.
 * @param rotation Rotation in radians.
 * @param rotCx Center X coordinate of rotation axis.
 * @param rotCy Center Y coordinate of rotation axis.
 */
void MaterialShader::drawIcon(float x, float y, float size, const Icon& icon, M3Color color, float blur, float rotation, float rotCx, float rotCy) {
    float fx = std::round(x);
    float fy = std::round(y);
    float fsize = std::round(size);

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, IconNone>) {}
        else if constexpr (std::is_same_v<T, IconM3>) {
            if (m_iconFontLoaded) {
                auto it = fontAtlases.find("MaterialIcons");
                if (it != fontAtlases.end()) {
                    FontAtlas& atlas = *(it->second);
                    unsigned int cp = static_cast<uint32_t>(arg.icon);
                    CharInfo info = getOrBakeChar(atlas, cp, fsize);

                    float cx = std::round(fx + (fsize - info.bw) * 0.5f);
                    float cy = std::round(fy + (fsize - info.bh) * 0.5f);

                    drawM3UI(cx, cy, info.bw, info.bh, 0, 0, 0, 0, color, 0, 0, 0, 0, { 0,0,0,0 }, 0, 0, { 0,0,0,0 }, { 0,0,0,0 }, 0.0f, atlas.texture, true, info.tx0, info.ty0, info.tx1, info.ty1, blur, rotation, rotCx, rotCy);
                    return;
                }
            }
            int vectorId = 0;
            switch (arg.icon) {
            case M3Icon::Check: case M3Icon::Favorite: case M3Icon::Message: case M3Icon::Bell: case M3Icon::Mail: vectorId = 1; break;
            case M3Icon::Sun: case M3Icon::Share: case M3Icon::Location: vectorId = 2; break;
            case M3Icon::Moon: vectorId = 3; break;
            case M3Icon::Diamond: case M3Icon::Star: case M3Icon::Grid: case M3Icon::Dashboard: case M3Icon::Code: case M3Icon::Terminal: vectorId = 4; break;
            case M3Icon::Close: case M3Icon::Delete: vectorId = 5; break;
            case M3Icon::Add: case M3Icon::Refresh: case M3Icon::Cloud: case M3Icon::History: vectorId = 6; break;
            case M3Icon::Search: case M3Icon::Visibility: case M3Icon::VisibilityOff: vectorId = 7; break;
            case M3Icon::Settings: case M3Icon::Lock: case M3Icon::Unlock: vectorId = 8; break;
            case M3Icon::Menu: case M3Icon::MoreVert: case M3Icon::MoreHoriz: case M3Icon::Pause: vectorId = 9; break;
            case M3Icon::ArrowBack: case M3Icon::Download: vectorId = 10; break;
            case M3Icon::ArrowForward: case M3Icon::Upload: case M3Icon::Play: vectorId = 11; break;
            case M3Icon::Edit: vectorId = 12; break;
            case M3Icon::Home: case M3Icon::Person: case M3Icon::Folder: case M3Icon::File: vectorId = 13; break;
            case M3Icon::Info: vectorId = 14; break;
            case M3Icon::ChevronUp: vectorId = 15; break;
            case M3Icon::ChevronDown: vectorId = 16; break;
            case M3Icon::Warning: case M3Icon::PriorityHigh: case M3Icon::Error: case M3Icon::ErrorOutline: vectorId = 17; break;
            default: vectorId = 0; break;
            }
            if (vectorId != 0) renderVectorIcon(fx, fy, fsize, vectorId, color, blur, rotation, rotCx, rotCy);
        }
        else if constexpr (std::is_same_v<T, IconFile>) renderFileIcon(fx, fy, fsize, arg.path, color, blur, rotation, rotCx, rotCy);
        else if constexpr (std::is_same_v<T, IconTexture>) renderTextureIcon(fx, fy, fsize, arg.textureId, color, blur, rotation, rotCx, rotCy);
        else if constexpr (std::is_same_v<T, IconVector>) renderVectorIcon(fx, fy, fsize, arg.vectorId, color, blur, rotation, rotCx, rotCy);
    }, icon);
}

/**
 * @brief Renders an M3Icon by dispatching through the polymorphic pipeline.
 */
void MaterialShader::renderM3Icon(float x, float y, float size, M3Icon icon, M3Color color, float blur, float rotation, float rotCx, float rotCy) { 
    drawIcon(x, y, size, IconM3{ icon }, color, blur, rotation, rotCx, rotCy); 
}

/**
 * @brief Renders an icon from a file asset or string identifier.
 */
void MaterialShader::renderFileIcon(float x, float y, float size, const std::string& path, M3Color color, float blur, float rotation, float rotCx, float rotCy) {
    if (path.rfind("icon:", 0) == 0 || path.rfind("m3:", 0) == 0) { 
        renderM3Icon(x, y, size, parseM3Icon(path), color, blur, rotation, rotCx, rotCy); 
        return; 
    }
    unsigned int tex = loadCustomImageTexture(path);
    if (tex != 0) renderTextureIcon(x, y, size, tex, color, blur, rotation, rotCx, rotCy);
}

/**
 * @brief Renders an icon using a raw OpenGL texture descriptor.
 */
void MaterialShader::renderTextureIcon(float x, float y, float size, unsigned int tex, M3Color color, float blur, float rotation, float rotCx, float rotCy) {
    float fx = std::round(x);
    float fy = std::round(y);
    float fsize = std::round(size);
    drawM3UI(fx, fy, fsize, fsize, 0, 0, 0, 0, { 0.0f, 0.0f, 0.0f, 0.0f }, 0, 0, 0, 0, { 0,0,0,0 }, 0, 0, { 0,0,0,0 }, { 0,0,0,0 }, 0.0f, tex, false, 0.0f, 0.0f, 1.0f, 1.0f, blur, rotation, rotCx, rotCy);
}

/**
 * @brief Renders a procedural SDF vector icon.
 */
void MaterialShader::renderVectorIcon(float x, float y, float size, int vectorId, M3Color color, float blur, float rotation, float rotCx, float rotCy) {
    float fx = std::round(x);
    float fy = std::round(y);
    float fsize = std::round(size);
    drawM3UI(fx, fy, fsize, fsize, 0, 0, 0, 0, { 0.0f, 0.0f, 0.0f, 0.0f }, 0, 0, 0, 0, { 0,0,0,0 }, vectorId, fsize, color, { 0,0,0,0 }, 0.0f, 0, false, 0.0f, 0.0f, 1.0f, 1.0f, blur, rotation, rotCx, rotCy);
}

/**
 * @brief Renders a UTF-8 text string with optical baseline alignment.
 */
void MaterialShader::drawText(const std::string& textUtf8, float x, float y, float sizePx, M3Color color) {
    auto it = fontAtlases.find(currentFontKey);
    if (it == fontAtlases.end()) {
        if (!m_customDefaultFont.empty() && m_failedFonts.find(m_customDefaultFont) == m_failedFonts.end()) {
            it = fontAtlases.find(m_customDefaultFont);
            if (it == fontAtlases.end() && !loadFont(m_customDefaultFont)) return;
            it = fontAtlases.find(m_customDefaultFont);
        }
        else return;
    }

    FontAtlas& atlas = *(it->second);
    if (!atlas.face) return;

    float opticalOffset = getFontCapHeight(atlas, sizePx);
    float baselineY = y + sizePx * 0.5f + opticalOffset;
    float lineHeight = sizePx * 1.35f;

    float currentX = x;
    float currentY = y;

    size_t i = 0;
    while (i < textUtf8.size()) {
        unsigned int cp = getNextUtf8Char(textUtf8, i);
        if (cp == 0) continue;

        if (cp == '\r') continue;
        if (cp == '\n') {
            currentX = x;
            currentY += lineHeight;
            baselineY = currentY + sizePx * 0.5f + opticalOffset;
            continue;
        }
        if (cp == '\t') {
            CharInfo spaceInfo = getOrBakeChar(atlas, ' ', sizePx);
            currentX += spaceInfo.ax * 4.0f;
            continue;
        }

        CharInfo info = getOrBakeChar(atlas, cp, sizePx);

        float cx = std::round(currentX + info.bl);
        float cy = std::round(baselineY + info.bt);

        drawM3UI(
            cx, cy, info.bw, info.bh,
            0, 0, 0, 0, color, 0, 0, 0, 0, { 0,0,0,0 }, 0, 0, { 0,0,0,0 }, { 0,0,0,0 }, 0.0f,
            atlas.texture, true, info.tx0, info.ty0, info.tx1, info.ty1, 0.0f
        );

        currentX += info.ax;
    }
}

/**
 * @brief Renders text aligned or centered within a bounding box.
 */
void MaterialShader::drawTextCentered(const std::string& textUtf8, float boxX, float boxY, float boxW, float boxH, float sizePx, M3Color color, bool alignLeft, float paddingLeft) {
    auto it = fontAtlases.find(currentFontKey);
    if (it == fontAtlases.end()) {
        if (!m_customDefaultFont.empty()) {
            it = fontAtlases.find(m_customDefaultFont);
            if (it == fontAtlases.end() && !loadFont(m_customDefaultFont)) return;
            it = fontAtlases.find(m_customDefaultFont);
        }
        else return;
    }

    FontAtlas& atlas = *(it->second);
    if (!atlas.face) return;

    float opticalOffset = getFontCapHeight(atlas, sizePx);
    float baselineY = boxY + boxH * 0.5f + opticalOffset;

    float textW = getTextWidth(textUtf8, sizePx);
    float startX = alignLeft ? (boxX + paddingLeft) : (boxX + (boxW - textW) * 0.5f);

    float currentX = startX;
    size_t i = 0;
    while (i < textUtf8.size()) {
        unsigned int cp = getNextUtf8Char(textUtf8, i);
        if (cp == 0) continue;

        if (cp == '\r' || cp == '\n') continue;
        if (cp == '\t') {
            CharInfo spaceInfo = getOrBakeChar(atlas, ' ', sizePx);
            currentX += spaceInfo.ax * 4.0f;
            continue;
        }

        CharInfo info = getOrBakeChar(atlas, cp, sizePx);

        float cx = std::round(currentX + info.bl);
        float cy = std::round(baselineY + info.bt);

        drawM3UI(
            cx, cy, info.bw, info.bh,
            0, 0, 0, 0, color, 0, 0, 0, 0, { 0,0,0,0 }, 0, 0, { 0,0,0,0 }, { 0,0,0,0 }, 0.0f,
            atlas.texture, true, info.tx0, info.ty0, info.tx1, info.ty1, 0.0f
        );

        currentX += info.ax;
    }
}

/**
 * @brief Measures the total pixel width of a UTF-8 encoded text string.
 * 
 * @param textUtf8 UTF-8 encoded text string.
 * @param sizePx Rendering font size in pixels.
 * @return float Total computed horizontal width.
 */
float MaterialShader::getTextWidth(const std::string& textUtf8, float sizePx) {
    auto it = fontAtlases.find(currentFontKey);
    if (it == fontAtlases.end()) {
        if (!m_customDefaultFont.empty()) {
            it = fontAtlases.find(m_customDefaultFont);
            if (it == fontAtlases.end()) return 0.0f;
        }
        else return 0.0f;
    }
    FontAtlas& atlas = *(it->second);

    float maxWidth = 0.0f;
    float currentWidth = 0.0f;

    size_t i = 0;
    while (i < textUtf8.size()) {
        unsigned int cp = getNextUtf8Char(textUtf8, i);
        if (cp == 0) continue;

        if (cp == '\r') continue;
        if (cp == '\n') {
            if (currentWidth > maxWidth) maxWidth = currentWidth;
            currentWidth = 0.0f;
            continue;
        }
        if (cp == '\t') {
            CharInfo spaceInfo = getOrBakeChar(atlas, ' ', sizePx);
            currentWidth += spaceInfo.ax * 4.0f;
            continue;
        }

        CharInfo info = getOrBakeChar(atlas, cp, sizePx);
        currentWidth += info.ax;
    }

    if (currentWidth > maxWidth) maxWidth = currentWidth;
    return maxWidth;
}

/**
 * @brief Restores OpenGL state and unbinds bound textures and vertex arrays.
 */
void MaterialShader::end() {
    glBindTexture(GL_TEXTURE_2D, 0); 
    glBindVertexArray(0);
}