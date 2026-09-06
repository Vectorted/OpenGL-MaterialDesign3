/**
 * @file NavigationDrawer.hpp
 * @brief Material Design 3 Navigation Drawer component with expandable sections, smooth animations, and kinetic scrolling.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include "../layout/View.hpp"
#include "../shader/Icon.hpp"
#include "../theme/MaterialTheme.hpp"
#include <string>
#include <vector>
#include <functional>

/**
 * @enum DrawerAnimStyle
 * @brief Transition animation styles for the navigation drawer panel.
 */
enum class DrawerAnimStyle {
    Slide,      /**< Standard horizontal slide transition. */
    SharedAxis  /**< Coordinated slide and fade shared-axis transition. */
};

class NavigationDrawer;
class DrawerExpandableGroup;

/**
 * @class DrawerItem
 * @brief Interactive navigation item row inside a navigation drawer.
 */
class DrawerItem : public View {
private:
    /**
     * @brief Display label string for the item.
     */
    std::string m_label;

    /**
     * @brief Icon descriptor rendered on the leading edge.
     */
    Icon m_icon;

    /**
     * @brief Selection state flag of this navigation item.
     */
    bool m_selected = false;

    /**
     * @brief Unique integer identifier for this item.
     */
    int m_id = -1;

    /**
     * @brief Pointer to the parent NavigationDrawer instance.
     */
    NavigationDrawer* m_parent = nullptr;

    /**
     * @brief Nested indentation depth level.
     */
    int m_indentLevel = 0;

public:
    /**
     * @brief Constructs a DrawerItem instance.
     * 
     * @param id Unique item identifier.
     * @param label Display text label.
     * @param iconStr Icon identifier string or asset path.
     * @param parent Pointer to parent drawer instance.
     * @param indentLevel Hierarchy nesting indentation level.
     */
    DrawerItem(int id, const std::string& label, const std::string& iconStr, NavigationDrawer* parent, int indentLevel = 0);

    /**
     * @brief Virtual destructor for DrawerItem.
     */
    virtual ~DrawerItem() override = default;

    /**
     * @brief Gets the unique identifier of the item.
     * 
     * @return Integer item ID.
     */
    int getId() const { return m_id; }

    /**
     * @brief Sets the selected state of the item.
     * 
     * @param selected True to select, false to unselect.
     * @param instant True to skip transition animations.
     */
    void setSelected(bool selected, bool instant = false);

    /**
     * @brief Checks if the item is currently selected.
     * 
     * @return True if selected, false otherwise.
     */
    bool isSelected() const { return m_selected; }
    
    /**
     * @brief Gets the preferred layout height in dp.
     * 
     * @return Preferred height in dp.
     */
    float getPreferredHeight() override { return 48.0f; }

    /**
     * @brief Updates item animation states.
     * 
     * @param dt Delta time in seconds.
     */
    void update(float dt) override;

    /**
     * @brief Renders the item pill background, icon, label, and state layers.
     * 
     * @param renderer Reference to the Material UI shader renderer.
     * @param theme Reference to the active Material theme tokens.
     */
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    /**
     * @brief Handles click events on this drawer item.
     */
    void onClick() override;
};

/**
 * @class DrawerHeader
 * @brief Section category sub-header label within the drawer.
 */
class DrawerHeader : public View {
private:
    /**
     * @brief Header title text.
     */
    std::string m_title;

    /**
     * @brief Pointer to parent NavigationDrawer instance.
     */
    NavigationDrawer* m_parent;

public:
    /**
     * @brief Constructs a DrawerHeader instance.
     * 
     * @param title Header title text.
     * @param parent Pointer to parent drawer.
     */
    explicit DrawerHeader(const std::string& title, NavigationDrawer* parent);

    /**
     * @brief Gets the preferred layout height in dp.
     * 
     * @return Preferred height in dp.
     */
    float getPreferredHeight() override { return 44.0f; }

    /**
     * @brief Renders the header title text.
     * 
     * @param renderer Reference to the Material UI shader renderer.
     * @param theme Reference to the active Material theme tokens.
     */
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
};

/**
 * @class DrawerDivider
 * @brief Thin horizontal line separating logical drawer sections.
 */
class DrawerDivider : public View {
private:
    /**
     * @brief Pointer to parent NavigationDrawer instance.
     */
    NavigationDrawer* m_parent;

public:
    /**
     * @brief Constructs a DrawerDivider instance.
     * 
     * @param parent Pointer to parent drawer.
     */
    explicit DrawerDivider(NavigationDrawer* parent);

    /**
     * @brief Gets the preferred layout height in dp.
     * 
     * @return Preferred height in dp.
     */
    float getPreferredHeight() override { return 17.0f; }

    /**
     * @brief Renders the divider separator line.
     * 
     * @param renderer Reference to the Material UI shader renderer.
     * @param theme Reference to the active Material theme tokens.
     */
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
};

/**
 * @class DrawerExpandableGroup
 * @brief Expandable collapsible group container supporting nested hierarchies.
 */
