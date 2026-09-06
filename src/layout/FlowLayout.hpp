/**
 * @file FlowLayout.hpp
 * @brief Auto-wrapping flex row container for adaptive tags, chip groups, and dynamic badges.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include "View.hpp"

/**
 * @class FlowLayout
 * @brief Arranges child components horizontally and automatically wraps them to the next row when width limit is reached.
 */
class FlowLayout : public ViewGroup {
public:
    float horizontalGapDp = 8.0f;  /**< Horizontal spacing between adjacent child views in dp. */
    float verticalGapDp = 8.0f;    /**< Vertical spacing between consecutive wrapped rows in dp. */
    int gravity = Gravity::LEFT;   /**< Row alignment gravity (Left, CenterHorizontal, Right). */

    /**
     * @brief Constructs a FlowLayout with configurable spacing.
     * 
     * @param hGap Horizontal gap in dp.
     * @param vGap Vertical gap in dp.
     */
    FlowLayout(float hGap = 8.0f, float vGap = 8.0f);

    /**
     * @brief Virtual destructor.
     */
    virtual ~FlowLayout() override = default;

    /**
     * @brief Configures inter-item spatial gaps.
     * 
     * @param hGap Horizontal gap in dp.
     * @param vGap Vertical gap in dp.
     */
    void setGaps(float hGap, float vGap) {
        horizontalGapDp = hGap;
        verticalGapDp = vGap;
    }

    /**
     * @brief Sets row alignment gravity.
     * 
     * @param grav Bitwise gravity constant (e.g. Gravity::CENTER_HORIZONTAL).
     */
    void setGravity(int grav) { gravity = grav; }

    /**
     * @brief Computes preferred bounding width across unbroken rows.
     * 
     * @return Ideal preferred bounding width in dp.
     */
    float getPreferredWidth() override;

    /**
     * @brief Computes preferred bounding height accounting for multi-row wrap breaks.
     * 
     * @return Ideal preferred bounding height in dp.
     */
    float getPreferredHeight() override;

    /**
     * @brief Dispatches auto-wrapping multi-line layout flow.
     * 
     * @param parentX Parent origin X in pixels.
     * @param parentY Parent origin Y in pixels.
     * @param parentW Allocated parent bounding width in pixels.
     * @param parentH Allocated parent bounding height in pixels.
     */
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
};