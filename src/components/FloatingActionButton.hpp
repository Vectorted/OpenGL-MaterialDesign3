/**
 * @file FloatingActionButton.hpp
 * @brief Material Design 3 Floating Action Button (FAB) and expandable Speed Dial menu implementation.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <functional>
#include "../layout/View.hpp"
#include "../shader/MaterialShader.hpp"
#include "../shader/Icon.hpp"
#include "../theme/MaterialTheme.hpp"

/**
 * @enum FabSize
 * @brief Sizing options for the Floating Action Button.
 */
enum class FabSize { 
    Standard, /**< Default standard FAB dimension (56dp). */
    Small,    /**< Compact FAB dimension (40dp). */
    Large,    /**< Prominent large FAB dimension (96dp). */
    Extended  /**< Extended FAB containing an icon and an adjacent text label. */
};

/**
 * @enum FabType
 * @brief Color role variants for the Floating Action Button.
 */
enum class FabType { 
    Surface,  /**< Surface styled container with primary colored content. */
    Primary,  /**< Primary container styling with on-primary colored content. */
    Secondary /**< Secondary container styling with on-secondary colored content. */
};

/**
 * @enum FabAlignment
 * @brief Placement anchoring mode for the FAB within its parent viewport.
 */
enum class FabAlignment {
    Free,       /**< Standard coordinate layout mode. */
    BottomRight /**< Automatically pinned to the bottom-right corner with configured margins. */
};

/**
 * @enum FabShape
 * @brief Physical geometry corner rounding style.
 */
enum class FabShape {
    Container, /**< Rounded square conforming to Material Design 3 standards. */
    Circle     /**< Fully circular geometry. */
};

/**
 * @enum ExpandDirection
 * @brief Expansion direction for Speed Dial sub-action items.
 */
enum class ExpandDirection {
    Up,   /**< Speed dial items expand upwards. */
    Down, /**< Speed dial items expand downwards. */
    Left, /**< Speed dial items expand leftwards. */
    Right /**< Speed dial items expand rightwards. */
};

/**
 * @enum MenuAlignment
 * @brief Alignment of speed dial items relative to the primary FAB center.
 */
enum class MenuAlignment {
    Center, /**< Speed dial items are centered along the primary FAB axis. */
    Start,  /**< Speed dial items align to the start edge. */
    End     /**< Speed dial items align to the end edge. */
};

/**
 * @struct SpeedDialItem
 * @brief Descriptor for a secondary action item within an expandable Speed Dial list.
 */
struct SpeedDialItem {
    std::string label;               /**< Text label for the action item. */
    Icon icon;                       /**< Icon displayed within the sub-action button. */
    std::function<void()> onClick;   /**< Callback invoked on item selection. */
    bool isHovered = false;          /**< Cursor hover state. */
    FabType type = FabType::Secondary;/**< Color variant role. */
    bool showTextInButton = false;   /**< If true, renders label inline within an extended pill container. */
    float cachedWidth = 0.0f;        /**< Calculated layout bounding width. */
    float cachedHeight = 0.0f;       /**< Calculated layout bounding height. */
};

/**
 * @class FloatingActionButton
 * @brief Primary call-to-action button supporting elevation shadows, extended labels, and animated Speed Dial menus.
 */
class FloatingActionButton : public View {
public:
    FloatingActionButton();
    virtual ~FloatingActionButton() override = default;

    virtual void doLayout(float parentX, float parentY, float parentW, float parentH) override;
    virtual void update(float dt) override;
    virtual void render(MaterialShader& renderer, MaterialTheme& theme) override;
    virtual float getPreferredWidth() override;
    virtual float getPreferredHeight() override;

    virtual bool handleMouseMove(float mx, float my) override;
    virtual bool handleMouseButton(int button, int action, float mx, float my) override;
    void onClick() override;

    std::string text;                               /**< Display label string for Extended FAB mode. */
    Icon icon;                                      /**< Icon variant rendered inside the button. */
    FabSize fabSize = FabSize::Standard;            /**< Sizing variant mode. */
    FabType fabType = FabType::Primary;             /**< Color role variant. */
    FabShape fabShape = FabShape::Container;        /**< Corner curvature geometry shape. */
    float iconSizeDp = -1.0f;                       /**< Explicit icon dimension override in dp (-1 to use defaults). */
    std::function<void()> onClickListener;          /**< Primary action click callback. */

    ExpandDirection expandDirection = ExpandDirection::Up; /**< Expansion direction vector. */
    MenuAlignment menuAlignment = MenuAlignment::Center;   /**< Sub-item alignment relative to the main button. */
    std::vector<SpeedDialItem> speedDialItems;             /**< List of Speed Dial sub-actions. */
    bool isExpanded = false;                               /**< Indicates whether Speed Dial menu is open. */
    bool showTooltips = false;                             /**< Tooltip display flag. */

