/**
 * @file MaterialShader.hpp
 * @brief Material Design 3 multi-icon and FreeType text renderer with GPU hardware pipeline.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <fstream>
#include <set>
#include <cstdint>
#include <glad/glad.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../theme/MaterialTheme.hpp"
#include "Icon.hpp"

/**
 * @struct CharInfo
 * @brief Metric positioning and UV texture coordinates for a single rasterized character glyph.
 */
struct CharInfo {
    float ax = 0.0f;  /**< Horizontal advance step in pixels. */
    float bw = 0.0f;  /**< Bitmap bounding width in pixels. */
    float bh = 0.0f;  /**< Bitmap bounding height in pixels. */
    float bl = 0.0f;  /**< Horizontal bearing offset from origin. */
    float bt = 0.0f;  /**< Vertical bearing offset from origin (inverted). */
    float tx0 = 0.0f; /**< Top-left horizontal UV coordinate in the atlas. */
    float ty0 = 0.0f; /**< Top-left vertical UV coordinate in the atlas. */
    float tx1 = 0.0f; /**< Bottom-right horizontal UV coordinate in the atlas. */
    float ty1 = 0.0f; /**< Bottom-right vertical UV coordinate in the atlas. */
};

/**
 * @struct FontAtlas
 * @brief Encapsulates a dynamic font atlas texture and character metrics via FreeType with automatic RAII cleanup.
 */
struct FontAtlas {
    unsigned int texture = 0;              /**< OpenGL 2D texture descriptor for the glyph atlas. */
    FT_Face face = nullptr;                /**< FreeType loaded face handle. */
    std::vector<unsigned char> fontBuffer; /**< Raw binary buffer of the loaded font data. */
    std::map<uint64_t, CharInfo> charMap;  /**< Cache mapping (codepoint | (pixel_size << 32)) to CharInfo. */
    int nextX = 2;                         /**< Current horizontal packing cursor in pixels. */
    int nextY = 2;                         /**< Current vertical packing cursor in pixels. */
    int rowHeight = 0;                     /**< Height of the current packing row in pixels. */

    /**
     * @brief Destructor. Automatically deallocates FreeType faces and OpenGL texture descriptors.
     */
    ~FontAtlas() {
        if (face) {
            FT_Done_Face(face);
            face = nullptr;
        }
        if (texture != 0) {
            glDeleteTextures(1, &texture);
            texture = 0;
        }
    }
};

/**
 * @class MaterialShader
 * @brief GPU-accelerated renderer providing SDF shapes, ripples, FreeType text, and icons.
 */
class MaterialShader {
public:
    unsigned int shaderProgram = 0; /**< Compiled and linked OpenGL shader program handle. */
    unsigned int quadVAO = 0;       /**< Vertex Array Object for the normalized rendering quad. */
    unsigned int quadVBO = 0;       /**< Vertex Buffer Object containing quad vertex positions. */

    std::map<std::string, std::unique_ptr<FontAtlas>> fontAtlases; /**< Loaded font atlases keyed by font name. */
    std::string currentFontKey;     /**< Identifier of the active font used for immediate rendering. */
    std::string globalFontKey;      /**< Identifier of the primary global default font. */

    /** @brief Default constructor. */
    MaterialShader() = default;

    /** @brief Destructor that releases all GPU resources and FreeType contexts. */
    ~MaterialShader();

    /**
     * @brief Releases all allocated GPU buffers, programs, textures, and FreeType resources.
     */
    void cleanup();

    /**
     * @brief Initializes shader programs, uniform locations, and vertex buffers.
     */
    void init();

    /**
     * @brief Binds the shader pipeline and sets up the 2D orthographic projection matrix.
     * 
     * @param width Viewport width in pixels.
     * @param height Viewport height in pixels.
     */
    void begin(int width, int height);

