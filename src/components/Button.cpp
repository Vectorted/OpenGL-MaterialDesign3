/**
 * @file Button.cpp
 * @brief Implementation of Button component layout metrics, interaction states, and rendering.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "Button.hpp"
#include "../shader/TextureLoader.hpp"
#include <algorithm>
#include <cmath>

/**
 * @brief Constructs a Button instance and initializes layout constraints.
 * 
 * @param type Material Design 3 button variant style.
 */
Button::Button(M3ButtonType type)
    : m_type(type) {
    layout_width = WRAP_CONTENT;
    layout_height = WRAP_CONTENT;
}

/**
 * @brief Updates the button state animations.
 * 
 * @param dt Delta time in seconds.
 */
void Button::update(float dt) {
    View::update(dt);
}

/**
 * @brief Handles the click action and invokes the click listener if registered.
 */
void Button::onClick() {
    View::onClick();
    if (m_onClickListener) {
        m_onClickListener();
    }
}

/**
 * @brief Computes preferred layout width based on text length, icon dimensions, and padding metrics.
 * 
 * @return Preferred width in device-independent pixels (dp).
 */
float Button::getPreferredWidth() {
    float fontWidth = static_cast<float>(m_text.length()) * m_textSizeDp * 0.6f;
    float iconSize = m_hasIcon ? m_iconSizeDp : 0.0f;
    float gap = m_hasIcon ? 8.0f : 0.0f;
    float padding = m_hasIcon ? (16.0f + 24.0f) : (24.0f * 2.0f);
    return std::max(80.0f, fontWidth + iconSize + gap + padding);
}

/**
 * @brief Computes preferred layout height conforming to Material Design 3 specifications.
 * 
 * @return Preferred height in device-independent pixels (dp).
 */
float Button::getPreferredHeight() {
    return 40.0f;
}

/**
 * @brief Renders the visual presentation of the Button component.
 * 
 * @param shader Reference to the active Material UI shader pipeline.
 * @param theme Reference to the Material color system theme tokens.
 */
