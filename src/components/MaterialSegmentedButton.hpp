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
 */
class MaterialSegmentedButton : public View {
private:
    /**
     * @struct Segment
     * @brief Runtime state container for an individual segmented button option.
     */
    struct Segment {
        std::string label;       /**< Display text label for this segment. */
        float hoverAnim = 0.0f;  /**< Normalized hover transition progress [0.0, 1.0]. */
        float pressAnim = 0.0f;  /**< Normalized press transition progress [0.0, 1.0]. */
        float selectAnim = 0.0f; /**< Physical spring-damper selection animation progress. */
        float selectVel = 0.0f;  /**< Velocity parameter for spring-based selection animation. */
    };

    std::vector<Segment> m_segments;
    bool m_isMultiSelect = false;
    bool m_hasCheckmark = true;

    int m_selectedIndex = 0;
    std::vector<bool> m_selectedStates;

    int m_hoveredIndex = -1;
    int m_pressedIndex = -1;

    std::function<void(int)> m_onSelectionChanged;
    std::function<void(const std::vector<bool>&)> m_onMultiSelectionChanged;

public:
    /**
     * @brief Constructs a single-selection Segmented Button group.
     * 
     * @param items List of segment text labels.
     * @param defaultSelected Zero-based default selected index.
     */
    MaterialSegmentedButton(const std::vector<std::string>& items, int defaultSelected = 0);

    /**
     * @brief Constructs a multi-selection Segmented Button group.
     * 
     * @param items List of segment text labels.
     * @param defaultStates Boolean selection flags for each segment.
     */
    MaterialSegmentedButton(const std::vector<std::string>& items, const std::vector<bool>& defaultStates);

    /** @brief Virtual destructor. */
    virtual ~MaterialSegmentedButton() = default;

    /** @brief Sets callback listener for single-selection changes. */
    void setOnSelectionChanged(std::function<void(int)> callback) { m_onSelectionChanged = std::move(callback); }

    /** @brief Sets callback listener for multi-selection changes. */
    void setOnMultiSelectionChanged(std::function<void(const std::vector<bool>&)> callback) { m_onMultiSelectionChanged = std::move(callback); }

    /** @brief Retrieves the currently selected segment index in single-select mode. */
    int getSelectedIndex() const { return m_selectedIndex; }

    /** @brief Sets the selected segment index in single-select mode. */
    void setSelectedIndex(int index);

    /** @brief Retrieves boolean selection states for all segments in multi-select mode. */
    std::vector<bool> getSelectedStates() const { return m_selectedStates; }

    /** @brief Toggles the selection state of a specific segment in multi-select mode. */
    void toggleMultiSelectIndex(int index);

    /** @brief Enables or disables rendering of the vector checkmark icon on selected segments. */
    void setHasCheckmark(bool hasCheck) { m_hasCheckmark = hasCheck; }

    /** @brief Checks whether the vector checkmark icon is enabled. */
    bool getHasCheckmark() const { return m_hasCheckmark; }

    float getPreferredWidth() override;
    float getPreferredHeight() override;

    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;

    /** @brief Resets hover and press tracking indices when cursor leaves view bounds. */
    void onMouseLeave();
};