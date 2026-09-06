/**
 * @file ListView.hpp
 * @brief Virtualized list view component supporting dynamic item recycling and smooth kinetic scrolling.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include "ListItem.hpp"
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <cmath>
#include <glad/glad.h>

extern void requestUIWakeup(double seconds);

/**
 * @class ListView
 * @brief High-performance virtualized list view reusing ListItem instances for memory-efficient scrolling.
 */
class ListView : public View {
public:
    int m_totalItems = 0;              /**< Total number of data items in the list. */
    float m_itemHeight = 72.0f;        /**< Standard fixed height of each list row in dp. */

    double m_scrollY = 0.0;            /**< Current interpolated vertical scroll offset in pixels. */
    double m_targetScrollY = 0.0;      /**< Target vertical scroll position in pixels. */
    double m_startScrollY = 0.0;       /**< Start vertical scroll position for cubic transition. */
    float  m_scrollProgress = 1.0f;    /**< Progress factor for scroll ease-out curve. */

    std::unordered_map<int, ListItem*> m_activeViews; /**< Map of currently visible item views by index. */
    std::vector<ListItem*> m_recycledPool;            /**< Recycled item view cache pool. */

    std::function<ListItem* ()> m_createBind;         /**< Factory callback creating a new ListItem instance. */
    std::function<void(ListItem*, int)> m_updateBind; /**< Binder callback updating an item with data at index. */

    float m_lastMouseX = -1.0f;        /**< Last observed mouse X coordinate. */
    float m_lastMouseY = -1.0f;        /**< Last observed mouse Y coordinate. */

    /**
     * @brief Constructs a virtualized ListView.
     * 
     * @param totalItems Total number of items in dataset.
     * @param creator Factory callback creating view items.
     * @param updater Binder callback binding data to item views.
     */
    ListView(int totalItems, std::function<ListItem* ()> creator, std::function<void(ListItem*, int)> updater)
        : m_totalItems(totalItems), m_createBind(creator), m_updateBind(updater) {
        layout_width = MATCH_PARENT;
        layout_height = WRAP_CONTENT;
    }

    /** @brief Destructor deallocating active and pooled list item views. */
    ~ListView() override {
        for (auto& pair : m_activeViews) delete pair.second;
        for (auto* v : m_recycledPool) delete v;
    }

    void update(float dt) override {
        View::update(dt);
        const float fdt = std::min(dt, 0.033f);

        if (m_scrollProgress < 1.0f) {
            m_scrollProgress += fdt / 0.22f; // Deterministic 220ms ease-out scroll
            if (m_scrollProgress >= 1.0f) {
                m_scrollProgress = 1.0f;
                m_scrollY = m_targetScrollY;
            } else {
                float p = m_scrollProgress;
                float easeOut = 1.0f - std::pow(1.0f - p, 3.0f);
                m_scrollY = m_startScrollY + (m_targetScrollY - m_startScrollY) * (double)easeOut;
            }
            requestUIWakeup(0.016);
        } else {
            m_scrollY = m_targetScrollY;
        }
    }

    float getPreferredHeight() override {
        if (layout_height >= 0.0f) return layout_height;
        return m_totalItems * m_itemHeight;
    }

    void doLayout(float parentX, float parentY, float parentW, float parentH) override {
        x = std::round(parentX + dp(margin_left));
        y = std::round(parentY + dp(margin_top));
        if (layout_width == MATCH_PARENT) width = std::round(parentW - dp(margin_left) - dp(margin_right));
        else width = std::round(dp(layout_width));

        height = layout_height >= 0.0f ? std::round(dp(layout_height)) : std::round(dp(getPreferredHeight()));
    }

