/**
 * @file MaterialSegmentedButton.hpp
 * @brief Material Design 3 Segmented Button component supporting single and multi-selection with spring kinetics.
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
#include <vector>
#include <functional>

/**
 * @class MaterialSegmentedButton
 * @brief Segmented button group allowing users to select single or multiple options from a linear array.
 * 
 * Features include:
 * - Single or multi-selection modes
 * - Smooth sliding selection indicator with Material 3 easing curves
 * - Hover and press state feedback
 * - Optional checkmark icon on selected segments
 * - Spring-like animation for selection transitions
 * - Fully customizable via theme
 */
class MaterialSegmentedButton : public View {
private:
    /**
     * @struct Segment
     * @brief Runtime state container for an individual segmented button option.
     * 
     * Stores both visual animation progress values and the display label.
     */
    struct Segment {
        std::string label;       /**< Display text label for this segment. */
        float hoverAnim = 0.0f;  /**< Normalized hover transition progress [0.0, 1.0]. */
        float pressAnim = 0.0f;  /**< Normalized press transition progress [0.0, 1.0]. */
        float selectAnim = 0.0f; /**< Selection animation progress [0.0, 1.0], driven by a critical-damped spring model. */
        float selectVel = 0.0f;  /**< Velocity parameter for spring-based selection animation (inertia component). */
    };

    std::vector<Segment> m_segments;       /**< Array of segment state containers. */
    bool m_isMultiSelect = false;          /**< True for multi-selection mode; false for single-selection. */
    bool m_hasCheckmark = true;            /**< Whether to render a checkmark icon on selected segments. */

    int m_selectedIndex = 0;               /**< Currently selected index (single-select mode only). */
    std::vector<bool> m_selectedStates;    /**< Selection flags for each segment (multi-select mode only). */

    int m_hoveredIndex = -1;               /**< Index of the segment currently under the mouse cursor (-1 if none). */
    int m_pressedIndex = -1;               /**< Index of the segment currently being pressed (-1 if none). */

    std::function<void(int)> m_onSelectionChanged;                       /**< Callback for single-select changes. */
    std::function<void(const std::vector<bool>&)> m_onMultiSelectionChanged; /**< Callback for multi-select changes. */

public:
    /**
     * @brief Constructs a single-selection Segmented Button group.
     * @param items List of segment text labels.
     * @param defaultSelected Zero-based default selected index (clamped to valid range).
     */
    MaterialSegmentedButton(const std::vector<std::string>& items, int defaultSelected = 0);

    /**
     * @brief Constructs a multi-selection Segmented Button group.
     * @param items List of segment text labels.
     * @param defaultStates Boolean selection flags for each segment (resized to match items if needed).
     */
    MaterialSegmentedButton(const std::vector<std::string>& items, const std::vector<bool>& defaultStates);

    /** @brief Virtual destructor. */
    virtual ~MaterialSegmentedButton() = default;

    /**
     * @brief Registers a callback for single-selection changes.
     * @param callback Function receiving the newly selected index.
     */
    void setOnSelectionChanged(std::function<void(int)> callback) { m_onSelectionChanged = std::move(callback); }

    /**
     * @brief Registers a callback for multi-selection changes.
     * @param callback Function receiving the complete selection state vector.
     */
    void setOnMultiSelectionChanged(std::function<void(const std::vector<bool>&)> callback) { m_onMultiSelectionChanged = std::move(callback); }

    /**
     * @brief Retrieves the currently selected segment index in single-select mode.
     * @return Zero-based index of the selected segment.
     */
    int getSelectedIndex() const { return m_selectedIndex; }

    /**
     * @brief Sets the selected segment index in single-select mode.
     * @param index Target index (clamped to valid range). Triggers animation and callback.
     */
    void setSelectedIndex(int index);

    /**
     * @brief Retrieves boolean selection states for all segments in multi-select mode.
     * @return Vector of bool flags representing the selection state of each segment.
     */
    std::vector<bool> getSelectedStates() const { return m_selectedStates; }

    /**
     * @brief Toggles the selection state of a specific segment in multi-select mode.
     * @param index Target segment index. Triggers animation and callback.
     */
    void toggleMultiSelectIndex(int index);

    /**
     * @brief Enables or disables rendering of the vector checkmark icon on selected segments.
     * @param hasCheck True to show checkmark, false to hide it.
     */
    void setHasCheckmark(bool hasCheck) { m_hasCheckmark = hasCheck; }

    /**
     * @brief Checks whether the vector checkmark icon is enabled.
     * @return True if checkmark rendering is active.
     */
    bool getHasCheckmark() const { return m_hasCheckmark; }

    // --- View overrides ---
    float getPreferredWidth() override;
    float getPreferredHeight() override;

    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;

    /**
     * @brief Resets hover and press tracking indices when the cursor leaves the view bounds.
     */
    void onMouseLeave();
};