class DrawerExpandableGroup : public ViewGroup {
private:
    /**
     * @brief Section group title text.
     */
    std::string m_title;

    /**
     * @brief Icon descriptor for the group header.
     */
    Icon m_icon;

    /**
     * @brief Indicates whether a valid icon is configured.
     */
    bool m_hasIcon = false;

    /**
     * @brief Current expanded status of the group.
     */
    bool m_isExpanded = true;
    
    /**
     * @brief Current interpolated expansion progress value in range [0.0, 1.0].
     */
    float m_expandAnim = 1.0f;

    /**
     * @brief Initial expansion progress value when animation starts.
     */
    float m_startAnim = 1.0f;

    /**
     * @brief Linear timing progression of the expand collapse animation.
     */
    float m_animProgress = 1.0f;

    /**
     * @brief Pointer to the parent NavigationDrawer instance.
     */
    NavigationDrawer* m_drawer = nullptr;

    /**
     * @brief Indentation depth level for recursive hierarchy nesting.
     */
    int m_indentLevel = 0;

public:
    /**
     * @brief Constructs a DrawerExpandableGroup instance.
     * 
     * @param title Group title label.
     * @param iconStr Optional icon identifier or asset path.
     * @param defaultExpanded Initial expansion state.
     * @param drawer Pointer to the root NavigationDrawer.
     * @param indentLevel Indentation nesting level.
     */
    DrawerExpandableGroup(const std::string& title, const std::string& iconStr, bool defaultExpanded, NavigationDrawer* drawer, int indentLevel = 0);

    /**
     * @brief Virtual destructor for DrawerExpandableGroup.
     */
    virtual ~DrawerExpandableGroup() override = default;

    /**
     * @brief Adds a child navigation item into this expandable group.
     * 
     * @param id Unique item identifier.
     * @param label Display text label.
     * @param iconStr Icon identifier string or asset path.
     */
    void addItem(int id, const std::string& label, const std::string& iconStr);

    /**
     * @brief Adds a nested expandable child sub-group into this group.
     * 
     * @param title Sub-group title label.
     * @param defaultExpanded Initial expansion state for the sub-group.
     * @param iconStr Optional icon identifier string or asset path.
     * @return Pointer to the newly created DrawerExpandableGroup instance.
     */
    DrawerExpandableGroup* addGroup(const std::string& title, bool defaultExpanded = false, const std::string& iconStr = "");

    /**
     * @brief Sets the expansion state and begins the transition animation.
     * 
     * @param expanded True to expand, false to collapse.
     */
    void setExpanded(bool expanded);

    /**
     * @brief Toggles between expanded and collapsed states.
     */
    void toggle() { setExpanded(!m_isExpanded); }

    /**
     * @brief Checks if the group is currently expanded.
     * 
     * @return True if expanded, false if collapsed.
     */
    bool isExpanded() const { return m_isExpanded; }

    /**
     * @brief Gets current animated expansion factor in range [0.0, 1.0].
     * 
     * @return Interpolated expansion animation value.
     */
    float getExpandAnim() const { return m_expandAnim; }

    /**
     * @brief Computes dynamic preferred layout height based on expansion animation.
     * 
     * @return Preferred height in dp.
     */
    float getPreferredHeight() override;

    /**
     * @brief Updates animation interpolation state.
     * 
     * @param dt Delta time in seconds.
     */
    void update(float dt) override;

    /**
     * @brief Resolves layout bounds for group header and child elements.
     * 
     * @param parentX Parent origin X coordinate in pixels.
     * @param parentY Parent origin Y coordinate in pixels.
     * @param parentW Parent allocated width in pixels.
     * @param parentH Parent allocated height in pixels.
     */
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;

    /**
     * @brief Renders the group header, rotation arrow, and clipped child views.
     * 
     * @param renderer Reference to the Material UI shader renderer.
     * @param theme Reference to the active Material theme tokens.
     */
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    /**
     * @brief Handles mouse movement events for hover interactions.
     * 
     * @param mx Mouse X coordinate.
     * @param my Mouse Y coordinate.
     * @return True if cursor is inside this group bounds.
     */
    bool handleMouseMove(float mx, float my) override;

    /**
     * @brief Handles mouse click actions on header and expanded children.
     * 
     * @param button Mouse button index.
     * @param action Action type.
     * @param mx Mouse X coordinate.
     * @param my Mouse Y coordinate.
     * @return True if the event was consumed.
     */
    bool handleMouseButton(int button, int action, float mx, float my) override;
};

/**
 * @class NavigationDrawer
 * @brief Modal sliding drawer container conforming to Material Design 3 guidelines.
 */
class NavigationDrawer : public ViewGroup {
private:
    /**
     * @brief Drawer open state flag.
     */
    bool m_isOpen = false;

    /**
     * @brief Current slide animation progress in range [0.0, 1.0].
     */
    float m_slideAnim = 0.0f;

    /**
     * @brief Standard baseline panel width in dp.
     */
    float m_panelWidthBase = 360.0f;

    /**
     * @brief Currently selected navigation item identifier.
     */
    int m_selectedIndex = -1;

