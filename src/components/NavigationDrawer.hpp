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
 * 
 * Displays an icon and label, supports selection state with highlight,
 * hover/press state layers, and optional indentation for nested items.
 */
class DrawerItem : public View {
private:
    std::string m_label;          /**< Display text. */
    Icon m_icon;                  /**< Icon (may be empty). */
    bool m_selected = false;      /**< Whether this item is currently selected. */
    int m_id = -1;                /**< Unique identifier for callback. */
    NavigationDrawer* m_parent = nullptr; /**< Parent drawer (for master alpha). */
    int m_indentLevel = 0;        /**< Indentation depth for nested items. */

public:
    /**
     * @brief Constructs a drawer item.
     * @param id Unique identifier.
     * @param label Display text.
     * @param iconStr Icon identifier.
     * @param parent Parent drawer.
     * @param indentLevel Indentation depth.
     */
    DrawerItem(int id, const std::string& label, const std::string& iconStr, NavigationDrawer* parent, int indentLevel = 0);
    virtual ~DrawerItem() override = default;

    int getId() const { return m_id; }
    void setSelected(bool selected, bool instant = false);
    bool isSelected() const { return m_selected; }
    
    float getPreferredHeight() override { return 48.0f; }
    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
    void onClick() override;
};

/**
 * @class DrawerHeader
 * @brief Section category sub-header label within the drawer.
 * 
 * Renders a non-interactive title with lower opacity.
 */
class DrawerHeader : public View {
private:
    std::string m_title;
    NavigationDrawer* m_parent;

public:
    explicit DrawerHeader(const std::string& title, NavigationDrawer* parent);
    virtual ~DrawerHeader() override = default;

    float getPreferredHeight() override { return 44.0f; }
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
};

/**
 * @class DrawerDivider
 * @brief Thin horizontal line separating logical drawer sections.
 */
class DrawerDivider : public View {
private:
    NavigationDrawer* m_parent;

public:
    explicit DrawerDivider(NavigationDrawer* parent);
    virtual ~DrawerDivider() override = default;

    float getPreferredHeight() override { return 17.0f; }
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
};

/**
 * @class DrawerExpandableGroup
 * @brief Expandable collapsible group container supporting nested hierarchies.
 * 
 * Contains a header with an icon, title, and an arrow that rotates to indicate
 * expansion state. Children can be regular items or nested groups. Implements
 * a viewport-aware staged velocity animation for smooth expansion/collapse.
 */
class DrawerExpandableGroup : public ViewGroup {
private:
    std::string m_title;
    Icon m_icon;
    bool m_hasIcon = false;
    bool m_isExpanded = true;
    
    float m_expandAnim = 1.0f;      /**< Current expansion animation progress [0.0, 1.0]. */
    float m_startAnim = 1.0f;       /**< Starting value for the current animation cycle. */
    float m_animProgress = 1.0f;    /**< Normalized time progress within the animation [0.0, 1.0]. */
    const float ANIM_DURATION = 0.28f; /**< Duration of the expansion animation in seconds. */

    NavigationDrawer* m_drawer = nullptr; /**< Parent drawer (for scissor clipping and master alpha). */
    int m_indentLevel = 0;          /**< Indentation level for the group header. */

public:
    /**
     * @brief Constructs an expandable group.
     * @param title Group header text.
     * @param iconStr Icon for the header (optional).
     * @param defaultExpanded Initial expansion state.
     * @param drawer Parent drawer.
     * @param indentLevel Indentation depth.
     */
    DrawerExpandableGroup(const std::string& title, const std::string& iconStr, bool defaultExpanded, NavigationDrawer* drawer, int indentLevel = 0);
    virtual ~DrawerExpandableGroup() override = default;

    /** @brief Adds a child item with a unique ID. */
    void addItem(int id, const std::string& label, const std::string& iconStr);

    /** @brief Adds a nested expandable group. @return Pointer to the new group. */
    DrawerExpandableGroup* addGroup(const std::string& title, bool defaultExpanded = false, const std::string& iconStr = "");

    /** @brief Sets expansion state with animation. */
    void setExpanded(bool expanded);

    /** @brief Toggles expansion state. */
    void toggle() { setExpanded(!m_isExpanded); }

    /** @brief Returns current expansion state. */
    bool isExpanded() const { return m_isExpanded; }

    /** @brief Returns current animation progress [0.0, 1.0]. */
    float getExpandAnim() const { return m_expandAnim; }

    float getPreferredHeight() override;
    void update(float dt) override;
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
};

/**
 * @class NavigationDrawer
 * @brief Modal sliding drawer container conforming to Material Design 3 guidelines.
 * 
 * Manages a panel that slides in from the left (or right) edge. Supports
 * scrollable content, kinetic scrolling, item selection, and expandable
 * sections. Implements a scrim overlay and exclusive modal behavior.
 */
class NavigationDrawer : public ViewGroup {
private:
    bool m_isOpen = false;                      /**< Whether the drawer is open (target state). */
    float m_slideAnim = 0.0f;                   /**< Slide animation progress [0.0, 1.0]. */
    float m_panelWidthBase = 360.0f;            /**< Base panel width in dp (clamped to screen). */
    int m_selectedIndex = -1;                   /**< ID of the currently selected item. */
    std::function<void(int)> m_onItemSelectedCallback; /**< Selection callback. */
    DrawerAnimStyle m_animStyle = DrawerAnimStyle::Slide; /**< Animation style. */

    float m_scrollY = 0.0f;                     /**< Current scroll offset (pixels). */
    float m_targetScrollY = 0.0f;               /**< Target scroll offset (smoothly interpolated). */
    float m_maxScrollY = 0.0f;                  /**< Maximum allowable scroll offset. */

public:
    NavigationDrawer();
    virtual ~NavigationDrawer() override = default;

    /** @brief Opens the drawer with animation. */
    void open();

    /** @brief Closes the drawer with animation. */
    void close();

    /** @brief Toggles the drawer open/close state. */
    void toggle();

    /** @brief Returns current open/close target state. */
    bool isOpen() const { return m_isOpen; }

    /** @brief Returns current slide animation progress [0.0, 1.0]. */
    float getSlideAnim() const { return m_slideAnim; }

    /**
     * @brief Returns the master alpha for child elements (used in SharedAxis style).
     * @return Alpha value [0.0, 1.0].
     */
    float getMasterAlpha() const;

    /** @brief Sets the animation style. */
    void setAnimationStyle(DrawerAnimStyle style) { m_animStyle = style; }

    /** @brief Returns the current animation style. */
    DrawerAnimStyle getAnimationStyle() const { return m_animStyle; }

    /** @brief Adds a top-level drawer item. */
    void addItem(int id, const std::string& label, const std::string& iconStr);

    /** @brief Adds a non-interactive header label. */
    void addHeader(const std::string& title);

    /** @brief Adds a divider line. */
    void addDivider();

    /**
     * @brief Adds an expandable group at the top level.
     * @param title Group title.
     * @param defaultExpanded Initial expansion state.
     * @param iconStr Optional header icon.
     * @return Pointer to the created group.
     */
    DrawerExpandableGroup* addGroup(const std::string& title, bool defaultExpanded = true, const std::string& iconStr = "");

    /** @brief Sets the selected item by ID (updates UI and callback). */
    void setSelectedIndex(int id);

    /** @brief Registers a selection callback. */
    void setOnItemSelected(std::function<void(int)> callback);

    /** @brief Internal handler called when an item is clicked. */
    void onItemClicked(int id);

    void update(float dt) override;
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
    bool handleScroll(float mx, float my, float ox, float oy) override;
};
