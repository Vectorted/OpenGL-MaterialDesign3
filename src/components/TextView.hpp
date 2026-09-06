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
 */
class TextView : public View {
private:
    std::string m_text;
    float m_textSize = 14.0f;

    float m_lineSpacingMult = 1.2f; 
    float m_lineSpacingExtra = 0.0f;
    TextAlignment m_alignment = TextAlignment::Left;

    M3Color m_color = { 0.0f, 0.0f, 0.0f, 1.0f };
    bool m_useThemeColor = true;
    std::string m_themeColorToken = "onSurface";
    
    Icon m_icon;
    bool m_hasIcon = false;
    float m_iconSizeDp = 0.0f;

public:
    /** @brief Constructs a default TextView instance with WRAP_CONTENT sizing. */
    TextView();

    /** @brief Virtual destructor. */
    virtual ~TextView() override = default;

    /**
     * @brief Sets text content to be displayed.
     * 
     * @param text UTF-8 encoded string.
     */
    void setText(const std::string& text);

    /** 
     * @brief Retrieves the current text content string. 
     * 
     * @return Const reference to internal UTF-8 text string.
     */
    const std::string& getText() const { return m_text; }

    /**
     * @brief Sets font size in dp.
     * 
     * @param sizeDp Font size measured in density-independent pixels.
     */
    void setTextSize(float sizeDp);

    /** 
     * @brief Retrieves the current configured font size in dp. 
     * 
     * @return Configured font size in density-independent pixels.
     */
    float getTextSize() const { return m_textSize; }

    /**
     * @brief Sets horizontal text alignment mode.
     * 
     * @param align Target horizontal alignment (Left, Center, or Right).
     */
    void setTextAlignment(TextAlignment align) { m_alignment = align; }

    /**
     * @brief Retrieves the current horizontal text alignment mode.
     * 
     * @return Current TextAlignment enumeration value.
     */
    TextAlignment getTextAlignment() const { return m_alignment; }

    /**
     * @brief Configures typographical line spacing parameters.
     * 
     * @param extraDp Absolute spatial offset added to each line break in dp.
     * @param mult Multiplier applied to the base font height.
     */
    void setLineSpacing(float extraDp, float mult = 1.0f) {
        m_lineSpacingExtra = extraDp;
        m_lineSpacingMult = mult;
    }

    /**
     * @brief Sets explicit text color using an M3Color struct.
     * 
     * @param color Direct M3Color structure.
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
     * 
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @param a Alpha component (defaults to 255).
     */
    void setTextColor(int r, int g, int b, int a = 255);

    /**
     * @brief Sets an inline icon positioned before the text.
     * 
     * @param icon Formatted Icon structure.
     */
    void setIcon(const Icon& icon) { m_icon = icon; m_hasIcon = !isIconEmpty(icon); }

    /**
     * @brief Sets an inline icon using an icon token string.
     * 
     * @param str Icon identifier token or path.
     */
    void setIcon(const std::string& str) { m_icon = iconFromString(str); m_hasIcon = !isIconEmpty(m_icon); }
    
    /**
     * @brief Sets custom inline icon bounding dimension in dp.
     * 
     * @param sizeDp Target icon size in density-independent pixels.
     */
    void setIconSize(float sizeDp) { m_iconSizeDp = sizeDp; }

    /**
     * @brief Computes preferred layout width based on proportional font advance models.
     * 
     * @return Ideal preferred bounding width in dp.
     */
    float getPreferredWidth() override;

    /**
     * @brief Computes preferred layout height accounting for multi-line breaks and line spacing.
     * 
     * @return Ideal preferred bounding height in dp.
     */
    float getPreferredHeight() override;

    /**
     * @brief Renders the typography component using hardware-accelerated FreeType rasterization.
     * 
     * @param renderer Reference to active MaterialShader hardware renderer pipeline.
     * @param theme Reference to active MaterialTheme styling palette.
     */
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
};

