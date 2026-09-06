/**
 * @file TimePickerDial.hpp
 * @brief Material Design 3 Time Picker component supporting analog dial clock hands and digital input card modes.
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
 * @enum TimePickerMode
 * @brief Active selection mode indicating whether hours or minutes are being adjusted.
 */
enum class TimePickerMode { 
    Hour,   /**< Hours selection mode [1-12]. */
    Minute  /**< Minutes selection mode [0-59]. */
};

/**
 * @enum TimePickerStyle
 * @brief Visual presentation format for the time picker.
 */
enum class TimePickerStyle { 
    Dial,   /**< Analog circular clock dial interface. */
    Input   /**< Dual digital numeric input card blocks. */
};

/**
 * @class TimePickerDial
 * @brief Interactive Material Design 3 time selection control with polar spring physics animations.
 */
class TimePickerDial : public View {
public:
    TimePickerMode mode = TimePickerMode::Hour;    /**< Current active selection target mode. */
    TimePickerStyle style = TimePickerStyle::Dial; /**< Current presentation style. */

    int selectedHour = 10;                         /**< Selected hour value [1, 12]. */
    int selectedMinute = 30;                       /**< Selected minute value [0, 59]. */

    float currentAngle = 0.0f;                     /**< Current hand angle in radians. */
    float targetAngle = 0.0f;                      /**< Target hand angle in radians calculated from current value. */
    float angleVelocity = 0.0f;                    /**< Angular velocity for spring-damper rotation physics. */
    bool isDragging = false;                       /**< Flag indicating if dial hand is actively dragged. */

    /**
     * @brief Callback invoked when a time selection is confirmed upon releasing the mouse button.
     * Passes the selected hour and minute integers as parameters.
     */
    std::function<void(int, int)> onTimeSelected;

    /**
     * @brief Constructs a TimePickerDial instance.
     * 
     * @param w Initial layout width.
     * @param h Initial layout height.
     */
    TimePickerDial(float w, float h);

    /** @brief Virtual destructor. */
    virtual ~TimePickerDial() override = default;

    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;

private:
    float getTargetAngleFromValue();
    int calculateValueFromPosition(float mx, float my);
    bool isInsideIcon(float mx, float my, float ix, float iy, float size);
};