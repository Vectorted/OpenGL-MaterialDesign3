/**
 * @file DatePicker.hpp
 * @brief Material Design 3 calendar date picker component with month navigation and day selection.
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
 * @class DatePicker
 * @brief Interactive modal-style calendar grid component conforming to Material Design 3.
 */
class DatePicker : public View {
public:
    int selYear = 2026;  /**< Currently selected year. */
    int selMonth = 7;    /**< Currently selected month (1-12). */
    int selDay = 18;     /**< Currently selected day of month (1-31). */
    int viewYear = 2026; /**< Year currently being viewed in calendar grid. */
    int viewMonth = 7;   /**< Month currently being viewed in calendar grid (1-12). */
    int hoverDay = -1;   /**< Day of month currently hovered by cursor, or -1. */

    std::function<void(int year, int month, int day)> onDateSelected; /**< Callback dispatched on date selection. */

    /** @brief Constructs a DatePicker with default dimensions. */
    DatePicker();

    void render(MaterialShader& renderer, MaterialTheme& theme) override;
    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;

private:
    /**
     * @brief Computes total number of days in a given month of a specific year.
     */
    int getDaysInMonth(int y, int m);
    
    /**
     * @brief Computes weekday offset (0 = Sunday, 1 = Monday, etc.) for the first day of the month.
     */
    int getStartWeekday(int y, int m); 
    
    /**
     * @brief Point-in-rectangle collision helper.
     */
    bool isInsideRect(float mx, float my, float rx, float ry, float rw, float rh);

    float prevBtnX = 0.0f; /**< Previous month button X coordinate. */
    float prevBtnY = 0.0f; /**< Previous month button Y coordinate. */
    float nextBtnX = 0.0f; /**< Next month button X coordinate. */
    float nextBtnY = 0.0f; /**< Next month button Y coordinate. */
    float btnSize = 0.0f;  /**< Month navigation button bounding dimension. */
    float gridStartX = 0.0f;/**< Day cells grid start X coordinate. */
    float gridStartY = 0.0f;/**< Day cells grid start Y coordinate. */
    float cellSize = 0.0f;  /**< Width and height of an individual day cell. */
};