/**
 * @class TextViewBuilder
 * @brief Fluent builder helper for constructing configured TextView instances.
 */
class TextViewBuilder {
private:
    TextView* m_view;

public:
    /** @brief Constructs a TextViewBuilder with a fresh TextView instance. */
    TextViewBuilder() { m_view = new TextView(); }

    /** @brief Destructor. */
    ~TextViewBuilder() = default;

    /**
     * @brief Configures display text content.
     * 
     * @param val UTF-8 encoded string.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& text(const std::string& val) { m_view->setText(val); return *this; }

    /**
     * @brief Configures typographical font size.
     * 
     * @param sizeDp Font size in density-independent pixels.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& textSize(float sizeDp) { m_view->setTextSize(sizeDp); return *this; }

    /**
     * @brief Configures horizontal text alignment.
     * 
     * @param align Horizontal alignment mode.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& alignment(TextAlignment align) { m_view->setTextAlignment(align); return *this; }
    
    /**
     * @brief Configures line spacing parameters.
     * 
     * @param extraDp Absolute spatial offset in dp.
     * @param mult Line height scaling multiplier.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& lineSpacing(float extraDp, float mult = 1.0f) { m_view->setLineSpacing(extraDp, mult); return *this; }

    /**
     * @brief Configures direct text color structure.
     * 
     * @param color M3Color color descriptor.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& textColor(const M3Color& color) { m_view->setTextColor(color); return *this; }

    /**
     * @brief Configures text color using a string descriptor or theme token.
     * 
     * @param colorStr Hexadecimal string, RGB/RGBA string, or theme token.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& textColor(const std::string& colorStr) { m_view->setTextColor(colorStr); return *this; }

    /**
     * @brief Configures text color with discrete integer channel values.
     * 
     * @param r Red component [0, 255].
     * @param g Green component [0, 255].
     * @param b Blue component [0, 255].
     * @param a Alpha component [0, 255].
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& textColor(int r, int g, int b, int a = 255) { m_view->setTextColor(r, g, b, a); return *this; }

    /**
     * @brief Configures inline leading icon structure.
     * 
     * @param icon Formatted Icon structure.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& icon(const Icon& icon) { m_view->setIcon(icon); return *this; }

    /**
     * @brief Configures inline leading icon by string token.
     * 
     * @param str Icon identifier string.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& icon(const std::string& str) { m_view->setIcon(str); return *this; }

    /**
     * @brief Configures inline icon bounding dimension.
     * 
     * @param sizeDp Target icon size in dp.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& iconSize(float sizeDp) { m_view->setIconSize(sizeDp); return *this; }

    /**
     * @brief Configures explicit layout width in dp.
     * 
     * @param w Layout width in dp, MATCH_PARENT, or WRAP_CONTENT.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& layoutWidth(float w) { m_view->setLayoutParams(w, m_view->layout_height); return *this; }

    /**
     * @brief Configures explicit layout height in dp.
     * 
     * @param h Layout height in dp, MATCH_PARENT, or WRAP_CONTENT.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& layoutHeight(float h) { m_view->setLayoutParams(m_view->layout_width, h); return *this; }
    
    /**
     * @brief Sets external layout margins in dp.
     * 
     * @param l Left margin.
     * @param t Top margin.
     * @param r Right margin.
     * @param b Bottom margin.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& margins(float l, float t, float r, float b) { m_view->setMargins(l, t, r, b); return *this; }

    /**
     * @brief Sets internal padding in dp.
     * 
     * @param l Left padding.
     * @param t Top padding.
     * @param r Right padding.
     * @param b Bottom padding.
     * @return Reference to builder instance for method chaining.
     */
    TextViewBuilder& padding(float l, float t, float r, float b) { m_view->setPadding(l, t, r, b); return *this; }

    /**
     * @brief Allocates and returns the final configured TextView instance.
     * 
     * @return TextView* Raw pointer to newly constructed TextView.
     */
    TextView* build() { return m_view; }
};