    /**
     * @brief Configures custom explicit absolute screen coordinates.
     */
    void setFloatingPosition(float customX, float customY);

    /**
     * @brief Pins the FAB to the bottom-right corner of the parent container with margins.
     */
    void setFloatingAnchorBottomRight(float marginRightDp = 32.0f, float marginBottomDp = 32.0f);

    void setIcon(const Icon& i) { icon = i; }
    void setIcon(const std::string& str) { icon = iconFromString(str); }
    void setText(const std::string& t) { text = t; }
    void setShape(FabShape s) { fabShape = s; }
    void setExpandDirection(ExpandDirection dir) { expandDirection = dir; }
    void setMenuAlignment(MenuAlignment align) { menuAlignment = align; }
    void setShowTooltips(bool enable) { showTooltips = enable; }

    /**
     * @brief Appends a secondary action to the Speed Dial list using an Icon variant.
     */
    void addSpeedDialItem(const std::string& label, const Icon& icon, std::function<void()> onClick = nullptr, bool showTextInButton = false, FabType type = FabType::Secondary);

    /**
     * @brief Appends a secondary action to the Speed Dial list using an icon string token.
     */
    void addSpeedDialItem(const std::string& label, const std::string& iconStr, std::function<void()> onClick = nullptr, bool showTextInButton = false, FabType type = FabType::Secondary);
    
    /** @brief Toggles Speed Dial menu expansion state. */
    void toggleExpand();

    /** @brief Collapses Speed Dial menu. */
    void collapse();

    bool isHovered = false;
    bool isPressed = false;

private:
    float m_expandProgress = 0.0f;

    bool m_hasCustomPosition = false;
    float m_customX = 0.0f;
    float m_customY = 0.0f;

    FabAlignment m_alignment = FabAlignment::Free;
    float m_anchorMarginRight = 32.0f;
    float m_anchorMarginBottom = 32.0f;

    void updateItemSizes(MaterialShader* renderer);
    void getSubItemBounds(size_t index, float mainW, float mainH, float ease, float& outX, float& outY, float& outW, float& outH, MaterialShader* renderer = nullptr);
};

/**
 * @class FloatingActionButtonBuilder
 * @brief Fluent builder helper for configuring and creating FloatingActionButton instances.
 */
class FloatingActionButtonBuilder {
public:
    FloatingActionButtonBuilder();
    FloatingActionButtonBuilder& text(const std::string& t);
    FloatingActionButtonBuilder& icon(const Icon& i);
    FloatingActionButtonBuilder& icon(const std::string& i);
    FloatingActionButtonBuilder& size(FabSize s);
    FloatingActionButtonBuilder& type(FabType t);
    FloatingActionButtonBuilder& shape(FabShape s);
    FloatingActionButtonBuilder& expandDirection(ExpandDirection dir);
    FloatingActionButtonBuilder& menuAlignment(MenuAlignment align);
    FloatingActionButtonBuilder& showTooltips(bool enable);
    FloatingActionButtonBuilder& iconSize(float sizeDp);
    FloatingActionButtonBuilder& position(float customX, float customY);
    FloatingActionButtonBuilder& margins(float left, float top, float right, float bottom);
    FloatingActionButtonBuilder& onClick(std::function<void()> callback);
    FloatingActionButtonBuilder& addSubItem(const std::string& label, const Icon& icon, std::function<void()> callback = nullptr, bool showTextInButton = false, FabType type = FabType::Secondary);
    FloatingActionButtonBuilder& addSubItem(const std::string& label, const std::string& iconStr, std::function<void()> callback = nullptr, bool showTextInButton = false, FabType type = FabType::Secondary);
    FloatingActionButtonBuilder& anchorBottomRight(float marginRightDp = 32.0f, float marginBottomDp = 32.0f);
    
    /**
     * @brief Allocates and initializes the configured FloatingActionButton instance.
     * 
     * @return FloatingActionButton* Pointer to heap-allocated FAB instance.
     */
    FloatingActionButton* build();

private:
    std::string m_text;
    Icon m_icon;
    FabSize m_size = FabSize::Standard;
    FabType m_type = FabType::Primary;
    FabShape m_shape = FabShape::Container;
    ExpandDirection m_expandDirection = ExpandDirection::Up;
    MenuAlignment m_menuAlignment = MenuAlignment::Center;
    bool m_showTooltips = false;
    float m_iconSizeDp = -1.0f;
    bool m_hasCustomPosition = false;
    float m_customX = 0.0f;
    float m_customY = 0.0f;
    float m_marginLeft = 0.0f;
    float m_marginTop = 0.0f;
    float m_marginRight = 0.0f;
    float m_marginBottom = 0.0f;
    FabAlignment m_alignment = FabAlignment::Free;
    float m_anchorMarginRight = 32.0f;
    float m_anchorMarginBottom = 32.0f;
    std::function<void()> m_onClick;
    std::vector<SpeedDialItem> m_subItems;
};