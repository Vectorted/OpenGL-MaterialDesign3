/**
 * @file Checkbox.hpp
 * @brief Boolean toggle component with animated state transitions and label layout.
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
#include <string>
#include <functional>

/**
 * @class Checkbox
 * @brief Material Design 3 discrete check component with smooth color blending and checkmark scaling.
 * 
 * Supports a text label, hover and press states, and a ripple effect. The checked state
 * transitions with a smooth animation driven by a spring-like model.
 * 
 * @note The checkbox interacts with the global slider drag state to avoid stealing events
 *       during slider interaction.
 */
class Checkbox : public View {
public:
    bool checked = false;                                /**< Current checked state (true = checked). */
    std::string label;                                   /**< Text label displayed to the right of the checkbox. */
    float checkAnim = 0.0f;                              /**< Normalized animation progress [0.0, 1.0] from unchecked to checked. */
    std::function<void(bool)> onCheckedChange = nullptr; /**< Callback invoked when the checked state changes. */

    /**
     * @brief Constructs a Checkbox component.
     * @param text Companion label text (may be empty).
     * @param act Initial checked state.
     */
    Checkbox(std::string text = "", bool act = false);

    /**
     * @brief Computes the preferred width based on label length and fixed padding.
     * @return Preferred width in dp.
     */
    float getPreferredWidth() override;

    /**
     * @brief Updates the animation progress and ripple effect.
     * @param dt Delta time in seconds.
     */
    void update(float dt) override;

    /**
     * @brief Renders the checkbox, including background, border, checkmark icon, and label.
     * @param renderer Reference to the shader pipeline.
     * @param theme Current MaterialTheme.
     */
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    /**
     * @brief Handles left mouse button events for toggling and ripple activation.
     * @param button Mouse button code (GLFW_MOUSE_BUTTON_LEFT expected).
     * @param action GLFW_PRESS or GLFW_RELEASE.
     * @param mx Mouse X coordinate in screen space.
     * @param my Mouse Y coordinate in screen space.
     * @return true if the event was consumed.
     */
    bool handleMouseButton(int button, int action, float mx, float my) override;
};
