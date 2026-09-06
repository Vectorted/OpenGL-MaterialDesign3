/**
 * @file RadioButton.hpp
 * @brief Material Design 3 Radio Button component with animated selection dot and halo effects.
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
 * @class RadioButton
 * @brief Interactive single-selection radio button control.
 */
class RadioButton : public View {
public:
    bool checked = false;                            /**< Current selection state. */
    std::string label;                               /**< Text label displayed alongside the radio button. */
    float checkAnim = 0.0f;                          /**< Normalized animation progress for the inner dot [0.0, 1.0]. */
    
    std::function<void(bool)> onCheckedChange;       /**< Callback invoked when the selection state becomes checked. */

    /**
     * @brief Constructs a RadioButton component.
     * 
     * @param text Companion label string.
     * @param act Initial selection state.
     */
    RadioButton(std::string text = "", bool act = false);

    float getPreferredWidth() override;
    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
};