void Button::render(MaterialShader& shader, MaterialTheme& theme) {
    M3Color btnColor = theme.primary;
    M3Color textColor = theme.onPrimary;
    M3Color borderColor = { 0, 0, 0, 0 };
    float borderWidth = 0.0f;

    if (m_type == M3ButtonType::Tonal) {
        btnColor = theme.secondaryContainer;
        textColor = theme.onSecondaryContainer;
    }
    else if (m_type == M3ButtonType::Outlined) {
        btnColor = { 0, 0, 0, 0 };
        textColor = theme.primary;
        borderColor = theme.outline;
        borderWidth = std::max(1.0f, std::round(dp(1.0f)));
    }
    else if (m_type == M3ButtonType::Text) {
        btnColor = { 0, 0, 0, 0 };
        textColor = theme.primary;
    }
    else if (m_type == M3ButtonType::Elevated) {
        btnColor = theme.surface;
        textColor = theme.primary;
    }

    float pX = std::round(x);
    float pY = std::round(y);
    float pW = std::round(width);
    
    float rawH = height > 0.0f ? height : dp(40.0f);
    float pH = std::floor(std::round(rawH) * 0.5f) * 2.0f;
    float btnRadius = pH * 0.5f;

    float centerY = pY + pH * 0.5f;

    if (m_type == M3ButtonType::Elevated) {
        float shadowAlpha = pressAnim > 0.01f ? 0.06f : (hoverAnim > 0.01f ? 0.16f : 0.12f);
        float shadowOffsetY = std::round(dp(pressAnim > 0.01f ? 1.0f : (hoverAnim > 0.01f ? 3.0f : 1.5f)));
        float shadowBlur = std::round(dp(pressAnim > 0.01f ? 3.0f : (hoverAnim > 0.01f ? 8.0f : 4.0f)));

        M3Color shadowCol = theme.onSurface;
        shadowCol.a = shadowAlpha;
        float pad = shadowBlur;

        shader.drawM3UI(
            pX - pad, pY + shadowOffsetY - pad, pW + 2.0f * pad, pH + 2.0f * pad,
            btnRadius + pad, btnRadius + pad, btnRadius + pad, btnRadius + pad,
            shadowCol,
            0.0f, 0.0f, 0.0f, 0.0f, { 0,0,0,0 },
            0, 0.0f, { 0,0,0,0 }, { 0,0,0,0 }, 0.0f,
            0, false, 0.0f, 0.0f, 1.0f, 1.0f,
            shadowBlur
        );
    }

    M3Color finalBtnColor = btnColor;
    if (hoverAnim > 0.0f || pressAnim > 0.0f) {
        float overlayAlpha = (0.08f * hoverAnim + 0.12f * pressAnim);
        float outA = overlayAlpha + finalBtnColor.a * (1.0f - overlayAlpha);
        if (outA > 0.001f) {
            finalBtnColor.r = (textColor.r * overlayAlpha + finalBtnColor.r * finalBtnColor.a * (1.0f - overlayAlpha)) / outA;
            finalBtnColor.g = (textColor.g * overlayAlpha + finalBtnColor.g * finalBtnColor.a * (1.0f - overlayAlpha)) / outA;
            finalBtnColor.b = (textColor.b * overlayAlpha + finalBtnColor.b * finalBtnColor.a * (1.0f - overlayAlpha)) / outA;
            finalBtnColor.a = outA;
        }
    }

    shader.drawM3UI(
        pX, pY, pW, pH,
        btnRadius, btnRadius, btnRadius, btnRadius,
        finalBtnColor,
        ripple.localX, ripple.localY, ripple.radius, ripple.alpha, theme.primary,
        0, 0.0f, { 0,0,0,0 }, borderColor, borderWidth
    );

    shader.useFont(m_fontPath);

    float textPhysicalSize = dp(m_textSizeDp);
    float fontWidth = shader.getTextWidth(m_text, textPhysicalSize);
    
    float iconSize = std::floor(dp(m_iconSizeDp) * 0.5f) * 2.0f;
    float iconSpacing = std::round(dp(8.0f));
    float totalContentWidth = fontWidth;

    bool hasIconTarget = m_hasIcon && !isIconEmpty(m_icon);
    if (hasIconTarget) {
        totalContentWidth += iconSize + iconSpacing;
    }

    float startX = std::round(pX + (pW - totalContentWidth) * 0.5f);
    float curX = startX;

    float iconY = centerY - iconSize * 0.5f;
    float textY = std::round(centerY - textPhysicalSize * 0.5f);

    if (hasIconTarget) {
        shader.drawIcon(curX, iconY, iconSize, m_icon, textColor);
        curX = std::round(curX + iconSize + iconSpacing);
    }

    shader.drawText(m_text, curX, textY, textPhysicalSize, textColor);
    shader.useFont("");
}

