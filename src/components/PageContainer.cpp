/**
 * @file PageContainer.cpp
 * @brief Implementation of fluid page container animations with scissor clipping and scrim fade blending.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "PageContainer.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>

extern void requestUIWakeup(double seconds);

/**
 * @brief Constructs a new PageContainer instance.
 */
PageContainer::PageContainer() {
    setLayoutParams(MATCH_PARENT, MATCH_PARENT);
}

/**
 * @brief Appends a page View to the managed pages collection.
 */
int PageContainer::addPage(View* page) {
    if (!page) return -1;

    int pageIdx = static_cast<int>(m_pages.size());
    m_pages.push_back(page);
    addView(page);

    if (m_currentIndex == -1) {
        m_currentIndex = 0;
        page->setVisibility(Visibility::Visible);
    } else {
        page->setVisibility(Visibility::Gone);
    }

    return pageIdx;
}

/**
 * @brief Switches active page to target index with customizable transition animation.
 */
void PageContainer::switchToPage(size_t targetIndex, TransitionType type, float duration) {
    if (targetIndex >= m_pages.size()) return;
    if (static_cast<int>(targetIndex) == m_currentIndex && !m_isTransitioning) return;

    int fromIndex = m_currentIndex;
    int toIndex = static_cast<int>(targetIndex);

    // Instant switch
    if (type == TransitionType::None || duration <= 0.001f || m_currentIndex == -1) {
        if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_pages.size())) {
            m_pages[m_currentIndex]->setVisibility(Visibility::Gone);
        }
        m_currentIndex = toIndex;
        m_pages[m_currentIndex]->setVisibility(Visibility::Visible);
        m_isTransitioning = false;
        if (m_onTransitionEnd) m_onTransitionEnd(m_currentIndex);
        return;
    }

    m_previousIndex = m_currentIndex;
    m_targetIndex = toIndex;
    m_currentTransition = type;
    m_transitionDuration = (std::max)(0.05f, duration);
    m_transitionProgress = 0.0f;
    m_isTransitioning = true;

    // Determine direction: Forward (+1.0f) or Backward (-1.0f)
    m_slideDirection = (toIndex >= fromIndex) ? 1.0f : -1.0f;

    if (m_previousIndex >= 0 && m_previousIndex < static_cast<int>(m_pages.size())) {
        m_pages[m_previousIndex]->setVisibility(Visibility::Visible);
    }
    if (m_targetIndex >= 0 && m_targetIndex < static_cast<int>(m_pages.size())) {
        m_pages[m_targetIndex]->setVisibility(Visibility::Visible);
    }

    if (m_onTransitionStart) {
        m_onTransitionStart(m_previousIndex, m_targetIndex);
    }

    requestUIWakeup(m_transitionDuration + 0.05);
}

/**
 * @brief Updates transition progress and manages child update ticks.
 */
void PageContainer::update(float dt) {
    View::update(dt);

    if (m_isTransitioning) {
        float fdt = (std::min)(dt, 0.033f);
        m_transitionProgress += fdt / m_transitionDuration;

        if (m_transitionProgress >= 1.0f) {
            m_transitionProgress = 1.0f;
            m_isTransitioning = false;

            if (m_previousIndex >= 0 && m_previousIndex < static_cast<int>(m_pages.size())) {
                m_pages[m_previousIndex]->setVisibility(Visibility::Gone);
            }

            m_currentIndex = m_targetIndex;

            if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_pages.size())) {
                m_pages[m_currentIndex]->setVisibility(Visibility::Visible);
                m_pages[m_currentIndex]->doLayout(this->x, this->y, this->width, this->height);
            }

            if (m_onTransitionEnd) {
                m_onTransitionEnd(m_currentIndex);
            }
        } else {
            // Re-layout active page offsets per animation tick
            this->doLayout(this->x, this->y, this->width, this->height);
            requestUIWakeup(0.016);
        }
    }

    if (m_isTransitioning) {
        if (m_previousIndex >= 0 && m_previousIndex < static_cast<int>(m_pages.size())) {
            m_pages[m_previousIndex]->update(dt);
        }
        if (m_targetIndex >= 0 && m_targetIndex < static_cast<int>(m_pages.size())) {
            m_pages[m_targetIndex]->update(dt);
        }
    } else {
        if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_pages.size())) {
            m_pages[m_currentIndex]->update(dt);
        }
    }
}

/**
 * @brief Calculates dynamic positions of pages according to active transition mode.
 */