    /**
     * @brief Dispatches the low-level Material 3 procedural shape and texture rendering pass.
     * 
     * @param x Top-left X position.
     * @param y Top-left Y position.
     * @param w Width of the bounding box.
     * @param h Height of the bounding box.
     * @param rTL Top-left corner radius.
     * @param rTR Top-right corner radius.
     * @param rBR Bottom-right corner radius.
     * @param rBL Bottom-left corner radius.
     * @param color Background fill color.
     * @param rx Ripple center X coordinate.
     * @param ry Ripple center Y coordinate.
     * @param rRadius Ripple expansion radius.
     * @param rAlpha Ripple opacity multiplier.
     * @param rColor Ripple tint color.
     * @param iconType Built-in procedural vector icon index.
     * @param icSize Icon bounding dimensions.
     * @param icCol Icon color tint.
     * @param borderColor Perimeter border outline color.
     * @param borderW Border thickness in pixels.
     * @param customTexture OpenGL texture descriptor to bind.
     * @param isTextChar If true, samples texture as a single-channel alpha text glyph.
     * @param texUMin Minimum U texture coordinate.
     * @param texVMin Minimum V texture coordinate.
     * @param texUMax Maximum U texture coordinate.
     * @param texVMax Maximum V texture coordinate.
     * @param blur Edge feathering or shadow blur radius.
     * @param rotation Angular rotation in radians.
     * @param rotCx Center X coordinate of rotation axis.
     * @param rotCy Center Y coordinate of rotation axis.
     */
    void drawM3UI(float x, float y, float w, float h,
        float rTL, float rTR, float rBR, float rBL,
        M3Color color,
        float rx = 0.0f, float ry = 0.0f, float rRadius = 0.0f, float rAlpha = 0.0f, M3Color rColor = { 0,0,0,0 },
        int iconType = 0, float icSize = 0.0f, M3Color icCol = { 0,0,0,0 },
        M3Color borderColor = { 0,0,0,0 }, float borderW = 0.0f,
        unsigned int customTexture = 0, bool isTextChar = false,
        float texUMin = 0.0f, float texVMin = 0.0f, float texUMax = 1.0f, float texVMax = 1.0f,
        float blur = 0.0f,
        float rotation = 0.0f, float rotCx = 0.0f, float rotCy = 0.0f);

    /**
     * @brief Loads a TrueType or OpenType font into an atlas.
     * 
     * @param fontPath File path to the font file.
     * @param fontName Optional alias for font retrieval. If empty, uses the filename stem.
     * @return true if loaded successfully, false otherwise.
     */
    bool loadFont(const std::string& fontPath, const std::string& fontName = "");

    /**
     * @brief Configures the default fallback font name.
     * 
     * @param fontName Registered font name or file path.
     * @return true if successfully applied or loaded, false otherwise.
     */
    bool setDefaultFont(const std::string& fontName);

    /**
     * @brief Retrieves the name of the current default font.
     * 
     * @return Name of the configured default font.
     */
    std::string getDefaultFontName() const { return m_customDefaultFont; }

    /**
     * @brief Checks if a specific font is currently loaded in the atlas map.
     * 
     * @param fontName Target font name.
     * @return true if present, false otherwise.
     */
    bool isFontLoaded(const std::string& fontName) const;

    /**
     * @brief Switches the active font for subsequent drawing operations.
     * 
     * @param fontNameOrPath Name or file path of the font to use.
     */
    void useFont(const std::string& fontNameOrPath);

    /**
     * @brief Configures the primary global font.
     * 
     * @param fontNameOrPath Name or path of the font.
     * @return true if successful, false otherwise.
     */
    bool setGlobalFont(const std::string& fontNameOrPath);

    /**
     * @brief Draws a UTF-8 text string at the specified coordinates.
     * 
     * @param textUtf8 UTF-8 encoded text string.
     * @param x Top-left X coordinate.
     * @param y Baseline reference Y coordinate.
     * @param sizePx Font rendering size in pixels.
     * @param color Text fill color.
     */
    void drawText(const std::string& textUtf8, float x, float y, float sizePx, M3Color color);

    /**
     * @brief Draws text centered or left-aligned within a specified bounding box.
     * 
     * @param textUtf8 UTF-8 encoded text string.
     * @param boxX Container X origin.
     * @param boxY Container Y origin.
     * @param boxW Container width.
     * @param boxH Container height.
     * @param sizePx Font rendering size in pixels.
     * @param color Text fill color.
     * @param alignLeft If true, aligns text to the left side with padding.
     * @param paddingLeft Left padding offset in pixels when alignLeft is true.
     */
    void drawTextCentered(const std::string& textUtf8, float boxX, float boxY, float boxW, float boxH, float sizePx, M3Color color, bool alignLeft = false, float paddingLeft = 0.0f);

    /**
     * @brief Calculates the horizontal pixel width of a UTF-8 text string.
     * 
     * @param textUtf8 UTF-8 encoded string.
     * @param sizePx Target font size in pixels.
     * @return Total horizontal width in pixels.
     */
    float getTextWidth(const std::string& textUtf8, float sizePx);

