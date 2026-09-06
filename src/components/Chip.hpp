/**
 * @file Chip.hpp
 * @brief Material Design 3 Chip component definitions supporting Assist, Filter, Input, and Suggestion variants.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include "../layout/View.hpp"
#include <string>
#include <functional>

/**
 * @enum ChipType
 * @brief Chip component variant types conforming to Material Design 3 specifications.
 */
enum class ChipType {
    Assist,      /**< Action chip guiding contextual user operations. */
    Filter,      /**< Filter selection chip toggling discrete filtering states. */
    Input,       /**< Input token chip representing discrete information pieces with a remove action. */
    Suggestion   /**< Recommendation chip suggesting dynamic quick responses. */
};

/**
 * @class Chip
 * @brief Compact interactive element representing an attribute, action, or filter.
 */
class Chip : public View {
private:
    /**
     * @brief Chip visual style variant.
     */
    ChipType m_type;

    /**
     * @brief Text label displayed on the chip.
     */
    std::string m_text;

    /**
     * @brief Resource identifier or path for the leading icon.
     */
    std::string m_leadingIcon;

    /**
     * @brief Resource identifier or path for the trailing icon.
     */
    std::string m_trailingIcon;

    /**
     * @brief Current selected state flag.
     */
    bool m_selected = false;

    /**
     * @brief Indicates whether the chip supports selection toggling.
     */
    bool m_selectable = false;

    /**
     * @brief Indicates whether elevation shadow styling is applied.
     */
    bool m_elevated = false;

    /**
     * @brief Indicates whether the chip responds to user interactions.
     */
    bool m_enabled = true;

    /**
     * @brief Normalized animation progress for selection transition.
     */
    float m_selectAnim = 0.0f;

    /**
     * @brief Alpha value for hover and press overlay rendering.
     */
    float m_stateAlpha = 0.0f;

    /**
     * @brief Hover state flag for the chip body.
     */
    bool m_isHovered = false;

    /**
     * @brief Pressed state flag for the chip body.
     */
    bool m_isPressed = false;

    /**
     * @brief Hover state flag specifically for the trailing icon region.
     */
    bool m_isTrailingHovered = false;

    /**
     * @brief Horizontal starting coordinate of trailing icon hitbox in physical pixels.
     */
    float m_trailingBoundsX = 0.0f;

    /**
     * @brief Width of trailing icon hitbox in physical pixels.
     */
    float m_trailingBoundsW = 0.0f;

    /**
     * @brief Callback invoked when selection state changes.
     */
    std::function<void(bool)> m_onSelectedChange;

    /**
     * @brief Callback invoked when the trailing icon is clicked.
     */
    std::function<void()> m_onTrailingClick;

    /**
     * @brief Callback invoked when the chip body is clicked.
     */
    std::function<void()> m_onClickListener;

    /**
     * @brief Computes total content width based on typography and icon dimensions.
     * 
     * @param renderer Reference to the shader renderer for measuring font bounds.
     * @param outContentW Output reference receiving total content width in pixels.
     * @param outTextW Output reference receiving text width in pixels.
     */
    void calculateContentWidth(MaterialShader& renderer, float& outContentW, float& outTextW);

    /**
     * @brief Computes left padding depending on presence of leading icons.
     * 
     * @return Left padding value in physical pixels.
     */
    float padLeft();

public:
    /**
     * @brief Constructs a Chip component instance.
     * 
     * @param type Chip variant type.
     * @param text Text label for the chip.
     */
    Chip(ChipType type = ChipType::Assist, const std::string& text = "");

    /**
     * @brief Sets the chip variant type.
     * 
     * @param type Chip variant type.
     * @return Pointer to this Chip instance.
     */
    Chip* setType(ChipType type);

    /**
     * @brief Sets the text label.
     * 
     * @param text Text string.
     * @return Pointer to this Chip instance.
     */
    Chip* setText(const std::string& text);

    /**
     * @brief Sets the leading icon resource path or identifier.
     * 
     * @param iconPath Resource identifier or asset path.
     * @return Pointer to this Chip instance.
     */
    Chip* setLeadingIcon(const std::string& iconPath);

    /**
     * @brief Sets the trailing icon resource path or identifier.
     * 
     * @param iconPath Resource identifier or asset path.
     * @return Pointer to this Chip instance.
     */
    Chip* setTrailingIcon(const std::string& iconPath);

    /**
     * @brief Sets the selection state.
     * 
     * @param selected True to select the chip, false otherwise.
     * @return Pointer to this Chip instance.
     */
    Chip* setSelected(bool selected);

    /**
     * @brief Sets whether the chip can be toggled by user interaction.
     * 
     * @param selectable True if selectable, false otherwise.
     * @return Pointer to this Chip instance.
     */
    Chip* setSelectable(bool selectable);

    /**
     * @brief Sets whether the chip renders with an elevated surface style.
     * 
     * @param elevated True if elevated, false otherwise.
     * @return Pointer to this Chip instance.
     */
    Chip* setElevated(bool elevated);

    /**
     * @brief Sets whether the chip is enabled for user interaction.
     * 
     * @param enabled True to enable, false to disable.
     * @return Pointer to this Chip instance.
     */
    Chip* setEnabled(bool enabled);

    /**
     * @brief Sets the selection change callback listener.
     * 
     * @param listener Callback function receiving the new selection state.
     * @return Pointer to this Chip instance.
     */
    Chip* setOnSelectedChangeListener(std::function<void(bool)> listener);

    /**
     * @brief Sets the trailing icon click callback listener.
     * 
     * @param listener Callback function invoked upon clicking the trailing icon.
     * @return Pointer to this Chip instance.
     */
    Chip* setOnTrailingIconClickListener(std::function<void()> listener);

    /**
     * @brief Sets the main chip click callback listener.
     * 
     * @param listener Callback function invoked upon clicking the chip body.
     * @return Pointer to this Chip instance.
     */
    Chip* setOnClickListener(std::function<void()> listener);

    /**
     * @brief Computes preferred layout width based on content metrics.
     * 
     * @return Preferred width in device-independent pixels (dp).
     */
    float getPreferredWidth() override;

    /**
     * @brief Computes preferred layout height conforming to Material Design 3 guidelines.
     * 
     * @return Preferred height in device-independent pixels (dp).
     */
    float getPreferredHeight() override;

    /**
     * @brief Updates state animations and interaction transitions.
     * 
     * @param dt Delta time in seconds.
     */
    void update(float dt) override;

    /**
     * @brief Calculates absolute positions and bounds during layout resolution.
     * 
     * @param parentX Parent origin X coordinate in pixels.
     * @param parentY Parent origin Y coordinate in pixels.
     * @param parentW Parent allocated width in pixels.
     * @param parentH Parent allocated height in pixels.
     */
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;

    /**
     * @brief Renders the chip background, borders, icons, text, and state overlays.
     * 
     * @param renderer Reference to the active Material UI shader pipeline.
     * @param theme Reference to the active Material color tokens.
     */
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    /**
     * @brief Processes mouse cursor movements to update hover states.
     * 
     * @param mx Mouse X position in window coordinates.
     * @param my Mouse Y position in window coordinates.
     * @return True if the mouse is hovering over the chip, false otherwise.
     */
    bool handleMouseMove(float mx, float my) override;

    /**
     * @brief Processes mouse button press and release events.
     * 
     * @param button Mouse button index.
     * @param action Action type (press or release).
     * @param mx Mouse X position in window coordinates.
     * @param my Mouse Y position in window coordinates.
     * @return True if the event was consumed by the chip, false otherwise.
     */
    bool handleMouseButton(int button, int action, float mx, float my) override;
};