void PageContainer::doLayout(float parentX, float parentY, float parentW, float parentH) {
    View::doLayout(parentX, parentY, parentW, parentH);

    float innerX = this->x;
    float innerY = this->y;
    float innerW = this->width;
    float innerH = this->height;

    if (!m_isTransitioning) {
        if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_pages.size())) {
            m_pages[m_currentIndex]->doLayout(innerX, innerY, innerW, innerH);
        }
        return;
    }

    float t = (std::clamp)(m_transitionProgress, 0.0f, 1.0f);
    float easeOut = 1.0f - std::pow(1.0f - t, 3.2f);

    View* prevPage = (m_previousIndex >= 0 && m_previousIndex < static_cast<int>(m_pages.size())) ? m_pages[m_previousIndex] : nullptr;
    View* targetPage = (m_targetIndex >= 0 && m_targetIndex < static_cast<int>(m_pages.size())) ? m_pages[m_targetIndex] : nullptr;

    switch (m_currentTransition) {
        case TransitionType::Slide: {
            /* 
             * Android ViewPager standard full horizontal sliding:
             * Page dimensions are maintained at 100% innerW to prevent content squishing and text relayout.
             */
            float gap = dp(24.0f);
            float totalDist = innerW + gap;

            if (prevPage) {
                float prevOffsetX = -m_slideDirection * totalDist * easeOut;
                prevPage->doLayout(std::round(innerX + prevOffsetX), innerY, innerW, innerH);
            }
            if (targetPage) {
                float targetOffsetX = m_slideDirection * totalDist * (1.0f - easeOut);
                targetPage->doLayout(std::round(innerX + targetOffsetX), innerY, innerW, innerH);
            }
            break;
        }

        case TransitionType::FadeThrough: {
            /* 
             * Material 3 subtle fade-through:
             * Both pages stay anchored in place while the Scrim overlay dissolves smoothly.
             */
            if (prevPage) prevPage->doLayout(innerX, innerY, innerW, innerH);
            if (targetPage) targetPage->doLayout(innerX, innerY, innerW, innerH);
            break;
        }

        case TransitionType::None:
        default: {
            if (targetPage) targetPage->doLayout(innerX, innerY, innerW, innerH);
            break;
        }
    }
}

/**
 * @brief Renders the page container with hardware viewport scissor clipping and transition effects.
 */
void PageContainer::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (m_pages.empty()) return;

    if (!m_isTransitioning) {
        if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_pages.size())) {
            View* activePage = m_pages[m_currentIndex];
            if (activePage && activePage->getVisibility() == Visibility::Visible) {
                activePage->render(renderer, theme);
            }
        }
        return;
    }

    // Scissor viewport clipping prevents animations from overflowing container bounds
    int fbW = 0, fbH = 0;
    GLFWwindow* win = glfwGetCurrentContext();
    if (win) glfwGetFramebufferSize(win, &fbW, &fbH);

    float scissorY = (float)fbH - (this->y + this->height);
    glEnable(GL_SCISSOR_TEST);
    glScissor((int)this->x, (int)scissorY, (int)this->width, (int)this->height);

    float t = (std::clamp)(m_transitionProgress, 0.0f, 1.0f);
    View* prevPage = (m_previousIndex >= 0 && m_previousIndex < static_cast<int>(m_pages.size())) ? m_pages[m_previousIndex] : nullptr;
    View* targetPage = (m_targetIndex >= 0 && m_targetIndex < static_cast<int>(m_pages.size())) ? m_pages[m_targetIndex] : nullptr;

    switch (m_currentTransition) {
        case TransitionType::FadeThrough: {
            /*
             * True smooth Scrim dissolving cross-fade:
             * First half (0.0 -> 0.5): Previous page rendered under increasing surface scrim (Alpha 0.0 -> 1.0)
             * Second half (0.5 -> 1.0): Target page rendered under decreasing surface scrim (Alpha 1.0 -> 0.0)
             */
            M3Color bg = theme.surface;

            if (t < 0.5f) {
                if (prevPage) prevPage->render(renderer, theme);
                float maskAlpha = t / 0.5f; // 0.0 -> 1.0
                M3Color scrimColor = { bg.r, bg.g, bg.b, maskAlpha };
                renderer.drawM3UI(this->x, this->y, this->width, this->height, 0.0f, 0.0f, 0.0f, 0.0f, scrimColor);
            } else {
                if (targetPage) targetPage->render(renderer, theme);
                float maskAlpha = 1.0f - ((t - 0.5f) / 0.5f); // 1.0 -> 0.0
                M3Color scrimColor = { bg.r, bg.g, bg.b, maskAlpha };
                renderer.drawM3UI(this->x, this->y, this->width, this->height, 0.0f, 0.0f, 0.0f, 0.0f, scrimColor);
            }
            break;
        }

        case TransitionType::Slide:
        default: {
            if (prevPage) prevPage->render(renderer, theme);
            if (targetPage) targetPage->render(renderer, theme);
            break;
        }
    }

    glDisable(GL_SCISSOR_TEST);
}

/**
 * @brief Dispatches mouse movement to the active child page.
 */
bool PageContainer::handleMouseMove(float mx, float my) {
    if (m_isTransitioning) return true; 
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_pages.size())) {
        return m_pages[m_currentIndex]->handleMouseMove(mx, my);
    }
    return false;
}

/**
 * @brief Dispatches mouse button clicks to the active child page.
 */
bool PageContainer::handleMouseButton(int button, int action, float mx, float my) {
    if (m_isTransitioning) return true; 
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_pages.size())) {
        return m_pages[m_currentIndex]->handleMouseButton(button, action, mx, my);
    }
    return false;
}

/**
 * @brief Dispatches scroll events to the active child page.
 */
bool PageContainer::handleScroll(float mx, float my, float ox, float oy) {
    if (m_isTransitioning) return true;
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_pages.size())) {
        return m_pages[m_currentIndex]->handleScroll(mx, my, ox, oy);
    }
    return false;
}

/**
 * @brief Dispatches physical keyboard events to the active child page.
 */
bool PageContainer::handleKey(int key, int action) {
    if (m_isTransitioning) return true;
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_pages.size())) {
        return m_pages[m_currentIndex]->handleKey(key, action);
    }
    return false;
}

/**
 * @brief Dispatches character text inputs to the active child page.
 */
bool PageContainer::handleChar(unsigned int codepoint) {
    if (m_isTransitioning) return true;
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_pages.size())) {
        return m_pages[m_currentIndex]->handleChar(codepoint);
    }
    return false;
}