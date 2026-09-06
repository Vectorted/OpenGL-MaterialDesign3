/**
 * @file RelativeLayout.hpp
 * @brief Rule-based relative layout positioning views in relation to each other or parent bounds.
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
#include <unordered_map>

/**
 * @enum RelativeRule
 * @brief Positional relationship verbs for RelativeLayout constraints.
 */
enum class RelativeRule {
    AlignParentLeft,        /**< Align view's left edge with parent's left inner edge. */
    AlignParentTop,         /**< Align view's top edge with parent's top inner edge. */
    AlignParentRight,       /**< Align view's right edge with parent's right inner edge. */
    AlignParentBottom,      /**< Align view's bottom edge with parent's bottom inner edge. */
    CenterInParent,         /**< Center view both horizontally and vertically inside parent. */
    CenterHorizontal,       /**< Center view horizontally inside parent. */
    CenterVertical,         /**< Center view vertically inside parent. */
    LeftOf,                 /**< Position view's right edge immediately to the left of anchor. */
    RightOf,                /**< Position view's left edge immediately to the right of anchor. */
    Above,                  /**< Position view's bottom edge immediately above anchor. */
    Below,                  /**< Position view's top edge immediately below anchor. */
    AlignLeft,              /**< Align view's left edge with anchor's left edge. */
    AlignTop,               /**< Align view's top edge with anchor's top edge. */
    AlignRight,             /**< Align view's right edge with anchor's right edge. */
    AlignBottom             /**< Align view's bottom edge with anchor's bottom edge. */
};

/**
 * @class RelativeLayout
 * @brief Positions child components in relation to siblings or parent boundaries using declarative rules.
 */
class RelativeLayout : public ViewGroup {
public:
    /**
     * @struct LayoutParams
     * @brief Relationship rules and anchor bindings assigned to a child View.
     */
    struct LayoutParams {
        std::unordered_map<RelativeRule, View*> rules; /**< Set of applied constraint rules with optional sibling targets. */
    };

private:
    std::unordered_map<View*, LayoutParams> m_childParams;

public:
    /**
     * @brief Constructs a default RelativeLayout container.
     */
    RelativeLayout();

    /**
     * @brief Virtual destructor.
     */
    virtual ~RelativeLayout() override = default;

    /**
     * @brief Binds a parent-relative rule to a child view (e.g. CenterInParent, AlignParentRight).
     * 
     * @param child Pointer to child View inside this layout.
     * @param rule Rule verb that does not require an anchor sibling.
     * @return RelativeLayout* Pointer to this layout for fluent configuration.
     */
    RelativeLayout* addRule(View* child, RelativeRule rule);

    /**
     * @brief Binds a sibling-relative rule to a child view (e.g. Below, RightOf).
     * 
     * @param child Pointer to child View being constrained.
     * @param rule Sibling-based rule verb.
     * @param anchor Pointer to reference sibling View.
     * @return RelativeLayout* Pointer to this layout for fluent configuration.
     */
    RelativeLayout* addRule(View* child, RelativeRule rule, View* anchor);

    /**
     * @brief Measures aggregate preferred width across all relatively positioned children.
     * 
     * @return Ideal preferred bounding width in dp.
     */
    float getPreferredWidth() override;

    /**
     * @brief Measures aggregate preferred height across all relatively positioned children.
     * 
     * @return Ideal preferred bounding height in dp.
     */
    float getPreferredHeight() override;

    /**
     * @brief Resolves dependency graphs and computes bounds for each constrained child.
     * 
     * @param parentX Parent origin X in pixels.
     * @param parentY Parent origin Y in pixels.
     * @param parentW Allocated parent bounding width in pixels.
     * @param parentH Allocated parent bounding height in pixels.
     */
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
};