/**
 * @brief Sets the button style to Filled.
 * 
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::filled() { m_type = M3ButtonType::Filled; return *this; }

/**
 * @brief Sets the button style to Tonal.
 * 
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::tonal() { m_type = M3ButtonType::Tonal; return *this; }

/**
 * @brief Sets the button style to Outlined.
 * 
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::outlined() { m_type = M3ButtonType::Outlined; return *this; }

/**
 * @brief Sets the button style to Text.
 * 
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::text() { m_type = M3ButtonType::Text; return *this; }

/**
 * @brief Sets the button style to Elevated.
 * 
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::elevated() { m_type = M3ButtonType::Elevated; return *this; }

/**
 * @brief Sets the button text label.
 * 
 * @param txt Text string.
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::text(const std::string& txt) { m_text = txt; return *this; }

/**
 * @brief Sets the typography font size in dp.
 * 
 * @param sizeDp Font size in dp.
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::textSize(float sizeDp) { m_textSizeDp = sizeDp; return *this; }

/**
 * @brief Sets the icon size in dp.
 * 
 * @param sizeDp Icon dimension in dp.
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::iconSize(float sizeDp) { m_iconSizeDp = sizeDp; return *this; }

/**
 * @brief Sets the custom font path.
 * 
 * @param fontPath Custom font resource path.
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::font(const std::string& fontPath) { m_fontPath = fontPath; return *this; }

/**
 * @brief Sets the icon using an Icon struct descriptor.
 * 
 * @param icon Icon descriptor.
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::icon(const Icon& icon) { m_icon = icon; m_hasIcon = !isIconEmpty(icon); return *this; }

/**
 * @brief Sets the icon using a resource string identifier.
 * 
 * @param iconPath Identifier or file path string.
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::icon(const std::string& iconPath) { m_icon = iconFromString(iconPath); m_hasIcon = !isIconEmpty(m_icon); return *this; }

/**
 * @brief Sets the layout width in dp.
 * 
 * @param w Width in dp.
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::width(float w) { m_widthDp = w; return *this; }

/**
 * @brief Sets the layout height in dp.
 * 
 * @param h Height in dp.
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::height(float h) { m_heightDp = h; return *this; }

/**
 * @brief Sets whether an icon should be rendered.
 * 
 * @param val True if an icon should be active.
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::withIcon(bool val) { m_hasIcon = val; if (!val) m_icon = IconNone(); return *this; }

/**
 * @brief Sets outer margin values in dp.
 * 
 * @param left Left margin in dp.
 * @param top Top margin in dp.
 * @param right Right margin in dp.
 * @param bottom Bottom margin in dp.
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::margins(float left, float top, float right, float bottom) {
    m_marginLeft = left; m_marginTop = top; m_marginRight = right; m_marginBottom = bottom;
    return *this;
}

/**
 * @brief Sets inner padding values in dp.
 * 
 * @param left Left padding in dp.
 * @param top Top padding in dp.
 * @param right Right padding in dp.
 * @param bottom Bottom padding in dp.
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::padding(float left, float top, float right, float bottom) {
    m_paddingLeft = left; m_paddingTop = top; m_paddingRight = right; m_paddingBottom = bottom;
    return *this;
}

/**
 * @brief Sets the click listener callback.
 * 
 * @param cb Click callback function.
 * @return Reference to the builder.
 */
ButtonBuilder& ButtonBuilder::setOnClickListener(std::function<void()> cb) { m_onClickListener = std::move(cb); return *this; }

/**
 * @brief Constructs and initializes a Button instance using current builder properties.
 * 
 * @return Pointer to the newly created Button instance.
 */
Button* ButtonBuilder::build() {
    Button* btn = new Button(m_type);
    btn->layout_width = m_widthDp;
    btn->layout_height = m_heightDp;
    btn->m_hasIcon = m_hasIcon;
    btn->m_textSizeDp = m_textSizeDp;
    btn->m_iconSizeDp = m_iconSizeDp;
    btn->m_fontPath = m_fontPath;
    btn->m_icon = m_icon;
    btn->m_onClickListener = std::move(m_onClickListener);

    btn->setMargins(m_marginLeft, m_marginTop, m_marginRight, m_marginBottom);
    btn->setPadding(m_paddingLeft, m_paddingTop, m_paddingRight, m_paddingBottom);

    if (m_text.empty()) {
        if (m_type == M3ButtonType::Filled) btn->m_text = "Filled Button";
        else if (m_type == M3ButtonType::Tonal) btn->m_text = "Tonal Button";
        else if (m_type == M3ButtonType::Outlined) btn->m_text = "Outlined Button";
        else if (m_type == M3ButtonType::Elevated) btn->m_text = "Elevated Button";
        else if (m_type == M3ButtonType::Text) btn->m_text = "Text Button";
    }
    else {
        btn->m_text = m_text;
    }
    return btn;
}