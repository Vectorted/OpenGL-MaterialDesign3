/**
 * @file View.hpp
 * @brief Core visual component node definitions and layout hierarchy infrastructure.
 * 
 * Defines the fundamental View base class and ViewGroup container, providing
 * layout, event handling, and rendering primitives for the Material 3 UI system.
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
 * 
 * This value is set by the application to convert density-independent pixels (dp)
 * to physical screen pixels. Default is 1.0.
 */
extern float g_dpiScale;

/**
 * @brief Converts density-independent pixels (dp) to physical device pixels.
 * @param dpValue Value in dp.
 * @return Value in physical pixels.
 */
inline float dp(float dpValue) { return dpValue * g_dpiScale; }

/**
 * @namespace Gravity
 * @brief Bitwise alignment flags for component positioning within parent containers.
 * 
 * These flags are used in conjunction with `layout_gravity` to control how a view
 * is positioned inside its parent ViewGroup.
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
 * 
 * Stores the current animation parameters for a ripple effect originating from
 * a touch or click event.
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
 * @class View
 * @brief Fundamental base visual element for all UI components in the hierarchy.
 * 
 * Provides common properties: position, size, margins, padding, visibility,
 * state management (hover/press), ripple animation, layout, event handling,
 * and rendering. All concrete UI components derive from this class.
 */
class View {
public:
    // --- Layout constants aligned with Android conventions ---
    static constexpr float MATCH_PARENT = -1.0f; /**< Fill the parent's available space. */
    static constexpr float WRAP_CONTENT = -2.0f; /**< Wrap to the content's preferred size. */

    float x = 0.0f;                              /**< Computed absolute X coordinate in pixels. */
    float y = 0.0f;                              /**< Computed absolute Y coordinate in pixels. */
    float width = 0.0f;                          /**< Computed layout width in pixels. */
    float height = 0.0f;                         /**< Computed layout height in pixels. */
    UIState state = UIState::Normal;             /**< Current interactive hover/press state. */
    Visibility visibility = Visibility::Visible; /**< Current layout visibility mode. */
    bool isDestroyed = false;                    /**< Flag marking view for destruction (deferred removal). */

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

    /** @brief Constructs a default View instance with zero bounds. */
    View() = default;

    /**
     * @brief Constructs a View with explicit initial bounds.
     * @param x Initial X coordinate.
     * @param y Initial Y coordinate.
     * @param w Initial width.
     * @param h Initial height.
     */
    View(float x, float y, float w, float h);

    /** @brief Virtual destructor. */
    virtual ~View() = default;

    /**
     * @brief Sets layout sizing parameters in dp or special constants (MATCH_PARENT, WRAP_CONTENT).
     * @param width Layout width (dp or constant).
     * @param height Layout height (dp or constant).
     */
    void setLayoutParams(float width, float height);

    /**
     * @brief Sets external margins in dp.
     * @param left Left margin.
     * @param top Top margin.
     * @param right Right margin.
     * @param bottom Bottom margin.
     */
    void setMargins(float left, float top, float right, float bottom);

    /**
     * @brief Sets internal padding in dp.
     * @param left Left padding.
     * @param top Top padding.
     * @param right Right padding.
     * @param bottom Bottom padding.
     */
    void setPadding(float left, float top, float right, float bottom);

    /**
     * @brief Sets container layout alignment gravity.
     * @param grav Gravity flags (bitwise OR of Gravity constants).
     */
    void setLayoutGravity(int grav) { layout_gravity = grav; }

    /**
     * @brief Retrieves configured layout gravity.
     * @return Gravity flags.
     */
    int getLayoutGravity() const { return layout_gravity; }

    /**
     * @brief Sets component visibility mode.
     * @param v Desired Visibility.
     */
    void setVisibility(Visibility v) { visibility = v; }

    /**
     * @brief Retrieves current component visibility mode.
     * @return Current Visibility.
     */
    Visibility getVisibility() const { return visibility; }

    /**
     * @brief Checks if component is gone (not participating in layout).
     * @return true if visibility is Gone or the view is marked for destruction.
     */
    bool isGone() const { return visibility == Visibility::Gone || isDestroyed; }

    /**
     * @brief Checks if component is active and visible.
     * @return true if visible and not destroyed.
     */
    bool isVisible() const { return visibility == Visibility::Visible && !isDestroyed; }

