/**
 * @file MaterialPrimaryTab.hpp
 * @brief Material Design 3 Primary Tab Bar component with animated active indicator underline.
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
#include "../theme/MaterialTheme.hpp"
#include "../shader/MaterialShader.hpp"
#include <vector>
#include <string>
#include <functional>

/**
 * @struct MaterialTabItem
 * @brief Descriptor representing a single tab item in the tab row.
 */
struct MaterialTabItem {
    std::string text;         /**< Tab label text. */
    Icon icon;                /**< Tab icon descriptor. */
    float boundsX = 0.0f;     /**< Calculated start horizontal position in pixels. */
    float boundsWidth = 0.0f; /**< Calculated bounding width in pixels. */
};

/**
 * @class MaterialTabRow
 * @brief Horizontal tab bar component featuring smooth sliding active indicator transitions.
 */
class MaterialTabRow : public View {
public:
    /** @brief Constructs a default MaterialTabRow. */
    MaterialTabRow();

    /** @brief Virtual destructor. */
    virtual ~MaterialTabRow() override = default;

    /**
     * @brief Appends a tab item to the row.
     * 
     * @param label Tab text label.
     * @param icon Optional tab icon.
     */
    void addTab(const std::string& label, const Icon& icon = IconNone());

    /**
     * @brief Sets the currently selected tab index.
     * 
     * @param index Target zero-based tab index.
     * @param animate If true, animates active indicator slide transition.
     */
    void setSelectedTab(size_t index, bool animate = true);

    /** @brief Retrieves the currently selected tab index. */
    size_t getSelectedIndex() const { return m_selectedIndex; }

    /**
     * @brief Sets callback listener for tab selection changes.
     * 
     * @param listener Callback receiving the newly selected tab index.
     */
    void setOnTabSelectedListener(std::function<void(size_t)> listener) {
        m_onTabSelected = listener;
    }

    virtual float getPreferredHeight() override;
    virtual void doLayout(float parentX, float parentY, float parentW, float parentH) override;
    virtual void update(float dt) override;
    virtual void render(MaterialShader& renderer, MaterialTheme& theme) override;

    virtual bool handleMouseMove(float mx, float my) override;
    virtual bool handleMouseButton(int button, int action, float mx, float my) override;

private:
    std::vector<MaterialTabItem> m_tabs;
    size_t m_selectedIndex = 0;
    std::function<void(size_t)> m_onTabSelected;

    float m_indicatorX = 0.0f;
    float m_indicatorWidth = 0.0f;
    float m_targetIndicatorX = 0.0f;
    float m_targetIndicatorW = 0.0f;

    float m_indicatorHeightDp = 3.0f;
    float m_indicatorRadiusDp = 3.0f;
};