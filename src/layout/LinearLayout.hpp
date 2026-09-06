/**
 * @file LinearLayout.hpp
 * @brief Linear flex box layout container for automatic component positioning along horizontal and vertical axes.
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
 * @class LinearLayout
 * @brief Layout container aligning child view nodes along a single horizontal or vertical axis with auto-scaling and gravity alignment.
 */
class LinearLayout : public ViewGroup {
public:
    /**
     * @enum Orientation
     * @brief Layout directional axis orientation.
     */
    enum class Orientation { 
        HORIZONTAL, /**< Sequential horizontal flow from left to right. */
        VERTICAL    /**< Sequential vertical stack from top to bottom. */
    };

    Orientation orientation = Orientation::VERTICAL; /**< Primary stacking orientation. */
    float gapDp = 16.0f;                              /**< Inter-item spatial gap between adjacent children in dp. */
    int gravity = Gravity::NONE;                      /**< Overall container content alignment gravity. */

    /**
     * @brief Constructs a LinearLayout container.
     * 
     * @param orient Stacking orientation (defaults to Orientation::VERTICAL).
     */
    LinearLayout(Orientation orient = Orientation::VERTICAL);

    /** @brief Virtual destructor. */
    virtual ~LinearLayout() override = default;

    /**
     * @brief Measures aggregate preferred width across all managed child views.
     * 
     * @return Ideal preferred bounding width in dp.
     */
    float getPreferredWidth() override;

    /**
     * @brief Measures aggregate preferred height across all managed child views.
     * 
     * @return Ideal preferred bounding height in dp.
     */
    float getPreferredHeight() override;

    /**
     * @brief Executes two-pass layout calculation for linear arrangement, flex expansion, and axis gravity offsets.
     * 
     * @param parentX Parent origin X in pixels.
     * @param parentY Parent origin Y in pixels.
     * @param parentW Parent bounding width in pixels.
     * @param parentH Parent bounding height in pixels.
     */
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;

    /**
     * @brief Configures layout stacking axis.
     * 
     * @param orient Target layout orientation (HORIZONTAL or VERTICAL).
     */
    void setOrientation(Orientation orient) { orientation = orient; }

    /**
     * @brief Configures spatial gap between consecutive children.
     * 
     * @param gap Spatial gap size in dp.
     */
    void setGap(float gap) { gapDp = gap; }

    /**
     * @brief Configures global layout gravity alignment for child components.
     * 
     * @param grav Bitwise combination of Gravity constants.
     */
    void setGravity(int grav) { gravity = grav; }

    /**
     * @brief Retrieves current container layout gravity.
     * 
     * @return Integer bitmask representing Gravity flags.
     */
    int getGravity() const { return gravity; }
};