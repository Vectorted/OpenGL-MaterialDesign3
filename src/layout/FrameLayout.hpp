/**
 * @file FrameLayout.hpp
 * @brief Stacking container layout positioning children in layers with individual gravity alignment.
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
 * @class FrameLayout
 * @brief Layout container that stacks child views on top of each other along the Z-axis.
 * 
 * Each child can declare its own `layout_gravity` to align itself to any corner, edge,
 * or center point inside the parent's padding boundary.
 */
class FrameLayout : public ViewGroup {
public:
    /**
     * @brief Constructs a default FrameLayout instance.
     */
    FrameLayout();

    /**
     * @brief Virtual destructor.
     */
    virtual ~FrameLayout() override = default;

    /**
     * @brief Measures aggregate preferred width by finding the maximum child width plus margins.
     * 
     * @return Ideal preferred bounding width in dp.
     */
    float getPreferredWidth() override;

    /**
     * @brief Measures aggregate preferred height by finding the maximum child height plus margins.
     * 
     * @return Ideal preferred bounding height in dp.
     */
    float getPreferredHeight() override;

    /**
     * @brief Dispatches layout pass, positioning each child view according to its layout_gravity.
     * 
     * @param parentX Parent origin X in pixels.
     * @param parentY Parent origin Y in pixels.
     * @param parentW Allocated parent bounding width in pixels.
     * @param parentH Allocated parent bounding height in pixels.
     */
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
};