    void render(MaterialShader& renderer, MaterialTheme& theme) override {
        bool scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
        GLint last_box[4];
        if (scissorEnabled) glGetIntegerv(GL_SCISSOR_BOX, last_box);

        if (layout_height >= 0.0f && this->width > 0.0f && this->height > 0.0f) {
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            int fbH = viewport[3];

            int sx = (int)std::round(this->x);
            int sy = (int)std::round(fbH - (this->y + this->height));
            int sw = (int)std::round(this->width);
            int sh = (int)std::round(this->height);

            if (scissorEnabled) {
                int cx = std::max(sx, (int)last_box[0]);
                int cy = std::max(sy, (int)last_box[1]);
                int cw = std::max(0, std::min(sx + sw, (int)(last_box[0] + last_box[2])) - cx);
                int ch = std::max(0, std::min(sy + sh, (int)(last_box[1] + last_box[3])) - cy);
                glScissor(cx, cy, cw, ch);
            }
            else {
                glEnable(GL_SCISSOR_TEST);
                glScissor(sx, sy, sw, sh);
            }
        }

        double localTop = m_scrollY;
        double localBottom = m_scrollY + this->height;
        float itemH_px = dp(m_itemHeight);

        int startIndex = std::max(0, (int)(localTop / itemH_px) - 1);
        int endIndex = std::min(m_totalItems - 1, (int)(localBottom / itemH_px) + 1);

        std::vector<int> toRemove;
        for (auto& pair : m_activeViews) {
            if (pair.first < startIndex || pair.first > endIndex) {
                m_recycledPool.push_back(pair.second);
                toRemove.push_back(pair.first);
            }
        }
        for (int idx : toRemove) m_activeViews.erase(idx);

        for (int i = startIndex; i <= endIndex; ++i) {
            if (i < 0 || i >= m_totalItems) continue;

            ListItem* visibleBlock = nullptr;
            if (m_activeViews.count(i)) {
                visibleBlock = m_activeViews[i];
            }
            else {
                if (!m_recycledPool.empty()) {
                    visibleBlock = m_recycledPool.back();
                    m_recycledPool.pop_back();
                }
                else {
                    visibleBlock = m_createBind();
                }
                m_updateBind(visibleBlock, i);
                m_activeViews[i] = visibleBlock;
            }

            double relativeY = (i * (double)itemH_px) - m_scrollY;
            visibleBlock->x = std::round(this->x);
            visibleBlock->y = std::round(this->y + (float)relativeY);
            visibleBlock->width = std::round(this->width);
            visibleBlock->height = std::round(itemH_px);

            if (m_lastMouseX >= 0.0f && m_lastMouseY >= 0.0f) {
                if (layout_height >= 0.0f && (m_lastMouseX < this->x || m_lastMouseX > this->x + this->width ||
                    m_lastMouseY < this->y || m_lastMouseY > this->y + this->height)) {
                    visibleBlock->handleMouseMove(-1000.f, -1000.f);
                }
                else {
                    visibleBlock->handleMouseMove(m_lastMouseX, m_lastMouseY);
                }
            }
            visibleBlock->render(renderer, theme);
        }

        if (layout_height >= 0.0f && this->width > 0.0f && this->height > 0.0f) {
            if (scissorEnabled) glScissor(last_box[0], last_box[1], last_box[2], last_box[3]);
            else glDisable(GL_SCISSOR_TEST);
        }
    }

    bool handleMouseMove(float mx, float my) override {
        m_lastMouseX = mx; m_lastMouseY = my;
        if (layout_height >= 0.0f && (mx < this->x || mx > this->x + this->width || my < this->y || my > this->y + this->height)) {
            for (auto& pair : m_activeViews) pair.second->handleMouseMove(-1000.f, -1000.f);
            return false;
        }
        bool handled = false;
        for (auto& pair : m_activeViews) if (pair.second->handleMouseMove(mx, my)) handled = true;
        return handled;
    }

    bool handleMouseButton(int button, int action, float mx, float my) override {
        m_lastMouseX = mx; m_lastMouseY = my;
        if (layout_height >= 0.0f && (mx < this->x || mx > this->x + this->width || my < this->y || my > this->y + this->height)) return false;
        for (auto& pair : m_activeViews) if (pair.second->handleMouseButton(button, action, mx, my)) return true;
        return false;
    }

    bool handleScroll(float mx, float my, float ox, float oy) override {
        m_lastMouseX = mx; m_lastMouseY = my;
        if (layout_height >= 0.0f && mx >= this->x && mx <= this->x + this->width && my >= this->y && my <= this->y + this->height) {
            double newTarget = m_targetScrollY - (double)(oy * dp(100.0f));
            double maxScroll = (m_totalItems * (double)dp(m_itemHeight)) - this->height;
            if (maxScroll < 0) maxScroll = 0;
            if (newTarget < 0) newTarget = 0;
            if (newTarget > maxScroll) newTarget = maxScroll;

            if (std::abs(newTarget - m_targetScrollY) > 0.5) {
                m_startScrollY = m_scrollY;
                m_targetScrollY = newTarget;
                m_scrollProgress = 0.0f;
                requestUIWakeup(0.016);
            }
            return true;
        }
        return false;
    }
};