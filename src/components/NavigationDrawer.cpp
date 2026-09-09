/**
 * @file NavigationDrawer.cpp
 * @brief High-performance Material Design 3 Navigation Drawer with Viewport-Staged Velocity Animation.
 * 
 * Implements a modal navigation drawer with expandable groups, kinetic scrolling,
 * and a viewport-aware staged velocity algorithm for smooth expansion of large
 * groups. Uses scissor clipping and viewport culling for optimal rendering performance.
 * 
 * @author Vectorted
 * @repository github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "NavigationDrawer.hpp"
#include <algorithm>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

extern void requestUIWakeup(double seconds);

DrawerItem::DrawerItem(int id, const std::string& label, const std::string& iconStr, NavigationDrawer* parent, int indentLevel)
    : m_id(id), m_label(label), m_icon(iconFromString(iconStr)), m_parent(parent), m_indentLevel(indentLevel) {
    layout_width = MATCH_PARENT;
}

void DrawerItem::setSelected(bool selected, bool instant) { 
    m_selected = selected; 
}

void DrawerItem::update(float dt) { 
    View::update(dt); 
}

void DrawerItem::onClick() { 
    if (m_parent) m_parent->onItemClicked(m_id); 
}

void DrawerItem::render(MaterialShader& renderer, MaterialTheme& theme) {
    float masterAlpha = m_parent ? m_parent->getMasterAlpha() : 1.0f;

    float paddingH = std::round(dp(12.0f));
    float marginY  = std::round(dp(2.0f));
    float indent   = m_indentLevel * std::round(dp(8.0f));
    
    float pillX = std::round(x + paddingH + indent);
    float pillY = std::round(y + marginY);
    float pillW = std::round(width - paddingH * 2.0f - indent);
    
    float rawPillH = height - marginY * 2.0f;
    float pillH = std::floor(rawPillH * 0.5f) * 2.0f; 
    float radius = pillH * 0.5f;

    float centerY = pillY + pillH * 0.5f;
    float stateLayerAlpha = hoverAnim * 0.08f + pressAnim * 0.12f;
    
    // Selected background
    if (m_selected) {
        M3Color activeBg = theme.secondaryContainer; 
        activeBg.a *= masterAlpha;
        renderer.drawM3UI(pillX, pillY, pillW, pillH, radius, radius, radius, radius, activeBg);
    }
    
    // Hover/press state layer
    if (stateLayerAlpha > 0.001f) {
        M3Color hoverColor = m_selected ? theme.onSecondaryContainer : theme.onSurfaceVariant;
        hoverColor.a = stateLayerAlpha * masterAlpha;
        renderer.drawM3UI(pillX, pillY, pillW, pillH, radius, radius, radius, radius, hoverColor);
    }

    // Icon
    float iconSize = std::floor(dp(22.0f) * 0.5f) * 2.0f; 
    float iconY = centerY - iconSize * 0.5f;
    float iconX = std::round(pillX + dp(14.0f));

    M3Color contentColor = m_selected ? theme.onSecondaryContainer : theme.onSurfaceVariant;
    contentColor.a *= masterAlpha;

    if (!isIconEmpty(m_icon)) { 
        renderer.drawIcon(iconX, iconY, iconSize, m_icon, contentColor); 
    }
    
    // Text label
    float textX = std::round(iconX + iconSize + dp(10.0f)); 
    float textSize = dp(14.0f);
    float textDrawY = std::round(centerY - textSize * 0.5f);
    
    renderer.drawText(m_label, textX, textDrawY, textSize, contentColor);
}

DrawerHeader::DrawerHeader(const std::string& title, NavigationDrawer* parent) 
    : m_title(title), m_parent(parent) { 
    layout_width = MATCH_PARENT; 
}

void DrawerHeader::render(MaterialShader& renderer, MaterialTheme& theme) {
    float masterAlpha = m_parent ? m_parent->getMasterAlpha() : 1.0f;
    M3Color color = theme.onSurfaceVariant;  
    color.a *= (0.85f * masterAlpha);
    
    float centerY = y + height * 0.5f;
    float textX = std::round(x + dp(28.0f));
    float textSize = dp(13.0f);
    float textY = std::round(centerY - textSize * 0.5f);
    renderer.drawText(m_title, textX, textY, textSize, color);
}

DrawerDivider::DrawerDivider(NavigationDrawer* parent) : m_parent(parent) {
    layout_width = MATCH_PARENT;
}

void DrawerDivider::render(MaterialShader& renderer, MaterialTheme& theme) {
    float masterAlpha = m_parent ? m_parent->getMasterAlpha() : 1.0f;
    M3Color color = theme.outlineVariant; 
    color.a = 0.5f * masterAlpha;
    float padding = std::round(dp(28.0f));
    float centerY = std::round(y + height * 0.5f);
    renderer.drawM3UI(std::round(x + padding), centerY, std::round(width - padding * 2.0f), std::round(dp(1.0f)), 0.0f, 0.0f, 0.0f, 0.0f, color);
}

DrawerExpandableGroup::DrawerExpandableGroup(const std::string& title, const std::string& iconStr, bool defaultExpanded, NavigationDrawer* drawer, int indentLevel)
    : m_title(title),
      m_isExpanded(defaultExpanded),
      m_expandAnim(defaultExpanded ? 1.0f : 0.0f),
      m_startAnim(defaultExpanded ? 1.0f : 0.0f),
      m_animProgress(1.0f),
      m_drawer(drawer),
      m_indentLevel(indentLevel) {
    layout_width = MATCH_PARENT;
    if (!iconStr.empty()) {
        m_icon = iconFromString(iconStr);
        m_hasIcon = !isIconEmpty(m_icon);
    }
}

void DrawerExpandableGroup::addItem(int id, const std::string& label, const std::string& iconStr) {
    addView(new DrawerItem(id, label, iconStr, m_drawer, m_indentLevel + 1));
}

DrawerExpandableGroup* DrawerExpandableGroup::addGroup(const std::string& title, bool defaultExpanded, const std::string& iconStr) {
    auto* subGroup = new DrawerExpandableGroup(title, iconStr, defaultExpanded, m_drawer, m_indentLevel + 1);
    addView(subGroup);
    return subGroup;
}

void DrawerExpandableGroup::setExpanded(bool expanded) {
    if (m_isExpanded == expanded) return;
    m_isExpanded = expanded;
    
    m_startAnim = m_expandAnim;
    m_animProgress = 0.0f;
    requestUIWakeup(ANIM_DURATION + 0.05);
}

float DrawerExpandableGroup::getPreferredHeight() {
    float headerH = 48.0f;
    if (m_expandAnim <= 0.0001f) {
        return headerH;
    }

    float childrenH = 0.0f;
    for (View* child : children) {
        childrenH += child->getPreferredHeight();
    }

    // Compute visible area within the drawer's viewport
    float visibleV = childrenH;
    if (m_drawer) {
        float drawerBottom = m_drawer->y + m_drawer->height;
        float headerBottom = y + headerH;
        float availableH = std::max(0.0f, drawerBottom - headerBottom);
        float availableDp = availableH / (dp(1.0f) > 0.0f ? dp(1.0f) : 1.0f);
        visibleV = std::clamp(availableDp, 48.0f, childrenH);
    }

    float p = m_expandAnim; // 0.0 -> 1.0

    // Case 1: children fit entirely within viewport — smooth uniform acceleration
    if (childrenH <= visibleV + 1.0f) {
        float uCurve = 0.85f * p + 0.15f * (p * p);
        return headerH + childrenH * uCurve;
    }

    // Case 2: children exceed viewport — staged velocity algorithm
    // First 82% of time expands the visible portion (linear+small acceleration),
    // remaining 18% uses rapid quadratic sprint to finish.
    const float SPLIT = 0.82f;
    float currentChildrenH = 0.0f;

    if (p <= SPLIT) {
        float u = p / SPLIT;
        float uCurve = 0.85f * u + 0.15f * (u * u);
        currentChildrenH = visibleV * uCurve;
    } else {
        float w = (p - SPLIT) / (1.0f - SPLIT);
        float wCurve = w * w;
        currentChildrenH = visibleV + (childrenH - visibleV) * wCurve;
    }

    return headerH + currentChildrenH;
}

void DrawerExpandableGroup::update(float dt) {
    if (m_expandAnim > 0.001f) {
        ViewGroup::update(dt);
    } else {
        View::update(dt);
    }

    if (m_animProgress < 1.0f) {
        float dtSafe = std::min(dt, 0.033f);
        m_animProgress += dtSafe / ANIM_DURATION;
        
        if (m_animProgress >= 1.0f) {
            m_animProgress = 1.0f;
            m_expandAnim = m_isExpanded ? 1.0f : 0.0f;
        } else {
            float target = m_isExpanded ? 1.0f : 0.0f;
            // Linear progress; getPreferredHeight() applies the staged velocity curve
            m_expandAnim = m_startAnim + (target - m_startAnim) * m_animProgress;
            requestUIWakeup(0.016);
        }
    }
}

void DrawerExpandableGroup::doLayout(float parentX, float parentY, float parentW, float parentH) {
    x = std::round(parentX);
    y = std::round(parentY);
    width = std::round(parentW);
    height = std::round(parentH);

    if (m_expandAnim <= 0.0001f) return;

    float headerH = std::round(dp(48.0f));
    float curY = y + headerH;

    for (View* child : children) {
        float chH = std::round(dp(child->getPreferredHeight()));
        child->doLayout(x, curY, width, chH);
        curY += chH;
    }
}

bool DrawerExpandableGroup::handleMouseMove(float mx, float my) {
    bool inside = isInside(mx, my);
    if (m_expandAnim > 0.05f) {
        if (m_drawer && (my < m_drawer->y || my > m_drawer->y + m_drawer->height)) {
            return inside;
        }
        ViewGroup::handleMouseMove(mx, my);
    } else {
        View::handleMouseMove(mx, my);
    }
    return inside;
}

bool DrawerExpandableGroup::handleMouseButton(int button, int action, float mx, float my) {
    if (button == 0) {
        float headerH = std::round(dp(48.0f));
        if (mx >= x && mx <= x + width && my >= y && my <= y + headerH) {
            if (action == 1) {
                toggle();
            }
            return true;
        }

        if (m_expandAnim > 0.05f && my > y + headerH && my <= y + height) {
            if (m_drawer && (my < m_drawer->y || my > m_drawer->y + m_drawer->height)) {
                return false;
            }
            return ViewGroup::handleMouseButton(button, action, mx, my);
        }
    }
    return false;
}

void DrawerExpandableGroup::render(MaterialShader& renderer, MaterialTheme& theme) {
    float masterAlpha = m_drawer ? m_drawer->getMasterAlpha() : 1.0f;
    float headerH = std::round(dp(48.0f));

    float paddingH = std::round(dp(12.0f));
    float marginY  = std::round(dp(2.0f));
    float indent   = m_indentLevel * std::round(dp(8.0f));

    float pillX = std::round(x + paddingH + indent);
    float pillY = std::round(y + marginY);
    float pillW = std::round(width - paddingH * 2.0f - indent);
    
    float rawPillH = headerH - marginY * 2.0f;
    float pillH = std::floor(rawPillH * 0.5f) * 2.0f;
    float radius = std::round(dp(12.0f));
    float centerY = pillY + pillH * 0.5f;

    // 1. Header hover background
    if (hoverAnim > 0.01f) {
        M3Color hoverBg = theme.onSurfaceVariant;
        hoverBg.a = hoverAnim * 0.06f * masterAlpha;
        renderer.drawM3UI(pillX, pillY, pillW, pillH, radius, radius, radius, radius, hoverBg);
    }

    // 2. Header icon
    float contentX = std::round(pillX + dp(14.0f));
    if (m_hasIcon) {
        float iconSize = std::floor(dp(20.0f) * 0.5f) * 2.0f;
        M3Color iconColor = theme.onSurfaceVariant; 
        iconColor.a *= masterAlpha;
        renderer.drawIcon(contentX, centerY - iconSize * 0.5f, iconSize, m_icon, iconColor);
        contentX += iconSize + std::round(dp(10.0f));
    }

    // 3. Header title
    M3Color titleColor = theme.onSurface;
    titleColor.a *= (0.85f * masterAlpha);
    float textSize = dp(14.0f);
    float textY = std::round(centerY - textSize * 0.5f);
    renderer.drawText(m_title, contentX, textY, textSize, titleColor);

    // 4. Rotating arrow indicator
    float arrowSize = std::floor(dp(18.0f) * 0.5f) * 2.0f;
    float arrowX = std::round(pillX + pillW - arrowSize - dp(12.0f));
    float arrowY = centerY - arrowSize * 0.5f;
    
    float rotAngle = m_expandAnim * 3.14159265f;
    float rotCx = arrowX + arrowSize * 0.5f;
    float rotCy = arrowY + arrowSize * 0.5f;

    Icon arrowIcon = IconM3(M3Icon::ChevronDown);
    M3Color arrowColor = theme.onSurfaceVariant;
    arrowColor.a *= (0.7f * masterAlpha);
    renderer.drawIcon(arrowX, arrowY, arrowSize, arrowIcon, arrowColor, 0.0f, rotAngle, rotCx, rotCy);

    // 5. Child rendering with scissor clipping and viewport culling
    float groupContentH = height - headerH;
    if (m_expandAnim > 0.001f && groupContentH > 0.5f) {
        GLFWwindow* win = glfwGetCurrentContext();
        int fbW = 0, fbH = 0;
        if (win) glfwGetFramebufferSize(win, &fbW, &fbH);

        float groupClipTop = y + headerH;
        float groupClipBottom = y + height;

        float drawerTop = m_drawer ? m_drawer->y : 0.0f;
        float drawerBottom = m_drawer ? (m_drawer->y + m_drawer->height) : (float)fbH;

        float effectiveTop = std::max(groupClipTop, drawerTop);
        float effectiveBottom = std::min(groupClipBottom, drawerBottom);

        if (effectiveTop < effectiveBottom && fbH > 0) {
            int scissorX = (int)std::max(0.0f, x);
            int scissorY = (int)std::round((float)fbH - effectiveBottom);
            int scissorW = (int)std::round(width);
            int scissorH = (int)std::round(effectiveBottom - effectiveTop);

            glScissor(scissorX, scissorY, std::max(0, scissorW), std::max(0, scissorH));

            for (View* child : children) {
                if (child->y + child->height <= effectiveTop || child->y >= effectiveBottom) {
                    continue;
                }
                child->render(renderer, theme);
            }

            // Restore the main drawer scissor (will be re-applied by the drawer itself)
            if (m_drawer) {
                int mainScissorY = (int)std::round((float)fbH - (m_drawer->y + m_drawer->height));
                glScissor(
                    (int)std::max(0.0f, m_drawer->x),
                    mainScissorY,
                    (int)std::round(m_drawer->width),
                    (int)std::round(m_drawer->height)
                );
            }
        }
    }
}

NavigationDrawer::NavigationDrawer() { 
    layout_width = MATCH_PARENT; 
    layout_height = MATCH_PARENT; 
}

void NavigationDrawer::open() { m_isOpen = true; requestUIWakeup(0.35); }
void NavigationDrawer::close() { m_isOpen = false; requestUIWakeup(0.35); }
void NavigationDrawer::toggle() { m_isOpen = !m_isOpen; requestUIWakeup(0.35); }

void NavigationDrawer::addItem(int id, const std::string& label, const std::string& iconStr) {
    DrawerItem* item = new DrawerItem(id, label, iconStr, this, 0);
    if (children.empty()) { 
        item->setSelected(true, true); 
        m_selectedIndex = id; 
    }
    addView(item);
}

void NavigationDrawer::addHeader(const std::string& title) { 
    addView(new DrawerHeader(title, this)); 
}

void NavigationDrawer::addDivider() { 
    addView(new DrawerDivider(this)); 
}

DrawerExpandableGroup* NavigationDrawer::addGroup(const std::string& title, bool defaultExpanded, const std::string& iconStr) {
    auto* group = new DrawerExpandableGroup(title, iconStr, defaultExpanded, this, 0);
    addView(group);
    return group;
}

void NavigationDrawer::setOnItemSelected(std::function<void(int)> callback) { 
    m_onItemSelectedCallback = std::move(callback); 
}

static void updateDrawerSelection(const std::vector<View*>& views, int targetId) {
    for (View* v : views) {
        if (auto* item = dynamic_cast<DrawerItem*>(v)) {
            item->setSelected(item->getId() == targetId);
        } else if (auto* group = dynamic_cast<DrawerExpandableGroup*>(v)) {
            updateDrawerSelection(group->children, targetId);
        }
    }
}

void NavigationDrawer::setSelectedIndex(int id) {
    if (m_selectedIndex == id) return;
    updateDrawerSelection(children, id);
    m_selectedIndex = id;
    if (m_onItemSelectedCallback) m_onItemSelectedCallback(id);
}

void NavigationDrawer::onItemClicked(int id) { 
    setSelectedIndex(id); 
    close(); 
}

float NavigationDrawer::getMasterAlpha() const {
    if (m_animStyle == DrawerAnimStyle::SharedAxis) return std::pow(m_slideAnim, 2.0f);
    return 1.0f;
}

void NavigationDrawer::update(float dt) {
    if (m_slideAnim <= 0.001f && !m_isOpen) return;

    ViewGroup::update(dt);
    float dtSafe = std::min(dt, 0.033f);
    float animSpeed = 4.5f;

    if (m_isOpen) {
        m_slideAnim += animSpeed * dtSafe;
        if (m_slideAnim > 1.0f) m_slideAnim = 1.0f;
    } else {
        m_slideAnim -= animSpeed * dtSafe;
        if (m_slideAnim < 0.0f) m_slideAnim = 0.0f;
    }

    if (m_slideAnim > 0.0f && m_slideAnim < 1.0f) {
        requestUIWakeup(0.016);
    }

    float scrollDiff = m_targetScrollY - m_scrollY;
    if (std::abs(scrollDiff) >= 0.5f) {
        float factor = 1.0f - std::exp(-24.0f * dtSafe);
        m_scrollY += scrollDiff * factor;
        requestUIWakeup(0.016);
    } else {
        m_scrollY = m_targetScrollY;
    }
}

void NavigationDrawer::doLayout(float parentX, float parentY, float parentW, float parentH) {
    x = std::round(parentX); 
    y = std::round(parentY); 
    width = std::round(parentW); 
    height = std::round(parentH);

    if (m_slideAnim <= 0.001f && !m_isOpen) return;

    float easeOutCubic = 1.0f - std::pow(1.0f - m_slideAnim, 3.0f);
    float panelW = std::min(std::round(dp(m_panelWidthBase)), width - std::round(dp(48.0f)));

    float slideDist = (m_animStyle == DrawerAnimStyle::Slide) ? panelW : std::round(dp(64.0f));
    float currentPanelX = std::round(x - slideDist * (1.0f - easeOutCubic));

    float paddingVertical = std::round(dp(16.0f));
    float effectiveScrollY = std::round(m_scrollY);
    float currentY = y + paddingVertical - effectiveScrollY;
    float totalContentHeight = paddingVertical;

    for (View* child : children) {
        float childHeight = std::round(dp(child->getPreferredHeight()));
        child->doLayout(currentPanelX, currentY, panelW, childHeight);
        currentY += childHeight;
        totalContentHeight += childHeight;
    }

    totalContentHeight += paddingVertical;

    m_maxScrollY = std::max(0.0f, std::round(totalContentHeight - height));
    m_targetScrollY = std::clamp(std::round(m_targetScrollY), 0.0f, m_maxScrollY);
}

bool NavigationDrawer::handleMouseMove(float mx, float my) {
    if (m_slideAnim <= 0.001f) return false;
    ViewGroup::handleMouseMove(mx, my); 
    return true;
}

bool NavigationDrawer::handleMouseButton(int button, int action, float mx, float my) {
    if (m_slideAnim <= 0.001f) return false;
    
    float easeOutCubic = 1.0f - std::pow(1.0f - m_slideAnim, 3.0f);
    float panelW = std::min(std::round(dp(m_panelWidthBase)), width - std::round(dp(48.0f)));
    float slideDist = (m_animStyle == DrawerAnimStyle::Slide) ? panelW : std::round(dp(64.0f));
    float currentPanelX = std::round(x - slideDist * (1.0f - easeOutCubic));

    if (mx > currentPanelX + panelW) { 
        if (action == 1) close(); 
        return true; 
    }

    ViewGroup::handleMouseButton(button, action, mx, my);
    return true;
}

bool NavigationDrawer::handleScroll(float mx, float my, float ox, float oy) {
    if (m_slideAnim <= 0.001f) return false;

    float easeOutCubic = 1.0f - std::pow(1.0f - m_slideAnim, 3.0f);
    float panelW = std::min(std::round(dp(m_panelWidthBase)), width - std::round(dp(48.0f)));
    float slideDist = (m_animStyle == DrawerAnimStyle::Slide) ? panelW : std::round(dp(64.0f));
    float currentPanelX = std::round(x - slideDist * (1.0f - easeOutCubic));

    if (mx < currentPanelX || mx > currentPanelX + panelW) {
        return false;
    }

    bool handled = ViewGroup::handleScroll(mx, my, ox, oy);

    if (!handled && m_maxScrollY > 0.0f) {
        m_targetScrollY -= oy * std::round(dp(48.0f));
        m_targetScrollY = std::clamp(std::round(m_targetScrollY), 0.0f, m_maxScrollY);
        requestUIWakeup(0.016);
    }

    return true;
}

void NavigationDrawer::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (m_slideAnim <= 0.001f) return;

    GLFWwindow* win = glfwGetCurrentContext();
    int fbW = 0, fbH = 0;
    if (win) glfwGetFramebufferSize(win, &fbW, &fbH);
    if (fbH <= 0) return;

    float easeOutCubic = 1.0f - std::pow(1.0f - m_slideAnim, 3.0f);
    float panelW = std::min(std::round(dp(m_panelWidthBase)), width - std::round(dp(48.0f)));
    float slideDist = (m_animStyle == DrawerAnimStyle::Slide) ? panelW : std::round(dp(64.0f));
    float currentPanelX = std::round(x - slideDist * (1.0f - easeOutCubic));

    // 1. Scrim overlay
    M3Color scrimColor = { 0.0f, 0.0f, 0.0f, 0.32f * m_slideAnim };
    renderer.drawM3UI(x, y, width, height, 0.0f, 0.0f, 0.0f, 0.0f, scrimColor);

    // 2. Drawer background panel
    float bgFade = (m_animStyle == DrawerAnimStyle::SharedAxis) ? std::pow(m_slideAnim, 1.8f) : 1.0f;
    float bgRadius = std::round(dp(16.0f));
    M3Color surfaceColor = theme.surfaceContainerLow;
    surfaceColor.a *= bgFade;
    renderer.drawM3UI(currentPanelX, y, panelW, height, 0.0f, bgRadius, bgRadius, 0.0f, surfaceColor);

    // 3. Main scissor clip for drawer content
    glEnable(GL_SCISSOR_TEST);
    glScissor(
        (GLint)std::max(0.0f, currentPanelX),
        (GLint)std::round((float)fbH - y - height),
        (GLint)panelW,
        (GLint)height
    );

    // 4. Viewport culling (skip children outside drawer bounds)
    float viewTop = y;
    float viewBottom = y + height;

    for (View* child : children) {
        if (child->y + child->height < viewTop || child->y > viewBottom) {
            continue;
        }
        child->render(renderer, theme);
    }

    glDisable(GL_SCISSOR_TEST);
}
