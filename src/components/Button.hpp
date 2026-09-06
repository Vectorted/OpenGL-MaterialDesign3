/**
 * @file Button.hpp
 * @brief Interactive button component and builder conforming to Material Design 3 guidelines.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include <string>
#include <functional>
#include "../layout/View.hpp"
#include "../shader/Icon.hpp"

/**
 * @class Button
 * @brief Material Design 3 button supporting filled, tonal, outlined, text, and elevated styles.
 */
class Button : public View {
public:
    /**
     * @brief Text label displayed on the button.
     */
    std::string m_text = "";

    /**
     * @brief Visual style variant of the button.
     */
    M3ButtonType m_type = M3ButtonType::Filled;

    /**
     * @brief Icon descriptor rendered alongside the label.
     */
    Icon m_icon;

    /**
     * @brief Flag indicating whether the icon is active and visible.
     */
    bool m_hasIcon = false;

    /**
     * @brief Typography font size in device-independent pixels (dp).
     */
    float m_textSizeDp = 14.0f;

    /**
     * @brief Icon bounding dimension in device-independent pixels (dp).
     */
    float m_iconSizeDp = 18.0f;

    /**
     * @brief Optional path to a custom font file.
     */
    std::string m_fontPath = "";

    /**
     * @brief Callback handler triggered when the button is clicked.
     */
    std::function<void()> m_onClickListener = nullptr;

    /**
     * @brief Constructs a Button instance with a specific visual variant.
     * 
     * @param type Material Design 3 button variant style.
     */
    Button(M3ButtonType type);

    /**
     * @brief Updates internal animation states and timers.
     * 
     * @param dt Delta time since the last update frame in seconds.
     */
    void update(float dt) override;

    /**
     * @brief Renders the button background, state overlays, icon, and label.
     * 
     * @param shader Reference to the Material UI shader renderer.
     * @param theme Reference to the active Material theme palette.
     */
    void render(MaterialShader& shader, MaterialTheme& theme) override;

    /**
     * @brief Handles the click action and invokes the registered listener.
     */
    void onClick() override;

    /**
     * @brief Calculates the preferred width of the button based on content and metrics.
     * 
     * @return Preferred width in device-independent pixels (dp).
     */
    float getPreferredWidth() override;

    /**
     * @brief Calculates the preferred height of the button.
     * 
     * @return Preferred height in device-independent pixels (dp).
     */
    float getPreferredHeight() override;

    /**
     * @brief Sets the button display text.
     * 
     * @param text String representing the new label text.
     */
    void setText(const std::string& text) { m_text = text; }

    /**
     * @brief Sets the click event callback handler.
     * 
     * @param cb Callback function to be executed on click.
     */
    void setOnClickListener(std::function<void()> cb) { m_onClickListener = std::move(cb); }

    /**
     * @brief Sets the label typography size.
     * 
     * @param sizeDp Font size in device-independent pixels (dp).
     */
    void setTextSize(float sizeDp) { m_textSizeDp = sizeDp; }

    /**
     * @brief Sets the icon size.
     * 
     * @param sizeDp Icon dimension in device-independent pixels (dp).
     */
    void setIconSize(float sizeDp) { m_iconSizeDp = sizeDp; }

    /**
     * @brief Sets the button icon using a typed Icon structure.
     * 
     * @param icon Icon variant descriptor.
     */
    void setIcon(const Icon& icon) { m_icon = icon; m_hasIcon = !isIconEmpty(icon); }

    /**
     * @brief Sets the button icon using a resource string identifier or path.
     * 
     * @param str String identifier representing the icon resource.
     */
    void setIcon(const std::string& str) { m_icon = iconFromString(str); m_hasIcon = !isIconEmpty(m_icon); }

    /**
     * @brief Sets the path to a custom font asset.
     * 
     * @param fontPath File path to the custom font.
     */
    void setFont(const std::string& fontPath) { m_fontPath = fontPath; }
};

/**
 * @class ButtonBuilder
 * @brief Fluent builder helper for constructing configured Button instances.
 */
class ButtonBuilder {
private:
    /**
     * @brief Configured button label text.
     */
    std::string m_text = "";

    /**
     * @brief Configured button visual variant.
     */
    M3ButtonType m_type = M3ButtonType::Filled;

    /**
     * @brief Configured layout width in dp.
     */
    float m_widthDp = WRAP_CONTENT;

    /**
     * @brief Configured layout height in dp.
     */
    float m_heightDp = WRAP_CONTENT;

    /**
     * @brief Configured text size in dp.
     */
    float m_textSizeDp = 14.0f;

    /**
     * @brief Configured icon size in dp.
     */
    float m_iconSizeDp = 18.0f;

    /**
     * @brief Configured icon active state flag.
     */
    bool m_hasIcon = false;