    /**
     * @brief Unbinds the active VAO and texture units after rendering.
     */
    void end();

    /**
     * @brief Loads a dedicated Material Design icon font file.
     * 
     * @param path File system path to the icon TTF/OTF font file.
     * @return true if loaded successfully, false otherwise.
     */
    bool loadIconFont(const std::string& path);

    /**
     * @brief Checks if a dedicated icon font is loaded.
     * 
     * @return true if loaded, false otherwise.
     */
    bool isIconFontLoaded() const { return m_iconFontLoaded; }

    /**
     * @brief Enables or disables SDF fallback rendering for icons.
     * 
     * @param useSDF If true, uses procedural vector SDF when glyphs are unavailable.
     */
    void setIconFontFallback(bool useSDF) { m_useSDFFallback = useSDF; }

    /**
     * @brief Retrieves the path to the loaded icon font asset.
     * 
     * @return Path string to the icon font file.
     */
    std::string getIconFontPath() const { return m_iconFontPath; }

    /**
     * @brief Renders a polymorphic Icon variant with transform options.
     * 
     * @param x Top-left X position.
     * @param y Top-left Y position.
     * @param size Bounding square dimension of the icon.
     * @param icon Polymorphic Icon container.
     * @param color Icon tint color.
     * @param blur Blur feathering radius.
     * @param rotation Angular rotation in radians.
     * @param rotCx Center X coordinate of rotation.
     * @param rotCy Center Y coordinate of rotation.
     */
    void drawIcon(float x, float y, float size, const Icon& icon, M3Color color, float blur = 0.0f, float rotation = 0.0f, float rotCx = 0.0f, float rotCy = 0.0f);

    /**
     * @brief Convenience overload for rendering an icon by string name.
     * 
     * @param x Top-left X position.
     * @param y Top-left Y position.
     * @param size Bounding square dimension of the icon.
     * @param iconName String name or path of the icon.
     * @param color Icon tint color.
     * @param blur Blur feathering radius.
     * @param rotation Angular rotation in radians.
     * @param rotCx Center X coordinate of rotation.
     * @param rotCy Center Y coordinate of rotation.
     */
    void drawIcon(float x, float y, float size, const std::string& iconName, M3Color color, float blur = 0.0f, float rotation = 0.0f, float rotCx = 0.0f, float rotCy = 0.0f) {
        drawIcon(x, y, size, iconFromString(iconName), color, blur, rotation, rotCx, rotCy);
    }

    /**
     * @brief Translates an icon name string into its UTF-32 codepoint value.
     * 
     * @param name Name identifier of the icon.
     * @return uint32_t Unicode codepoint.
     */
    static inline unsigned int getCodepointForM3Icon(const std::string& name) {
        return toUnicode(parseM3Icon(name));
    }

    /**
     * @brief Resolves an icon name string to an internal procedural SDF vector ID.
     * 
     * @param name Icon string name or alias.
     * @return Procedural vector ID, or 0 if unsupported.
     */
    static inline int resolveM3IconName(const std::string& name) {
        std::string key = name;
        if (key.rfind("icon:", 0) == 0) key = key.substr(5);
        else if (key.rfind("m3:", 0) == 0) key = key.substr(3);

        if (key == "check") return 1;
        if (key == "sun") return 2;
        if (key == "moon") return 3;
        if (key == "diamond") return 4;
        if (key == "close" || key == "cross") return 5;
        if (key == "add") return 6;
        if (key == "search") return 7;
        if (key == "settings") return 8;
        if (key == "menu") return 9;
        if (key == "arrow_back") return 10;
        if (key == "arrow_forward") return 11;
        if (key == "edit") return 12;
        if (key == "home") return 13;
        if (key == "info") return 14;
        if (key == "chevron_up" || key == "expand_less") return 15;
        if (key == "chevron_down" || key == "expand_more") return 16;
        if (key == "warning" || key == "alert" || key == "priority_high" || key == "exclamation" || key == "error") return 17;
        return 0;
    }

private:
    /**
     * @brief Calculates optical midline offset based on Cap-Height and x-Height.
     * 
     * @param atlas Font atlas to sample metrics from.
     * @param sizePx Pixel size of the font.
     * @return Optical midline vertical adjustment.
     */
    float getFontCapHeight(FontAtlas& atlas, float sizePx);