    /**
     * @brief Calculates ideal preferred width based on content metrics.
     * @return Preferred width in dp.
     */
    virtual float getPreferredWidth();

    /**
     * @brief Calculates ideal preferred height based on content metrics.
     * @return Preferred height in dp.
     */
    virtual float getPreferredHeight();

    /**
     * @brief Updates internal animation states (hover, press, ripple) over time.
     * @param dt Delta time in seconds since last update.
     */
    virtual void update(float dt);

    /**
     * @brief Calculates absolute screen coordinates and resolves child layout boundaries.
     * @param parentX Parent container's X coordinate.
     * @param parentY Parent container's Y coordinate.
     * @param parentW Parent container's width.
     * @param parentH Parent container's height.
     */
    virtual void doLayout(float parentX, float parentY, float parentW, float parentH);

    /**
     * @brief Renders visual elements via the OpenGL shader pipeline.
     * @param renderer Reference to the MaterialShader renderer.
     * @param theme Current MaterialTheme for color tokens.
     */
    virtual void render(MaterialShader& renderer, MaterialTheme& theme);

    /**
     * @brief Handles mouse movement events and updates hover states.
     * @param mx Mouse X coordinate in screen space.
     * @param my Mouse Y coordinate in screen space.
     * @return true if the event was consumed.
     */
    virtual bool handleMouseMove(float mx, float my);

    /**
     * @brief Handles mouse button press and release events.
     * @param button Mouse button identifier (GLFW constant).
     * @param action GLFW_PRESS or GLFW_RELEASE.
     * @param mx Mouse X coordinate.
     * @param my Mouse Y coordinate.
     * @return true if the event was consumed.
     */
    virtual bool handleMouseButton(int button, int action, float mx, float my);

    /**
     * @brief Handles scrolling wheel gestures.
     * @param mx Mouse X coordinate (unused by default).
     * @param my Mouse Y coordinate (unused by default).
     * @param ox Horizontal scroll delta.
     * @param oy Vertical scroll delta.
     * @return true if consumed.
     */
    virtual bool handleScroll(float mx, float my, float ox, float oy);

    /**
     * @brief Handles keyboard key events.
     * @param key GLFW key code.
     * @param action GLFW_PRESS, GLFW_RELEASE, or GLFW_REPEAT.
     * @return true if consumed.
     */
    virtual bool handleKey(int key, int action);

    /**
     * @brief Handles raw text Unicode input codepoints.
     * @param codepoint Unicode code point.
     * @return true if consumed.
     */
    virtual bool handleChar(unsigned int codepoint);

    /**
     * @brief Tests if a physical coordinate falls within this view's bounding box.
     * @param px X coordinate in screen space.
     * @param py Y coordinate in screen space.
     * @return true if inside.
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
     * @return true if focusable.
     */
    virtual bool isFocusable() const { return false; }
};

/**
 * @class ViewGroup
 * @brief Base composite container class managing child view collections and event routing.
 * 
 * Extends View to support a list of children. Handles layout, rendering,
 * and event dispatch to children in reverse order (topmost first).
 * Also manages child destruction and deferred removal.
 */
class ViewGroup : public View {
public:
    std::vector<View*> children; /**< Collection of child view pointers. */

    /** @brief Default constructor. */
    ViewGroup() = default;

    /**
     * @brief Constructs a ViewGroup with explicit bounds.
     * @param x Initial X coordinate.
     * @param y Initial Y coordinate.
     * @param w Initial width.
     * @param h Initial height.
     */
    ViewGroup(float x, float y, float w, float h) : View(x, y, w, h) {}

    /** @brief Virtual destructor – deletes all children. */
    virtual ~ViewGroup();

    /**
     * @brief Adds a child view to the container.
     * @param child Pointer to the child view (ownership transferred).
     */
    void addView(View* child);

    /**
     * @brief Marks a child for removal (deferred deletion).
     * @param child Pointer to the child view.
     */
    void removeView(View* child);

    /** @brief Marks all children for removal. */
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

// --- Global scope aliases for backward compatibility with existing code ---
inline constexpr float MATCH_PARENT = View::MATCH_PARENT;
inline constexpr float WRAP_CONTENT = View::WRAP_CONTENT;
