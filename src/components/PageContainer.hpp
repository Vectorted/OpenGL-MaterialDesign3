/**
 * @file PageContainer.hpp
 * @brief Fluid animated page container supporting Slide and FadeThrough transitions.
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
#include <functional>

/**
 * @enum TransitionType
 * @brief Transition motion animation styles for switching between pages.
 */
enum class TransitionType {
    Slide,       /**< Android ViewPager horizontal slide. */
    FadeThrough, /**< Material 3 Pure Scrim Cross-Fade. */
    None         /**< Instant switch without animation. */
};

/**
 * @class PageContainer
 * @brief Multi-page view host with animated transitions, scissor-clipped layout, and lifecycle dispatching.
 */
class PageContainer : public ViewGroup {
public:
    /**
     * @brief Constructs a new PageContainer instance with MATCH_PARENT sizing by default.
     */
    PageContainer();

    /**
     * @brief Appends a page View to the managed pages collection.
     * 
     * The first added page is automatically set to visible, while subsequent pages are hidden.
     * 
     * @param page Pointer to the child View representing the page.
     * @return int The 0-based index assigned to this page, or -1 if invalid.
     */
    int addPage(View* page);

    /**
     * @brief Switches active page to target index with customizable transition animation.
     * 
     * @param targetIndex 0-based index of destination page.
     * @param type Transition motion style variant.
     * @param duration Transition duration in seconds.
     */
    void switchToPage(size_t targetIndex, TransitionType type = TransitionType::Slide, float duration = 0.28f);

    /**
     * @brief Gets current active page index.
     * 
     * @return int 0-based index of current active page, or -1 if empty.
     */
    int getCurrentPageIndex() const { return m_currentIndex; }

    /**
     * @brief Gets total number of managed pages.
     * 
     * @return size_t Count of pages.
     */
    size_t getPageCount() const { return m_pages.size(); }

    /**
     * @brief Sets callback invoked when a page transition starts.
     * 
     * @param callback Callback taking (fromIndex, toIndex).
     */
    void setOnPageTransitionStart(std::function<void(int, int)> callback) { m_onTransitionStart = callback; }

    /**
     * @brief Sets callback invoked when a page transition finishes.
     * 
     * @param callback Callback taking (currentIndex).
     */
    void setOnPageTransitionEnd(std::function<void(int)> callback) { m_onTransitionEnd = callback; }

    /**
     * @brief Advances transition progress and propagates updates to active page views.
     * 
     * @param dt Delta time in seconds since the previous frame.
     */
    void update(float dt) override;

    /**
     * @brief Computes geometry layout positions for pages during animation and idle states.
     * 
     * @param parentX Parent container left origin.
     * @param parentY Parent container top origin.
     * @param parentW Allocated available width.
     * @param parentH Allocated available height.
     */
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;

    /**
     * @brief Renders the active pages with boundary scissor clipping and optional Scrim blending.
     * 
     * @param renderer MaterialShader rendering pipeline instance.
     * @param theme Current active theme tokens.
     */
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    /**
     * @brief Routes mouse motion events to active page views (blocked during transition).
     * 
     * @param mx Mouse X coordinate in window space.
     * @param my Mouse Y coordinate in window space.
     * @return true if the event was consumed, false otherwise.
     */
    bool handleMouseMove(float mx, float my) override;

    /**
     * @brief Routes mouse button events to active page views (blocked during transition).
     * 
     * @param button GLFW mouse button identifier.
     * @param action GLFW action (GLFW_PRESS or GLFW_RELEASE).
     * @param mx Mouse X coordinate in window space.
     * @param my Mouse Y coordinate in window space.
     * @return true if the event was consumed, false otherwise.
     */
    bool handleMouseButton(int button, int action, float mx, float my) override;

    /**
     * @brief Routes mouse scroll events to active page views (blocked during transition).
     * 
     * @param mx Mouse X coordinate in window space.
     * @param my Mouse Y coordinate in window space.
     * @param ox Horizontal scroll offset.
     * @param oy Vertical scroll offset.
     * @return true if the event was consumed, false otherwise.
     */
    bool handleScroll(float mx, float my, float ox, float oy) override;

    /**
     * @brief Routes keyboard key events to active page views (blocked during transition).
     * 
     * @param key GLFW key code.
     * @param action GLFW key action.
     * @return true if the event was consumed, false otherwise.
     */
    bool handleKey(int key, int action) override;

    /**
     * @brief Routes text input character events to active page views (blocked during transition).
     * 
     * @param codepoint Unicode character code point.
     * @return true if the event was consumed, false otherwise.
     */
    bool handleChar(unsigned int codepoint) override;

private:
    std::vector<View*> m_pages; /**< Collection of managed child page views. */

    int m_currentIndex = -1;    /**< Current active page index. */
    int m_targetIndex = -1;     /**< Target destination page index during transition. */
    int m_previousIndex = -1;   /**< Previous outgoing page index during transition. */

    bool m_isTransitioning = false;     /**< Flag indicating if an animation transition is currently active. */
    float m_transitionProgress = 1.0f;  /**< Current transition normalized progress [0.0, 1.0]. */
    float m_transitionDuration = 0.28f; /**< Total transition duration in seconds. */
    float m_slideDirection = 1.0f;      /**< Direction scalar (+1.0f forward, -1.0f backward). */

    TransitionType m_currentTransition = TransitionType::Slide; /**< Active transition motion style. */

    std::function<void(int, int)> m_onTransitionStart = nullptr; /**< Callback fired on transition commencement. */
    std::function<void(int)> m_onTransitionEnd = nullptr;        /**< Callback fired on transition completion. */
};