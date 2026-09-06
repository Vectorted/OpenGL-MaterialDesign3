/**
 * @file NavigationBar.hpp
 * @brief Material Design 3 Navigation Bar and Navigation Tab components.
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
#include "../shader/Icon.hpp"
#include "../theme/MaterialTheme.hpp"
#include <vector>
#include <string>
#include <functional>

/**
 * @enum NavigationIcon
 * @brief Legacy fallback enumeration for navigational icons.
 */
enum class NavigationIcon { Home, Grid, Message, Person };

class NavigationBar;

/**
 * @typedef NavigationIconRenderer
 * @brief Custom callback for programmatic rendering of custom navigation tab icons.
 * 
 * @param renderer MaterialShader renderer instance.
 * @param theme Active MaterialTheme instance.
 * @param centerX Horizontal center of the icon container.
 * @param centerY Vertical center of the icon container.
 * @param iconSize Bounding size allocated for the icon.
 * @param iconColor Resolved tint color.
 * @param selected Boolean indicating if the parent tab is currently selected.
 */
typedef std::function<void(
    MaterialShader& renderer,
    MaterialTheme& theme,
    float centerX,
    float centerY,
    float iconSize,
    M3Color iconColor,
    bool selected
)> NavigationIconRenderer;

/**
 * @class NavigationTab
 * @brief Individual destination tab item within a NavigationBar.
 */
class NavigationTab : public View {
private:
    std::string m_label;
    Icon m_icon;
    NavigationIcon m_legacyIcon = NavigationIcon::Home;
    bool m_useLegacyIcon = false;
    bool m_selected = false;
    float m_indicatorAnim = 0.0f;
    NavigationBar* m_parent = nullptr;
    NavigationIconRenderer m_customIconRenderer = nullptr;

    void renderIcon(MaterialShader& renderer, MaterialTheme& theme, float centerX, float centerY);

public:
    /** @brief Constructs a NavigationTab using an Icon variant. */
    NavigationTab(const std::string& label, const Icon& icon, NavigationBar* parent);

    /** @brief Constructs a NavigationTab using a legacy icon enumeration. */
    NavigationTab(const std::string& label, NavigationIcon icon, NavigationBar* parent);

    /** @brief Constructs a NavigationTab using a custom drawing callback. */
    NavigationTab(const std::string& label, NavigationIconRenderer customIconRenderer, NavigationBar* parent);

    /** @brief Virtual destructor. */
    virtual ~NavigationTab() override = default;

    /**
     * @brief Modifies the tab selection state.
     * 
     * @param selected True to activate tab.
     * @param instant If true, skips smooth pill animation.
     */
    void setSelected(bool selected, bool instant = false);
    
    /** @brief Checks if tab is currently selected. */
    bool isSelected() const { return m_selected; }

    float getPreferredWidth() override;
    float getPreferredHeight() override;

    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
    void onClick() override;
};

/**
 * @enum NavigationBarPlacement
 * @brief Placement mode for the NavigationBar container.
 */
enum class NavigationBarPlacement { 
    Bottom, /**< Docked across the bottom of the viewport. */
    Custom  /**< Positioned within explicit custom coordinate bounds. */
};

/**
 * @class NavigationBar
 * @brief Bottom navigation bar component managing destination tabs and active indicator animations.
 */
class NavigationBar : public ViewGroup {
private:
    int m_selectedIndex = -1;
    std::function<void(int)> m_onTabSelectedCallback;

    NavigationBarPlacement m_placement = NavigationBarPlacement::Bottom;
    float m_insetLeft = 0.0f;
    float m_insetRight = 0.0f;
    float m_insetBottom = 0.0f;
    float m_regionX = 0.0f;
    float m_regionY = 0.0f;
    float m_regionWidth = 0.0f;
    float m_regionHeight = 0.0f;
    float m_cornerRadius = 0.0f;
    bool m_drawBackground = true;

    bool m_showTopDivider = false;
    float m_dividerThickness = 1.0f;
    float m_dividerHorizontalInset = 0.0f;
    float m_dividerAlpha = 0.2f;

public:
    /** @brief Constructs a default NavigationBar. */
    NavigationBar();

    /** @brief Virtual destructor. */
    virtual ~NavigationBar() override = default;

    /** @brief Appends an Icon-based tab to the navigation bar. */
    void addTab(const std::string& label, const Icon& icon);
    
    /** @brief Appends a tab using an icon string token/path. */
    void addTab(const std::string& label, const std::string& iconStr) {
        addTab(label, iconFromString(iconStr));
    }
    
    /** @brief Appends a legacy icon tab to the navigation bar. */
    void addTab(const std::string& label, NavigationIcon icon);
    
    /** @brief Appends a custom-drawn icon tab using a renderer callback. */
    void addTab(const std::string& label, NavigationIconRenderer customIconRenderer);

    /** @brief Sets the active tab by zero-based index. */
    void setSelectedIndex(int index);
    
    /** @brief Retrieves the active tab index, or -1 if no tabs exist. */
    int getSelectedIndex() const { return m_selectedIndex; }

    /** @brief Sets callback listener for tab selection changes. */
    void setOnTabSelected(std::function<void(int)> callback);
    
    /** @brief Dispatches tab click events from child tabs to the parent bar. */
    void onTabClicked(NavigationTab* tab);

    /** @brief Configures standard bottom docking insets. */
    void setBottomPlacement(float leftInset, float rightInset, float bottomInset);
    
    /** @brief Configures explicit custom bounding box coordinates. */
    void setRegion(float regionX, float regionY, float regionWidth, float regionHeight);
    
    /** @brief Configures background corner rounding radius for floating navbar designs. */
    void setCornerRadius(float radius);
    
    /** @brief Enables or disables background rendering. */
    void setDrawBackground(bool enabled);
    
    /** @brief Configures top structural divider stroke parameters. */
    void setTopDivider(bool enabled, float thickness = 1.0f, float horizontalInset = 0.0f, float alpha = 0.2f);

    float getPreferredWidth() override;
    float getPreferredHeight() override;

    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
};