    /**
     * @brief Internal helper to load font binary into a FontAtlas descriptor.
     * 
     * @param fontPath Path to font file.
     * @param atlas Reference to FontAtlas structure.
     * @return true on success, false on failure.
     */
    bool loadFontToAtlas(const std::string& fontPath, FontAtlas& atlas);

    /**
     * @brief Checks and resolves an absolute or relative font file path.
     * 
     * @param fontNameOrPath Name or path of the font file.
     * @return Absolute path if found, or empty string.
     */
    std::string findFontFile(const std::string& fontNameOrPath);

    /**
     * @brief Retrieves cached glyph metric or rasterizes a new character into the atlas.
     * 
     * @param atlas Target font atlas.
     * @param codepoint UTF-32 character codepoint.
     * @param sizePx Rasterization size in pixels.
     * @return CharInfo containing metrics and UV coordinates.
     */
    CharInfo getOrBakeChar(FontAtlas& atlas, unsigned int codepoint, float sizePx);

    /**
     * @brief Dispatches rendering for an M3Icon enum instance.
     */
    void renderM3Icon(float x, float y, float size, M3Icon icon, M3Color color, float blur, float rotation = 0.0f, float rotCx = 0.0f, float rotCy = 0.0f);

    /**
     * @brief Dispatches rendering for an icon defined by a file path.
     */
    void renderFileIcon(float x, float y, float size, const std::string& path, M3Color color, float blur, float rotation = 0.0f, float rotCx = 0.0f, float rotCy = 0.0f);

    /**
     * @brief Dispatches rendering for an icon using a raw GPU texture ID.
     */
    void renderTextureIcon(float x, float y, float size, unsigned int tex, M3Color color, float blur, float rotation = 0.0f, float rotCx = 0.0f, float rotCy = 0.0f);

    /**
     * @brief Dispatches rendering for an icon using procedural SDF shaders.
     */
    void renderVectorIcon(float x, float y, float size, int vectorId, M3Color color, float blur, float rotation = 0.0f, float rotCx = 0.0f, float rotCy = 0.0f);

    FT_Library m_ftLibrary = nullptr;             /**< FreeType library instance handle. */

    std::string m_iconFontPath = "";               /**< Path to the icon font asset. */
    bool m_iconFontLoaded = false;                 /**< Flag indicating if icon font is ready. */
    bool m_useSDFFallback = true;                  /**< Flag indicating if SDF fallback is enabled. */
    std::string m_customDefaultFont = "";          /**< Default font name fallback. */
    std::set<std::string> m_failedFonts;           /**< Set of font paths that previously failed to load. */

    int loc_projection = -1;      /**< Uniform location: projection matrix. */
    int loc_u_position = -1;      /**< Uniform location: UI element position. */
    int loc_u_size = -1;          /**< Uniform location: UI element size. */
    int loc_u_borderRadius = -1;  /**< Uniform location: 4-corner border radius. */
    int loc_u_color = -1;         /**< Uniform location: element fill color. */
    int loc_u_blur = -1;          /**< Uniform location: edge blur radius. */
    int loc_u_ripplePos = -1;     /**< Uniform location: ripple center coordinates. */
    int loc_u_rippleRadius = -1;  /**< Uniform location: ripple current radius. */
    int loc_u_rippleAlpha = -1;   /**< Uniform location: ripple opacity factor. */
    int loc_u_rippleColor = -1;   /**< Uniform location: ripple color. */
    int loc_u_iconType = -1;      /**< Uniform location: procedural icon type index. */
    int loc_u_iconSize = -1;      /**< Uniform location: procedural icon size. */
    int loc_u_iconColor = -1;     /**< Uniform location: procedural icon color. */
    int loc_u_borderColor = -1;   /**< Uniform location: border outline color. */
    int loc_u_borderWidth = -1;   /**< Uniform location: border width thickness. */
    int loc_u_useTexture = -1;    /**< Uniform location: texture usage mode flag. */
    int loc_u_customTexture = -1; /**< Uniform location: custom texture sampler. */
    int loc_u_texRange = -1;      /**< Uniform location: sub-texture UV range bounds. */
    int loc_u_rotation = -1;      /**< Uniform location: rotation angle in radians. */
    int loc_u_rotationCenter = -1;/**< Uniform location: rotation pivot center coordinates. */
};