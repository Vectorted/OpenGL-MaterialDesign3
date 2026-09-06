/**
 * @file View.hpp
 * @brief Core visual component node definitions and layout hierarchy infrastructure.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include <vector>
#include "../theme/MaterialTheme.hpp"
#include "../shader/MaterialShader.hpp"

/**
 * @brief Global high-DPI scaling multiplier.
 */
extern float g_dpiScale;

/**
 * @brief Converts density-independent pixels (dp) to physical device pixels.
 * 
 * @param dpValue Value in dp.
 * @return float Equivalent pixel value scaled by current DPI.
 */
inline float dp(float dpValue) { return dpValue * g_dpiScale; }

/**
 * @namespace Gravity
 * @brief Bitwise alignment flags for component positioning within parent containers.
 */
namespace Gravity {
    constexpr int NONE              = 0;       /**< No alignment preference specified. */
    constexpr int LEFT              = 1 << 0;  /**< Align to parent left boundary. */
    constexpr int TOP               = 1 << 1;  /**< Align to parent top boundary. */
    constexpr int RIGHT             = 1 << 2;  /**< Align to parent right boundary. */
    constexpr int BOTTOM            = 1 << 3;  /**< Align to parent bottom boundary. */
    constexpr int CENTER_VERTICAL   = 1 << 4;  /**< Center along the vertical axis. */
    constexpr int CENTER_HORIZONTAL = 1 << 5;  /**< Center along the horizontal axis. */
    constexpr int CENTER            = CENTER_VERTICAL | CENTER_HORIZONTAL; /**< Center along both axes. */
}

/**
 * @enum UIState
 * @brief Interactive input states for visual components.
 */
enum class UIState { 
    Normal,  /**< Component is idle with no pointer interaction. */
    Hover,   /**< Pointer cursor is hovering inside component bounds. */
    Pressed  /**< Component is actively being pressed/clicked. */
};

/**
 * @enum Visibility
 * @brief Visibility states governing rendering and layout participation.
 */
enum class Visibility { 
    Visible, /**< Rendered and takes up space in layout. */
    Hidden,  /**< Not rendered, but retains space in layout. */
    Gone     /**< Not rendered and completely ignored during layout passes. */
};

/**
 * @struct M3Ripple
 * @brief Transient visual state data for Material Design 3 radial ink ripples.
 */
struct M3Ripple {
    bool active = false;      /**< Flag indicating if ripple animation is active. */
    float localX = 0.0f;      /**< Local origin X relative to component center. */
    float localY = 0.0f;      /**< Local origin Y relative to component center. */
    float radius = 0.0f;      /**< Current expansion radius in pixels. */
    float alpha = 0.0f;       /**< Current ripple opacity level [0.0, 1.0]. */
    float maxRadius = 0.0f;   /**< Maximum reachable expansion radius. */
};

/**
 * @brief Special dimension parameter indicating component should match parent size.
 */
constexpr float MATCH_PARENT = -1.0f;

/**
 * @brief Special dimension parameter indicating component should wrap content bounds.
 */
constexpr float WRAP_CONTENT = -2.0f;

/**
 * @class View
 * @brief Fundamental base visual element for all UI components in the hierarchy.
 */
class View {
public:
    float x = 0.0f;                              /**< Computed absolute X coordinate in pixels. */
    float y = 0.0f;                              /**< Computed absolute Y coordinate in pixels. */
    float width = 0.0f;                          /**< Computed layout width in pixels. */
    float height = 0.0f;                         /**< Computed layout height in pixels. */
    UIState state = UIState::Normal;             /**< Current interactive hover/press state. */
    Visibility visibility = Visibility::Visible; /**< Current layout visibility mode. */
    bool isDestroyed = false;                    /**< Flag marking view for destruction. */

    float layout_width = WRAP_CONTENT;           /**< Configured width in dp, MATCH_PARENT, or WRAP_CONTENT. */
    float layout_height = WRAP_CONTENT;          /**< Configured height in dp, MATCH_PARENT, or WRAP_CONTENT. */
    int layout_gravity = Gravity::NONE;          /**< Layout alignment flags inside parent container. */

