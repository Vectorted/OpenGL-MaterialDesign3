/**
 * @file ScrollView.hpp
 * @brief Scrollable container view managing dynamic scissor bounds and buttery-smooth kinetic scrolling.
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
#include <algorithm>
#include <vector>
#include <array>

/**
 * @class ScrollView
 * @brief Container class providing smooth kinetic scrolling capabilities with hardware scissor clipping.
 */
class ScrollView : public ViewGroup {
public:
    float scrollY = 0.0f;          /**< Current smooth interpolated vertical scroll offset in pixels. */
    float targetScrollY = 0.0f;    /**< Target scroll position in pixels. */
    float maxScrollY = 0.0f;       /**< Maximum reachable scroll offset in pixels. */
    float scrollVelocityY = 0.0f;  /**< Kinetic scrolling velocity. */

    /** @brief Constructs a ScrollView container with default bounds and zero padding. */
    ScrollView();

    /** @brief Virtual destructor. */
    virtual ~ScrollView() override = default;

    /**
     * @brief Clears the global OpenGL scissor test clipping stack.
     */
    static void clearScissorStack();

    /**
     * @brief Measures preferred container width based on root scroll child.
     * 
     * @return Ideal preferred bounding width in dp.
     */
    float getPreferredWidth() override;

    /**
     * @brief Measures preferred container height based on root scroll child.
     * 
     * @return Ideal preferred bounding height in dp.
     */
    float getPreferredHeight() override;

    /**
     * @brief Calculates absolute boundaries and updates scroll child constraints.
     * 
     * @param parentX Parent origin X in pixels.
     * @param parentY Parent origin Y in pixels.
     * @param parentW Allocated parent bounding width in pixels.
     * @param parentH Allocated parent bounding height in pixels.
     */
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;

    /**
     * @brief Renders the scroll container content using hardware scissor clipping.
     * 
     * @param renderer Active MaterialShader hardware pipeline instance.
     * @param theme Active MaterialTheme styling palette.
     */
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    /**
     * @brief Updates kinetic scroll physics and exponential easing interpolations.
     * 
     * @param dt Delta time in seconds elapsed since previous frame.
     */
    void update(float dt) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
    bool handleScroll(float mx, float my, float ox, float oy) override;

private:
    /**
     * @brief Synchronizes layout bounds and translation of child components with the current scroll offset.
     */
    void syncChildrenScrollPositions();
};