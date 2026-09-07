/**
 * @file TextView.hpp
 * @brief Material Design 3 basic and multi-line typographic display component with line spacing, inline icons, alignment, and color token support.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include "../layout/View.hpp"
#include "../shader/Icon.hpp"
#include "../theme/MaterialTheme.hpp"
#include <string>
#include <vector>
#include <utility>

/**
 * @enum TextAlignment
 * @brief Horizontal alignment options for typographic content inside bounding boundaries.
 */
enum class TextAlignment {
    Left,   /**< Left-align text lines. */
    Center, /**< Center text lines along horizontal axis. */
    Right   /**< Right-align text lines. */
};

/**
 * @class TextView
 * @brief Typography display element supporting multi-line strings, custom tinting, theme color tokens, optical alignment, and optional inline icons.
 * 
 * Features include automatic line breaking on newline characters, configurable line spacing,
 * horizontal alignment, theme-aware or explicit colors, and an optional leading icon.
 * The component caches layout metrics for efficient rendering, especially for long texts.
 */
class TextView : public View {
private:
    std::string m_text;                /**< Raw UTF-8 text content. */
    float m_textSize = 14.0f;          /**< Font size in dp. */

    float m_lineSpacingMult = 1.2f;    /**< Line height multiplier relative to font size. */
    float m_lineSpacingExtra = 0.0f;   /**< Additional spacing between lines in dp. */
    TextAlignment m_alignment = TextAlignment::Left; /**< Horizontal alignment mode. */

    M3Color m_color = { 0.0f, 0.0f, 0.0f, 1.0f }; /**< Explicit color (if not using theme). */
    bool m_useThemeColor = true;       /**< Flag indicating whether to use theme token color. */
    std::string m_themeColorToken = "onSurface"; /**< Theme color token (e.g., "primary", "onSurface"). */
    
    Icon m_icon;                       /**< Inline leading icon descriptor. */
    bool m_hasIcon = false;            /**< Whether an icon is set. */
    float m_iconSizeDp = 0.0f;         /**< Custom icon size in dp (0 means auto-calculated). */

    // --- High-performance cache for long texts ---
    /**
     * @struct LineRange
     * @brief Stores the start index and length of each line in the text buffer.
     */
    struct LineRange {
        size_t start = 0;   /**< Byte index of the line start. */
        size_t length = 0;  /**< Byte length of the line (excluding newline). */
    };
    std::vector<LineRange> m_lineRanges; /**< Cached line range list. */
    float m_cachedPreferredWidth = 0.0f; /**< Cached preferred width in dp. */
    float m_cachedPreferredHeight = 0.0f;/**< Cached preferred height in dp. */
    bool m_isDirty = true;               /**< Dirty flag for line cache and metrics. */

    /**
     * @brief Rebuilds line ranges and updates preferred dimensions.
     * 
     * Scans the text once, splits by '\n', calculates line widths using a proportional font model,
     * and stores the resulting metrics. This method is called automatically before rendering or
     * measuring when the dirty flag is set.
     */
    void updateTextLayoutMetrics();

public:
    /** @brief Constructs a default TextView instance with WRAP_CONTENT sizing. */
    TextView();

    /** @brief Virtual destructor. */
    virtual ~TextView() override = default;

    /**
     * @brief Sets the text content to be displayed.
     * @param text UTF-8 encoded string.
     */
    void setText(const std::string& text);

    /** 
     * @brief Retrieves the current text content.
     * @return Const reference to the internal UTF-8 string.
     */
    const std::string& getText() const { return m_text; }

    /**
     * @brief Sets the font size in density-independent pixels.
     * @param sizeDp Font size in dp.
     */
    void setTextSize(float sizeDp);

    /** 
     * @brief Retrieves the current font size.
     * @return Font size in dp.
     */
    float getTextSize() const { return m_textSize; }

    /**
     * @brief Sets the horizontal text alignment.
     * @param align Desired alignment (Left, Center, or Right).
     */
    void setTextAlignment(TextAlignment align) { m_alignment = align; }

    /**
     * @brief Retrieves the current horizontal alignment.
     * @return Current TextAlignment value.
     */
    TextAlignment getTextAlignment() const { return m_alignment; }

    /**
     * @brief Configures line spacing parameters.
     * @param extraDp Absolute additional spacing between lines in dp.
     * @param mult Multiplier applied to the base font height (default 1.0).
     */
    void setLineSpacing(float extraDp, float mult = 1.0f) {
        m_lineSpacingExtra = extraDp;
        m_lineSpacingMult = mult;
        m_isDirty = true;
    }

    /**
     * @brief Sets an explicit text color using an M3Color structure.
     * @param color The M3Color value.
     */
    void setTextColor(const M3Color& color);

    /**
     * @brief Parses and applies text color from a string descriptor.
     * 
     * Supports hexadecimal ("#RRGGBB", "#RRGGBBAA"), RGB/RGBA ("rgba(r,g,b,a)"),
     * or Material 3 Theme tokens ("primary", "onSurfaceVariant", etc.).
     * 
     * @param colorStr Target color descriptor string.
     */
    void setTextColor(const std::string& colorStr);

    /**
     * @brief Sets text color using RGBA byte integer components [0, 255].
     * @param r Red component (0-255).
     * @param g Green component (0-255).
     * @param b Blue component (0-255).
     * @param a Alpha component (0-255, defaults to 255).
     */
    void setTextColor(int r, int g, int b, int a = 255);

    /**
     * @brief Sets an inline icon positioned before the text.
     * @param icon Formatted Icon structure.
     */
    void setIcon(const Icon& icon) { m_icon = icon; m_hasIcon = !isIconEmpty(icon); m_isDirty = true; }