    float margin_left = 0.0f;                    /**< External left margin in dp. */
    float margin_right = 0.0f;                   /**< External right margin in dp. */
    float margin_top = 0.0f;                     /**< External top margin in dp. */
    float margin_bottom = 0.0f;                  /**< External bottom margin in dp. */
    float padding_left = 0.0f;                   /**< Internal left padding in dp. */
    float padding_right = 0.0f;                  /**< Internal right padding in dp. */
    float padding_top = 0.0f;                    /**< Internal top padding in dp. */
    float padding_bottom = 0.0f;                 /**< Internal bottom padding in dp. */

    float hoverAnim = 0.0f;                      /**< Interpolated hover transition weight [0.0, 1.0]. */
    float pressAnim = 0.0f;                      /**< Interpolated press transition weight [0.0, 1.0]. */
    M3Ripple ripple;                             /**< Active ripple animation state descriptor. */

    /** @brief Constructs a default View instance. */
    View() = default;

    /**
     * @brief Constructs a View with explicit bounds.
     * 
     * @param x Initial X position.
     * @param y Initial Y position.
     * @param w Initial width.
     * @param h Initial height.
     */
    View(float x, float y, float w, float h);

    /** @brief Virtual destructor. */
    virtual ~View() = default;

    /**
     * @brief Sets layout sizing parameters in dp or special constants.
     * 
     * @param width Width in dp, MATCH_PARENT, or WRAP_CONTENT.
     * @param height Height in dp, MATCH_PARENT, or WRAP_CONTENT.
     */
    void setLayoutParams(float width, float height);

    /**
     * @brief Sets external margins in dp.
     * 
     * @param left Left margin in dp.
     * @param top Top margin in dp.
     * @param right Right margin in dp.
     * @param bottom Bottom margin in dp.
     */
    void setMargins(float left, float top, float right, float bottom);

    /**
     * @brief Sets internal padding in dp.
     * 
     * @param left Left padding in dp.
     * @param top Top padding in dp.
     * @param right Right padding in dp.
     * @param bottom Bottom padding in dp.
     */
    void setPadding(float left, float top, float right, float bottom);

    /**
     * @brief Sets container layout alignment gravity.
     * 
     * @param grav Combination of Gravity bit flags.
     */
    void setLayoutGravity(int grav) { layout_gravity = grav; }

    /**
     * @brief Retrieves configured layout gravity.
     * 
     * @return Integer representing Gravity bit flags.
     */
    int getLayoutGravity() const { return layout_gravity; }

    /**
     * @brief Sets component visibility mode.
     * 
     * @param v Target Visibility mode.
     */
    void setVisibility(Visibility v) { visibility = v; }

    /**
     * @brief Retrieves current component visibility mode.
     * 
     * @return Visibility enumeration state.
     */
    Visibility getVisibility() const { return visibility; }

    /**
     * @brief Checks if component is gone from layout calculation.
     * 
     * @return true if visibility is Gone or view is destroyed, false otherwise.
     */
    bool isGone() const { return visibility == Visibility::Gone || isDestroyed; }

    /**
     * @brief Checks if component is active and visible.
     * 
     * @return true if visible and not destroyed, false otherwise.
     */
    bool isVisible() const { return visibility == Visibility::Visible && !isDestroyed; }

    /**
     * @brief Calculates ideal preferred width based on content metrics.
     * 
     * @return Preferred width in dp.
     */
    virtual float getPreferredWidth();

    /**
     * @brief Calculates ideal preferred height based on content metrics.
     * 
     * @return Preferred height in dp.
     */
    virtual float getPreferredHeight();

    /**
     * @brief Updates internal animation states and interpolations.
     * 
     * @param dt Delta time elapsed since previous frame in seconds.
     */
    virtual void update(float dt);

