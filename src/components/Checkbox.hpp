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
 */
class Checkbox : public View {
public:
    bool checked = false;                                /**< State indicating whether checkbox is active/checked. */
    std::string label;                                   /**< Text label rendered alongside the checkbox box. */
    float checkAnim = 0.0f;                              /**< Normalized transition progress [0.0, 1.0] between unchecked and checked. */
    std::function<void(bool)> onCheckedChange = nullptr; /**< Callback dispatched on toggle state change. */

    /**
     * @brief Constructs a Checkbox component.
     * 
     * @param text Companion label text.
     * @param act Initial checked state.
     */
    Checkbox(std::string text = "", bool act = false);

    float getPreferredWidth() override;
    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
};