    /**
     * @brief Sets an inline icon using an icon token string.
     * @param str Icon identifier token or path.
     */
    void setIcon(const std::string& str) { m_icon = iconFromString(str); m_hasIcon = !isIconEmpty(m_icon); m_isDirty = true; }
    
    /**
     * @brief Sets a custom inline icon bounding dimension.
     * @param sizeDp Target icon size in dp.
     */
    void setIconSize(float sizeDp) { m_iconSizeDp = sizeDp; m_isDirty = true; }

    /**
     * @brief Computes the preferred layout width based on text content and icon.
     * @return Ideal preferred width in dp.
     */
    float getPreferredWidth() override;

    /**
     * @brief Computes the preferred layout height accounting for multi-line breaks and line spacing.
     * @return Ideal preferred height in dp.
     */
    float getPreferredHeight() override;

    /**
     * @brief Renders the typography component with viewport culling for performance.
     * @param renderer Reference to the active MaterialShader renderer.
     * @param theme Reference to the active MaterialTheme.
     */
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
};

/**
 * @class TextViewBuilder
 * @brief Fluent builder for constructing configured TextView instances.
 * 
 * Provides method-chaining syntax for setting all TextView properties before final building.
 */
class TextViewBuilder {
private:
    TextView* m_view; /**< Pointer to the managed TextView instance. */

public:
    /** @brief Constructs a builder with a fresh TextView instance. */
    TextViewBuilder() { m_view = new TextView(); }

    /** @brief Default destructor. */
    ~TextViewBuilder() = default;

    /**
     * @brief Sets display text content.
     * @param val UTF-8 encoded string.
     * @return Reference to this builder for chaining.
     */
    TextViewBuilder& text(const std::string& val) { m_view->setText(val); return *this; }

    /**
     * @brief Sets typographical font size.
     * @param sizeDp Font size in dp.
     * @return Reference to this builder.
     */
    TextViewBuilder& textSize(float sizeDp) { m_view->setTextSize(sizeDp); return *this; }

    /**
     * @brief Sets horizontal text alignment.
     * @param align Alignment mode.
     * @return Reference to this builder.
     */
    TextViewBuilder& alignment(TextAlignment align) { m_view->setTextAlignment(align); return *this; }
    
    /**
     * @brief Sets line spacing parameters.
     * @param extraDp Additional spacing in dp.
     * @param mult Line height multiplier.
     * @return Reference to this builder.
     */
    TextViewBuilder& lineSpacing(float extraDp, float mult = 1.0f) { m_view->setLineSpacing(extraDp, mult); return *this; }

    /**
     * @brief Sets explicit text color.
     * @param color M3Color color descriptor.
     * @return Reference to this builder.
     */
    TextViewBuilder& textColor(const M3Color& color) { m_view->setTextColor(color); return *this; }

    /**
     * @brief Sets text color from a string descriptor or theme token.
     * @param colorStr Hexadecimal string, RGB/RGBA string, or theme token.
     * @return Reference to this builder.
     */
    TextViewBuilder& textColor(const std::string& colorStr) { m_view->setTextColor(colorStr); return *this; }

    /**
     * @brief Sets text color using discrete integer channel values.
     * @param r Red component [0,255].
     * @param g Green component [0,255].
     * @param b Blue component [0,255].
     * @param a Alpha component [0,255].
     * @return Reference to this builder.
     */
    TextViewBuilder& textColor(int r, int g, int b, int a = 255) { m_view->setTextColor(r, g, b, a); return *this; }

    /**
     * @brief Sets an inline leading icon.
     * @param icon Icon structure.
     * @return Reference to this builder.
     */
    TextViewBuilder& icon(const Icon& icon) { m_view->setIcon(icon); return *this; }

    /**
     * @brief Sets an inline leading icon by string token.
     * @param str Icon identifier.
     * @return Reference to this builder.
     */
    TextViewBuilder& icon(const std::string& str) { m_view->setIcon(str); return *this; }

    /**
     * @brief Sets inline icon size.
     * @param sizeDp Size in dp.
     * @return Reference to this builder.
     */
    TextViewBuilder& iconSize(float sizeDp) { m_view->setIconSize(sizeDp); return *this; }

    /**
     * @brief Sets explicit layout width.
     * @param w Width in dp, MATCH_PARENT, or WRAP_CONTENT.
     * @return Reference to this builder.
     */
    TextViewBuilder& layoutWidth(float w) { m_view->setLayoutParams(w, m_view->layout_height); return *this; }

    /**
     * @brief Sets explicit layout height.
     * @param h Height in dp, MATCH_PARENT, or WRAP_CONTENT.
     * @return Reference to this builder.
     */
    TextViewBuilder& layoutHeight(float h) { m_view->setLayoutParams(m_view->layout_width, h); return *this; }
    
    /**
     * @brief Sets external margins.
     * @param l Left margin.
     * @param t Top margin.
     * @param r Right margin.
     * @param b Bottom margin.
     * @return Reference to this builder.
     */
    TextViewBuilder& margins(float l, float t, float r, float b) { m_view->setMargins(l, t, r, b); return *this; }

    /**
     * @brief Sets internal padding.
     * @param l Left padding.
     * @param t Top padding.
     * @param r Right padding.
     * @param b Bottom padding.
     * @return Reference to this builder.
     */
    TextViewBuilder& padding(float l, float t, float r, float b) { m_view->setPadding(l, t, r, b); return *this; }

    /**
     * @brief Builds and returns the configured TextView instance.
     * @return Raw pointer to the newly created TextView.
     */
    TextView* build() { return m_view; }
};