    /**
     * @brief Configured custom font path.
     */
    std::string m_fontPath = "";

    /**
     * @brief Configured icon descriptor.
     */
    Icon m_icon;

    /**
     * @brief Left outer margin in dp.
     */
    float m_marginLeft = 0.0f;

    /**
     * @brief Top outer margin in dp.
     */
    float m_marginTop = 0.0f;

    /**
     * @brief Right outer margin in dp.
     */
    float m_marginRight = 0.0f;

    /**
     * @brief Bottom outer margin in dp.
     */
    float m_marginBottom = 0.0f;

    /**
     * @brief Left inner padding in dp.
     */
    float m_paddingLeft = 0.0f;

    /**
     * @brief Right inner padding in dp.
     */
    float m_paddingRight = 0.0f;

    /**
     * @brief Top inner padding in dp.
     */
    float m_paddingTop = 0.0f;

    /**
     * @brief Bottom inner padding in dp.
     */
    float m_paddingBottom = 0.0f;

    /**
     * @brief Configured click event listener callback.
     */
    std::function<void()> m_onClickListener = nullptr;

public:
    /**
     * @brief Default constructor for ButtonBuilder.
     */
    ButtonBuilder() = default;

    /**
     * @brief Sets button variant to Filled.
     * 
     * @return Reference to this builder instance.
     */
    ButtonBuilder& filled();

    /**
     * @brief Sets button variant to Tonal.
     * 
     * @return Reference to this builder instance.
     */
    ButtonBuilder& tonal();

    /**
     * @brief Sets button variant to Outlined.
     * 
     * @return Reference to this builder instance.
     */
    ButtonBuilder& outlined();

    /**
     * @brief Sets button variant to Text.
     * 
     * @return Reference to this builder instance.
     */
    ButtonBuilder& text();

    /**
     * @brief Sets button variant to Elevated.
     * 
     * @return Reference to this builder instance.
     */
    ButtonBuilder& elevated();

    /**
     * @brief Sets the button display text.
     * 
     * @param txt Display label string.
     * @return Reference to this builder instance.
     */
    ButtonBuilder& text(const std::string& txt);

    /**
     * @brief Sets the label text size in dp.
     * 
     * @param sizeDp Font size in device-independent pixels.
     * @return Reference to this builder instance.
     */
    ButtonBuilder& textSize(float sizeDp);

    /**
     * @brief Sets the icon size in dp.
     * 
     * @param sizeDp Icon dimension in device-independent pixels.
     * @return Reference to this builder instance.
     */
    ButtonBuilder& iconSize(float sizeDp);

    /**
     * @brief Sets the custom font asset path.
     * 
     * @param fontPath File path to the custom font.
     * @return Reference to this builder instance.
     */
    ButtonBuilder& font(const std::string& fontPath);

    /**
     * @brief Sets the button icon descriptor.
     * 
     * @param icon Icon structure descriptor.
     * @return Reference to this builder instance.
     */
    ButtonBuilder& icon(const Icon& icon);

    /**
     * @brief Sets the button icon by identifier string.
     * 
     * @param iconPath Resource identifier or asset path for the icon.
     * @return Reference to this builder instance.
     */
    ButtonBuilder& icon(const std::string& iconPath);

    /**
     * @brief Sets layout width in dp.
     * 
     * @param w Desired layout width.
     * @return Reference to this builder instance.
     */
    ButtonBuilder& width(float w);

    /**
     * @brief Sets layout height in dp.
     * 
     * @param h Desired layout height.
     * @return Reference to this builder instance.
     */
    ButtonBuilder& height(float h);

    /**
     * @brief Explicitly enables or disables icon rendering.
     * 
     * @param val True to enable icon rendering, false otherwise.
     * @return Reference to this builder instance.
     */
    ButtonBuilder& withIcon(bool val);

    /**
     * @brief Configures outer margins for the button layout.
     * 
     * @param left Left margin in dp.
     * @param top Top margin in dp.
     * @param right Right margin in dp.
     * @param bottom Bottom margin in dp.
     * @return Reference to this builder instance.
     */
    ButtonBuilder& margins(float left, float top, float right, float bottom);

    /**
     * @brief Configures inner padding for the button layout.
     * 
     * @param left Left padding in dp.
     * @param top Top padding in dp.
     * @param right Right padding in dp.
     * @param bottom Bottom padding in dp.
     * @return Reference to this builder instance.
     */
    ButtonBuilder& padding(float left, float top, float right, float bottom);

    /**
     * @brief Registers a click listener callback.
     * 
     * @param cb Callback function invoked upon clicking the button.
     * @return Reference to this builder instance.
     */
    ButtonBuilder& setOnClickListener(std::function<void()> cb);

    /**
     * @brief Allocates and constructs the configured Button instance.
     * 
     * @return Pointer to the heap-allocated Button instance.
     */
    Button* build();
};