/**
 * @file MediaSlider.hpp
 * @brief Versatile media playback slider supporting animated squiggly waveforms, standard, thick capsule, and progress styles.
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
#include <functional>

/**
 * @enum SliderStyle
 * @brief Visual presentation and interaction style for the media slider.
 */
enum class SliderStyle {
    Squiggly,       /**< Dynamic animated sine wave track active during media playback. */
    Standard,       /**< Classic Material Design thin track with prominent grab thumb. */
    Thick,          /**< Thickened capsule format optimized for touch-based adjustments. */
    LinearProgress  /**< Static progress indicator bar ignoring user input interactions. */
};

/**
 * @class MediaSlider
 * @brief Interactive slider component with exclusive mouse capture and sine wave kinetics.
 */
class MediaSlider : public View {
private:
    float m_value = 0.35f;
    float m_phase = 0.0f;

    bool m_isHovered = false;
    bool m_isDragging = false;
    bool m_enabled = true;
    bool m_isPlaying = true;

    SliderStyle m_style = SliderStyle::Squiggly;

    std::function<void(float)> m_onValueChanged;

public:
    /**
     * @brief Constructs a MediaSlider with an initial normalized progress value.
     * 
     * @param initialValue Normalized progress factor in [0.0, 1.0].
     */
    MediaSlider(float initialValue = 0.35f);

    /** @brief Retrieves the normalized progress value in [0.0, 1.0]. */
    float getValue() const;

    /**
     * @brief Sets progress value programmatically.
     * 
     * @param value Normalized target value in [0.0, 1.0].
     * @return MediaSlider* Pointer to this instance for chaining.
     */
    MediaSlider* setValue(float value);

    /** @brief Enables or disables user interaction. */
    MediaSlider* setEnabled(bool enabled);

    /** @brief Sets media playback state driving the squiggly sine wave animation. */
    MediaSlider* setPlaying(bool playing);

    /** @brief Configures the slider presentation style. */
    MediaSlider* setStyle(SliderStyle style);

    /** @brief Sets the value change callback listener. */
    MediaSlider* setOnValueChanged(std::function<void(float)> listener);

    float getPreferredWidth() override;
    float getPreferredHeight() override;
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;

    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
};