    /**
     * @brief Callback handler triggered upon selecting a navigation item.
     */
    std::function<void(int)> m_onItemSelectedCallback;

    /**
     * @brief Configured panel sliding animation style.
     */
    DrawerAnimStyle m_animStyle = DrawerAnimStyle::Slide;

    /**
     * @brief Current vertical content scroll position in physical pixels.
     */
    float m_scrollY = 0.0f;

    /**
     * @brief Target smooth vertical content scroll position in physical pixels.
     */
    float m_targetScrollY = 0.0f;

    /**
     * @brief Maximum scrollable vertical offset in physical pixels.
     */
    float m_maxScrollY = 0.0f;

public:
    /**
     * @brief Constructs a NavigationDrawer instance.
     */
    NavigationDrawer();

    /**
     * @brief Virtual destructor for NavigationDrawer.
     */
    virtual ~NavigationDrawer() override = default;

    /**
     * @brief Opens the navigation drawer with smooth slide animation.
     */
    void open();

    /**
     * @brief Closes the navigation drawer with smooth slide animation.
     */
    void close();

    /**
     * @brief Toggles between open and closed states.
     */
    void toggle();

    /**
     * @brief Checks if the drawer is opened.
     * 
     * @return True if open, false otherwise.
     */
    bool isOpen() const { return m_isOpen; }

    /**
     * @brief Gets current slide animation factor in range [0.0, 1.0].
     * 
     * @return Current animation value.
     */
    float getSlideAnim() const { return m_slideAnim; }

    /**
     * @brief Calculates master alpha value depending on animation style.
     * 
     * @return Master opacity factor in range [0.0, 1.0].
     */
    float getMasterAlpha() const;

    /**
     * @brief Sets the panel animation style.
     * 
     * @param style Transition style variant.
     */
    void setAnimationStyle(DrawerAnimStyle style) { m_animStyle = style; }

    /**
     * @brief Gets the current animation style.
     * 
     * @return Transition style variant.
     */
    DrawerAnimStyle getAnimationStyle() const { return m_animStyle; }

    /**
     * @brief Appends a navigation item row at top level.
     * 
     * @param id Unique item identifier.
     * @param label Display text label.
     * @param iconStr Icon resource identifier or asset path.
     */
    void addItem(int id, const std::string& label, const std::string& iconStr);

    /**
     * @brief Appends a category section header.
     * 
     * @param title Header title text.
     */
    void addHeader(const std::string& title);

    /**
     * @brief Appends a visual divider separator line.
     */
    void addDivider();

    /**
     * @brief Appends an expandable collapsible group section.
     * 
     * @param title Group title label.
     * @param defaultExpanded Initial expansion state.
     * @param iconStr Optional icon resource identifier or asset path.
     * @return Pointer to the created DrawerExpandableGroup instance.
     */
    DrawerExpandableGroup* addGroup(const std::string& title, bool defaultExpanded = true, const std::string& iconStr = "");

    /**
     * @brief Sets the selected item ID and updates all child visual states.
     * 
     * @param id Unique item identifier.
     */
    void setSelectedIndex(int id);

    /**
     * @brief Registers the item selection callback listener.
     * 
     * @param callback Callback receiving the selected item ID.
     */
    void setOnItemSelected(std::function<void(int)> callback);

    /**
     * @brief Handles item selection event and automatically closes the drawer.
     * 
     * @param id Selected item identifier.
     */
    void onItemClicked(int id);

    /**
     * @brief Updates slide and scroll animation mechanics.
     * 
     * @param dt Delta time in seconds.
     */
    void update(float dt) override;

    /**
     * @brief Resolves panel translation and children layout metrics.
     * 
     * @param parentX Parent origin X coordinate in pixels.
     * @param parentY Parent origin Y coordinate in pixels.
     * @param parentW Parent allocated width in pixels.
     * @param parentH Parent allocated height in pixels.
     */
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;

    /**
     * @brief Renders the modal background scrim, drawer surface, and clipped contents.
     * 
     * @param renderer Reference to the Material UI shader renderer.
     * @param theme Reference to the active Material theme tokens.
     */
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    /**
     * @brief Handles mouse hover events.
     * 
     * @param mx Mouse X coordinate.
     * @param my Mouse Y coordinate.
     * @return True if event is consumed.
     */
    bool handleMouseMove(float mx, float my) override;

    /**
     * @brief Handles mouse click events on scrim backdrop or content elements.
     * 
     * @param button Mouse button index.
     * @param action Action type.
     * @param mx Mouse X coordinate.
     * @param my Mouse Y coordinate.
     * @return True if event is consumed.
     */
    bool handleMouseButton(int button, int action, float mx, float my) override;

    /**
     * @brief Handles kinetic mouse wheel scrolling.
     * 
     * @param mx Mouse X coordinate.
     * @param my Mouse Y coordinate.
     * @param ox Horizontal scroll offset.
     * @param oy Vertical scroll offset.
     * @return True if scrolling was consumed.
     */
    bool handleScroll(float mx, float my, float ox, float oy) override;
};