    /**
     * @brief Calculates absolute screen coordinates and resolves child layout boundaries.
     * 
     * @param parentX Parent origin X in pixels.
     * @param parentY Parent origin Y in pixels.
     * @param parentW Allocated parent bounding width in pixels.
     * @param parentH Allocated parent bounding height in pixels.
     */
    virtual void doLayout(float parentX, float parentY, float parentW, float parentH);

    /**
     * @brief Renders visual elements via OpenGL shader pipeline.
     * 
     * @param renderer MaterialShader rendering pipeline instance.
     * @param theme MaterialTheme color token provider.
     */
    virtual void render(MaterialShader& renderer, MaterialTheme& theme);

    /**
     * @brief Handles mouse movement events and updates hover states.
     * 
     * @param mx Cursor X position in pixels.
     * @param my Cursor Y position in pixels.
     * @return true if event was consumed by this view, false otherwise.
     */
    virtual bool handleMouseMove(float mx, float my);

    /**
     * @brief Handles mouse button press and release events.
     * 
     * @param button GLFW mouse button code.
     * @param action GLFW action (e.g. GLFW_PRESS, GLFW_RELEASE).
     * @param mx Cursor X position in pixels.
     * @param my Cursor Y position in pixels.
     * @return true if event was consumed, false otherwise.
     */
    virtual bool handleMouseButton(int button, int action, float mx, float my);

    /**
     * @brief Handles scrolling wheel gestures.
     * 
     * @param mx Cursor X position in pixels.
     * @param my Cursor Y position in pixels.
     * @param ox Horizontal scroll offset delta.
     * @param oy Vertical scroll offset delta.
     * @return true if event was consumed, false otherwise.
     */
    virtual bool handleScroll(float mx, float my, float ox, float oy);

    /**
     * @brief Handles keyboard key events.
     * 
     * @param key GLFW key code.
     * @param action GLFW key action.
     * @return true if event was consumed, false otherwise.
     */
    virtual bool handleKey(int key, int action);

    /**
     * @brief Handles raw text Unicode input codepoints.
     * 
     * @param codepoint Decoded UTF-32 character.
     * @return true if consumed, false otherwise.
     */
    virtual bool handleChar(unsigned int codepoint);

    /**
     * @brief Tests if a physical coordinate falls within this view's bounding box.
     * 
     * @param px Target X coordinate in pixels.
     * @param py Target Y coordinate in pixels.
     * @return true if inside bounding box, false otherwise.
     */
    bool isInside(float px, float py);

    /** @brief Callback triggered on click activation. */
    virtual void onClick() {}

    /** @brief Callback triggered when focus is gained. */
    virtual void onFocusGained() {}

    /** @brief Callback triggered when focus is lost. */
    virtual void onFocusLost() {}

    /**
     * @brief Checks if view can receive keyboard focus.
     * 
     * @return true if focusable, false otherwise.
     */
    virtual bool isFocusable() const { return false; }
};

/**
 * @class ViewGroup
 * @brief Base composite container class managing child view collections and event routing.
 */
class ViewGroup : public View {
public:
    std::vector<View*> children; /**< Collection of child view pointers. */

    /** @brief Default constructor. */
    ViewGroup() = default;

    /**
     * @brief Constructs a ViewGroup with explicit bounds.
     * 
     * @param x Initial X position.
     * @param y Initial Y position.
     * @param w Initial width.
     * @param h Initial height.
     */
    ViewGroup(float x, float y, float w, float h) : View(x, y, w, h) {}

    /** @brief Virtual destructor. Automatically releases managed child views. */
    virtual ~ViewGroup();

    /**
     * @brief Appends a child view into this container.
     * 
     * @param child Pointer to child View.
     */
    void addView(View* child);

    /**
     * @brief Marks a child view for asynchronous destruction.
     * 
     * @param child Pointer to child View to remove.
     */
    void removeView(View* child);

    /**
     * @brief Marks all child views for removal and destruction.
     */
    void removeAllViews();

    void update(float dt) override;
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
    bool handleScroll(float mx, float my, float ox, float oy) override;
    bool handleKey(int key, int action) override;
    bool handleChar(unsigned int codepoint) override;
};