/**
 * @file MediaSlider.hpp
 * @brief Material Design 3 Media & Discrete Slider with Tick Marks, Capsule Gap, and Value Tooltip.
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
#include <string>

/**
 * @enum SliderStyle
 * @brief Visual presentation and interaction style for the slider.
 */
enum class SliderStyle {
    Squiggly,       /**< Dynamic animated sine wave track active during media playback. */
    Standard,       /**< Classic Material Design thin track with prominent grab thumb. */
    Thick,          /**< Modern Material 3 Thick Capsule style with gap and vertical handle. */
    LinearProgress  /**< Static progress indicator bar ignoring user input interactions. */
};

/**
 * @class MediaSlider
 * @brief Interactive slider component conforming to Material Design 3 specification.
 * 
 * Supports multiple visual styles (Thick, Standard, Squiggly, LinearProgress),
 * discrete stepping with tick marks, time display, value tooltip on drag, and
 * smooth hover/drag animations. Implements exclusive drag capture to prevent
 * event conflicts with other interactive components.
 */
class MediaSlider : public View {
private:
    float m_value = 0.35f;            /**< Normalized progress [0.0 – 1.0]. */
    float m_phase = 0.0f;             /**< Phase angle for squiggly wave animation (radians). */

    bool m_isHovered = false;         /**< Whether the mouse is hovering over the slider. */
    bool m_isDragging = false;        /**< Whether the slider is currently being dragged. */
    bool m_enabled = true;            /**< If false, interaction is disabled and colors dim. */
    bool m_isPlaying = true;          /**< Drives the squiggly wave animation when true. */

    SliderStyle m_style = SliderStyle::Thick; /**< Current visual style. */

    // Time display support
    bool m_showTime = false;          /**< Whether to render time labels at track ends. */
    float m_currentSeconds = 180.0f;  /**< Current playback position in seconds. */
    float m_totalSeconds = 202.0f;    /**< Total duration in seconds. */

    // Discrete slider / tick marks support
    int m_steps = 0;                  /**< Number of discrete steps (>0 enables snapping). */
    bool m_discrete = false;          /**< Whether to snap to the nearest tick on release/drag. */
    bool m_showTicks = false;         /**< Whether to render tick marks as small dots. */
    bool m_showValueIndicator = false;/**< Whether to show a floating value tooltip on drag. */
    float m_tooltipAnim = 0.0f;       /**< Tooltip fade/slide animation progress [0.0, 1.0]. */

    // Interaction animation state
    float m_handleScaleAnim = 0.0f;   /**< Handle scale animation progress (hover/drag expansion). */

    std::function<void(float)> m_onValueChanged; /**< Callback invoked on value change. */

    /**
     * @brief Formats seconds into "MM:SS" string.
     * @param seconds Time in seconds (rounded to nearest integer).
     * @return Formatted time string.
     */
    static std::string formatTime(float seconds);

    /**
     * @brief Snaps a raw value to the nearest discrete step if discrete mode is enabled.
     * @param rawValue Raw normalized value [0.0, 1.0].
     * @return Snapped value clamped to [0.0, 1.0].
     */
    float snapValue(float rawValue) const;

public:
    /**
     * @brief Constructs a MediaSlider with an initial normalized value.
     * @param initialValue Initial progress [0.0, 1.0], clamped.
     */
    explicit MediaSlider(float initialValue = 0.35f);

    /** @brief Returns the current normalized value. */
    float getValue() const;

    /** @brief Sets the value programmatically (snaps if discrete). @return This instance for chaining. */
    MediaSlider* setValue(float value);

    /** @brief Enables or disables user interaction. @return This instance. */
    MediaSlider* setEnabled(bool enabled);

    /** @brief Sets playback state (drives squiggly wave). @return This instance. */
    MediaSlider* setPlaying(bool playing);

    /** @brief Sets the visual style. @return This instance. */
    MediaSlider* setStyle(SliderStyle style);

    /** @brief Enables or disables time label rendering. @return This instance. */
    MediaSlider* setShowTime(bool show);

    /** @brief Sets current and total time for time display. @return This instance. */
    MediaSlider* setTime(float currentSec, float totalSec);

    /**
     * @brief Configures discrete stepping and tick mark display.
     * @param steps Number of steps (e.g., 5 produces 6 tick positions from 0 to 5).
     * @param discrete Whether to snap to nearest step on drag/release.
     * @param showTicks Whether to render tick dots.
     * @param showValueIndicator Whether to show the floating tooltip.
     * @return This instance.
     */
    MediaSlider* setDiscreteConfig(int steps, bool discrete = true, bool showTicks = true, bool showValueIndicator = true);

    /**
     * @brief Registers a callback for value change events.
     * @param listener Callback receiving the new normalized value.
     * @return This instance.
     */
    MediaSlider* setOnValueChanged(std::function<void(float)> listener);

    // --- View overrides ---
    float getPreferredWidth() override;
    float getPreferredHeight() override;
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;

    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
};
