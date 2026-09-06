/**
 * @file NavigationDrawer.cpp
 * @brief Implementation of sliding modal panel with expandable/collapsible navigation sections.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
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

extern void requestUIWakeup(double seconds);

/**
 * @brief Calculates intersection with existing OpenGL scissor box and applies scissor clipping.
 * 
 * @param x Desired scissor left coordinate in window pixels.
 * @param y Desired scissor bottom coordinate in window pixels.
 * @param w Desired scissor width in window pixels.
 * @param h Desired scissor height in window pixels.
 * @param prevScissor Existing 4-element scissor rectangle array.
 * @param scissorEnabled Flag indicating whether scissor test was previously active.
 */
static void setIntersectedScissor(int x, int y, int w, int h, const GLint prevScissor[4], GLboolean scissorEnabled) {
    if (!scissorEnabled) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y, std::max(0, w), std::max(0, h));
        return;
    }

    int px = prevScissor[0];
    int py = prevScissor[1];
    int pw = prevScissor[2];
    int ph = prevScissor[3];

    int nx = std::max(x, px);
    int ny = std::max(y, py);
    int nr = std::min(x + w, px + pw);
    int nt = std::min(y + h, py + ph);

    int nw = std::max(0, nr - nx);
    int nh = std::max(0, nt - ny);

    glEnable(GL_SCISSOR_TEST);
    glScissor(nx, ny, nw, nh);
}

/**
 * @brief Constructs a DrawerItem instance.
 * 
 * @param id Unique item identifier.
 * @param label Display text label.
 * @param iconStr Icon identifier string or asset path.
 * @param parent Pointer to parent drawer instance.
 * @param indentLevel Hierarchy nesting indentation level.
 */
DrawerItem::DrawerItem(int id, const std::string& label, const std::string& iconStr, NavigationDrawer* parent, int indentLevel)
    : m_id(id), m_label(label), m_icon(iconFromString(iconStr)), m_parent(parent), m_indentLevel(indentLevel) {
    layout_width = MATCH_PARENT;
}

/**
 * @brief Sets selection state for this item.
 * 
 * @param selected True to select, false otherwise.
 * @param instant True to skip animations.
 */
void DrawerItem::setSelected(bool selected, bool instant) { 
    m_selected = selected; 
}

/**
 * @brief Updates item state transitions.
 * 
 * @param dt Delta time in seconds.
 */
void DrawerItem::update(float dt) { 
    View::update(dt); 
}

/**
 * @brief Invokes item selection on parent drawer when clicked.
 */
void DrawerItem::onClick() { 
    if (m_parent) m_parent->onItemClicked(m_id); 
}

/**
 * @brief Renders the item background pill, state overlays, leading icon, and text.
 * 
 * @param renderer Reference to the Material UI shader renderer.
 * @param theme Reference to the active Material theme tokens.
 */
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
    
    if (m_selected) {
        M3Color activeBg = theme.secondaryContainer; 
        activeBg.a *= masterAlpha;
        renderer.drawM3UI(pillX, pillY, pillW, pillH, radius, radius, radius, radius, activeBg);
    }
    
    if (stateLayerAlpha > 0.001f) {
        M3Color hoverColor = m_selected ? theme.onSecondaryContainer : theme.onSurfaceVariant;
        hoverColor.a = stateLayerAlpha * masterAlpha;
        renderer.drawM3UI(pillX, pillY, pillW, pillH, radius, radius, radius, radius, hoverColor);
    }

    float iconSize = std::floor(dp(22.0f) * 0.5f) * 2.0f; 
    float iconY = centerY - iconSize * 0.5f;
    float iconX = std::round(pillX + dp(14.0f));

    M3Color contentColor = m_selected ? theme.onSecondaryContainer : theme.onSurfaceVariant;
    contentColor.a *= masterAlpha;

    if (!isIconEmpty(m_icon)) { 
        renderer.drawIcon(iconX, iconY, iconSize, m_icon, contentColor); 
    }
    
    float textX = std::round(iconX + iconSize + dp(10.0f)); 
    float textSize = dp(14.0f);
    float textDrawY = std::round(centerY - textSize * 0.5f);
    
    renderer.drawText(m_label, textX, textDrawY, textSize, contentColor);
}

/**
 * @brief Constructs a DrawerHeader instance.
 * 
 * @param title Header title string.
 * @param parent Pointer to parent drawer instance.
 */
DrawerHeader::DrawerHeader(const std::string& title, NavigationDrawer* parent) 
    : m_title(title), m_parent(parent) { 
    layout_width = MATCH_PARENT; 
}

/**
 * @brief Renders the section header text.
 * 
 * @param renderer Reference to the Material UI shader renderer.
 * @param theme Reference to the active Material theme tokens.
 */
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

/**
 * @brief Constructs a DrawerDivider instance.
 * 
 * @param parent Pointer to parent drawer instance.
 */
DrawerDivider::DrawerDivider(NavigationDrawer* parent) : m_parent(parent) {
    layout_width = MATCH_PARENT;
}

/**
 * @brief Renders the horizontal separator line.
 * 
 * @param renderer Reference to the Material UI shader renderer.
 * @param theme Reference to the active Material theme tokens.
 */
void DrawerDivider::render(MaterialShader& renderer, MaterialTheme& theme) {
    float masterAlpha = m_parent ? m_parent->getMasterAlpha() : 1.0f;
    M3Color color = theme.outlineVariant; 
    color.a = 0.5f * masterAlpha;
    float padding = std::round(dp(28.0f));
    float centerY = std::round(y + height * 0.5f);
    renderer.drawM3UI(std::round(x + padding), centerY, std::round(width - padding * 2.0f), std::round(dp(1.0f)), 0.0f, 0.0f, 0.0f, 0.0f, color);
}

/**
 * @brief Constructs a DrawerExpandableGroup instance.
 * 
 * @param title Group title label.
 * @param iconStr Optional icon resource identifier or asset path.
 * @param defaultExpanded Initial expansion state.
 * @param drawer Pointer to the root NavigationDrawer.
 * @param indentLevel Indentation nesting level.
 */
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

/**
 * @brief Adds a child navigation item into this group.
 * 
 * @param id Unique item identifier.
 * @param label Display text label.
 * @param iconStr Icon identifier string or asset path.
 */
void DrawerExpandableGroup::addItem(int id, const std::string& label, const std::string& iconStr) {
    addView(new DrawerItem(id, label, iconStr, m_drawer, m_indentLevel + 1));
}

/**
 * @brief Adds a nested expandable child sub-group into this group.
 * 
 * @param title Sub-group title label.
 * @param defaultExpanded Initial expansion state for the sub-group.
 * @param iconStr Optional icon identifier string or asset path.
 * @return Pointer to the newly created DrawerExpandableGroup instance.
 */
DrawerExpandableGroup* DrawerExpandableGroup::addGroup(const std::string& title, bool defaultExpanded, const std::string& iconStr) {
    auto* subGroup = new DrawerExpandableGroup(title, iconStr, defaultExpanded, m_drawer, m_indentLevel + 1);
    addView(subGroup);
    return subGroup;
}

/**
 * @brief Sets the expansion state and initiates animation progression.
 * 
 * @param expanded True to expand, false to collapse.
 */
void DrawerExpandableGroup::setExpanded(bool expanded) {
    if (m_isExpanded == expanded) return;
    m_isExpanded = expanded;
    
    m_startAnim = m_expandAnim;
    m_animProgress = 0.0f;
    requestUIWakeup(0.35);
}

/**
 * @brief Computes preferred height dynamically depending on child heights and expansion factor.
 * 
 * @return Preferred height in dp.
 */
float DrawerExpandableGroup::getPreferredHeight() {
    float headerH = 48.0f;
    float childrenH = 0.0f;
    for (View* child : children) {
        childrenH += child->getPreferredHeight();
    }
    return headerH + childrenH * m_expandAnim;
}

/**
 * @brief Updates expansion cubic easing interpolation.
 * 
 * @param dt Delta time in seconds.
 */
void DrawerExpandableGroup::update(float dt) {
    ViewGroup::update(dt);

    if (m_animProgress < 1.0f) {
        float dtSafe = std::min(dt, 0.033f);
        const float DURATION = 0.28f;

        m_animProgress += dtSafe / DURATION;
        
        if (m_animProgress >= 1.0f) {
            m_animProgress = 1.0f;
            m_expandAnim = m_isExpanded ? 1.0f : 0.0f;
        } else {
            float t = m_animProgress;
            float ease = 1.0f - std::pow(1.0f - t, 3.0f);
            float target = m_isExpanded ? 1.0f : 0.0f;
            
            m_expandAnim = m_startAnim + (target - m_startAnim) * ease;
            requestUIWakeup(0.05);
        }
    }
}

/**
 * @brief Resolves layout bounds for group header and child items.
 * 
 * @param parentX Parent origin X coordinate in pixels.
 * @param parentY Parent origin Y coordinate in pixels.
 * @param parentW Parent allocated width in pixels.
 * @param parentH Parent allocated height in pixels.
 */
void DrawerExpandableGroup::doLayout(float parentX, float parentY, float parentW, float parentH) {
    x = std::round(parentX);
    y = std::round(parentY);
    width = std::round(parentW);
    height = std::round(parentH);

    float headerH = std::round(dp(48.0f));
    float curY = y + headerH;

    for (View* child : children) {
        float chH = std::round(dp(child->getPreferredHeight()));
        child->doLayout(x, curY, width, chH);
        curY += chH;
    }
}

/**
 * @brief Handles mouse movement events.
 * 
 * @param mx Mouse X coordinate.
 * @param my Mouse Y coordinate.
 * @return True if the mouse cursor is inside the bounds.
 */
bool DrawerExpandableGroup::handleMouseMove(float mx, float my) {
    bool inside = isInside(mx, my);
    if (m_expandAnim > 0.05f) {
        ViewGroup::handleMouseMove(mx, my);
    }
    return inside;
}

/**
 * @brief Handles mouse button events on header or children.
 * 
 * @param button Mouse button index.
 * @param action Action type.
 * @param mx Mouse X coordinate.
 * @param my Mouse Y coordinate.
 * @return True if event was consumed.
 */
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
            return ViewGroup::handleMouseButton(button, action, mx, my);
        }
    }
    return false;
}

/**
 * @brief Renders the group header, rotation chevron, and scissor-clipped children.
 * 
 * @param renderer Reference to the Material UI shader renderer.
 * @param theme Reference to the active Material theme tokens.
 */
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

    if (hoverAnim > 0.01f) {
        M3Color hoverBg = theme.onSurfaceVariant;
        hoverBg.a = hoverAnim * 0.06f * masterAlpha;
        renderer.drawM3UI(pillX, pillY, pillW, pillH, radius, radius, radius, radius, hoverBg);
    }

    float contentX = std::round(pillX + dp(14.0f));
    if (m_hasIcon) {
        float iconSize = std::floor(dp(20.0f) * 0.5f) * 2.0f;
        M3Color iconColor = theme.onSurfaceVariant; 
        iconColor.a *= masterAlpha;
        renderer.drawIcon(contentX, centerY - iconSize * 0.5f, iconSize, m_icon, iconColor);
        contentX += iconSize + std::round(dp(10.0f));
    }

    M3Color titleColor = theme.onSurface;
    titleColor.a *= (0.85f * masterAlpha);
    
    float textSize = dp(14.0f);
    float textY = std::round(centerY - textSize * 0.5f);
    renderer.drawText(m_title, contentX, textY, textSize, titleColor);

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

    float clipTop = y + headerH;
    float clipH = std::max(0.0f, height - headerH);

    if (m_expandAnim > 0.005f && clipH >= 1.0f) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        GLint winHeight = viewport[3];

        GLint prevScissor[4];
        GLboolean prevScissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
        if (prevScissorEnabled) {
            glGetIntegerv(GL_SCISSOR_BOX, prevScissor);
        }

        setIntersectedScissor(
            (int)std::round(x), 
            (int)std::round(winHeight - clipTop - clipH), 
            (int)std::round(width), 
            (int)std::round(clipH), 
            prevScissor, 
            prevScissorEnabled
        );

        for (View* child : children) {
            child->render(renderer, theme);
        }

        if (prevScissorEnabled) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(prevScissor[0], prevScissor[1], prevScissor[2], prevScissor[3]);
        } else {
            glDisable(GL_SCISSOR_TEST);
        }
    }
}

/**
 * @brief Constructs a NavigationDrawer instance.
 */
NavigationDrawer::NavigationDrawer() { 
    layout_width = MATCH_PARENT; 
    layout_height = MATCH_PARENT; 
}

/**
 * @brief Opens the drawer with slide animation.
 */
void NavigationDrawer::open() { m_isOpen = true; requestUIWakeup(0.5); }

/**
 * @brief Closes the drawer with slide animation.
 */
void NavigationDrawer::close() { m_isOpen = false; requestUIWakeup(0.5); }

/**
 * @brief Toggles the drawer open state.
 */
void NavigationDrawer::toggle() { m_isOpen = !m_isOpen; requestUIWakeup(0.5); }

/**
 * @brief Appends a top-level navigation item.
 * 
 * @param id Unique item identifier.
 * @param label Display text label.
 * @param iconStr Icon resource identifier or asset path.
 */
void NavigationDrawer::addItem(int id, const std::string& label, const std::string& iconStr) {
    DrawerItem* item = new DrawerItem(id, label, iconStr, this, 0);
    if (children.empty()) { 
        item->setSelected(true, true); 
        m_selectedIndex = id; 
    }
    addView(item);
}

/**
 * @brief Appends a category section header.
 * 
 * @param title Header title text.
 */
void NavigationDrawer::addHeader(const std::string& title) { 
    addView(new DrawerHeader(title, this)); 
}

/**
 * @brief Appends a visual horizontal divider separator.
 */
void NavigationDrawer::addDivider() { 
    addView(new DrawerDivider(this)); 
}

/**
 * @brief Appends an expandable collapsible group.
 * 
 * @param title Group title label.
 * @param defaultExpanded Initial expansion state.
 * @param iconStr Optional icon identifier string or asset path.
 * @return Pointer to created DrawerExpandableGroup instance.
 */
DrawerExpandableGroup* NavigationDrawer::addGroup(const std::string& title, bool defaultExpanded, const std::string& iconStr) {
    auto* group = new DrawerExpandableGroup(title, iconStr, defaultExpanded, this, 0);
    addView(group);
    return group;
}

/**
 * @brief Sets callback handler for item selection events.
 * 
 * @param callback Callback receiving selected item ID.
 */
void NavigationDrawer::setOnItemSelected(std::function<void(int)> callback) { 
    m_onItemSelectedCallback = std::move(callback); 
}

/**
 * @brief Recursively traverses view hierarchy to update selection flags.
 * 
 * @param views Vector of child view pointers.
 * @param targetId Target item identifier to be selected.
 */
static void updateDrawerSelection(const std::vector<View*>& views, int targetId) {
    for (View* v : views) {
        if (auto* item = dynamic_cast<DrawerItem*>(v)) {
            item->setSelected(item->getId() == targetId);
        } else if (auto* group = dynamic_cast<DrawerExpandableGroup*>(v)) {
            updateDrawerSelection(group->children, targetId);
        }
    }
}

/**
 * @brief Updates selected item index and refreshes hierarchy state.
 * 
 * @param id Selected item identifier.
 */
void NavigationDrawer::setSelectedIndex(int id) {
    if (m_selectedIndex == id) return;

    updateDrawerSelection(children, id);
    m_selectedIndex = id;
    if (m_onItemSelectedCallback) m_onItemSelectedCallback(id);
}

/**
 * @brief Handles item selection event and automatically closes the drawer.
 * 
 * @param id Selected item identifier.
 */
void NavigationDrawer::onItemClicked(int id) { 
    setSelectedIndex(id); 
    close(); 
}

/**
 * @brief Calculates overall master alpha for current animation frame.
 * 
 * @return Opacity factor in range [0.0, 1.0].
 */
float NavigationDrawer::getMasterAlpha() const {
    if (m_animStyle == DrawerAnimStyle::SharedAxis) return std::pow(m_slideAnim, 2.0f);
    return 1.0f;
}

/**
 * @brief Updates panel slide progression and kinetic scroll smoothing.
 * 
 * @param dt Delta time in seconds.
 */
void NavigationDrawer::update(float dt) {
    ViewGroup::update(dt);
    float dtSafe = std::min(dt, 0.033f);
    float animSpeed = 4.5f;

    if (m_isOpen) {
        m_slideAnim += animSpeed * dtSafe;
        if (m_slideAnim > 1.0f) m_slideAnim = 1.0f;
    }
    else {
        m_slideAnim -= animSpeed * dtSafe;
        if (m_slideAnim < 0.0f) m_slideAnim = 0.0f;
    }

    if (m_slideAnim > 0.0f && m_slideAnim < 1.0f) {
        requestUIWakeup(0.05);
    }

    float scrollDiff = m_targetScrollY - m_scrollY;
    if (std::abs(scrollDiff) >= 0.5f) {
        float factor = 1.0f - std::exp(-22.0f * dtSafe);
        m_scrollY += scrollDiff * factor;
        requestUIWakeup(0.05);
    } else {
        m_scrollY = m_targetScrollY;
    }
}

/**
 * @brief Resolves panel sliding offset, scroll metrics, and children layout.
 * 
 * @param parentX Parent origin X coordinate in pixels.
 * @param parentY Parent origin Y coordinate in pixels.
 * @param parentW Parent allocated width in pixels.
 * @param parentH Parent allocated height in pixels.
 */
void NavigationDrawer::doLayout(float parentX, float parentY, float parentW, float parentH) {
    x = std::round(parentX); 
    y = std::round(parentY); 
    width = std::round(parentW); 
    height = std::round(parentH);

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

/**
 * @brief Processes mouse movement events across drawer panel.
 * 
 * @param mx Mouse X coordinate in window space.
 * @param my Mouse Y coordinate in window space.
 * @return True if event is consumed.
 */
bool NavigationDrawer::handleMouseMove(float mx, float my) {
    if (m_slideAnim <= 0.001f) return false;
    ViewGroup::handleMouseMove(mx, my); 
    return true;
}

/**
 * @brief Processes mouse button clicks on drawer content or outside scrim.
 * 
 * @param button Mouse button index.
 * @param action Action type.
 * @param mx Mouse X coordinate in window space.
 * @param my Mouse Y coordinate in window space.
 * @return True if event is consumed.
 */
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

/**
 * @brief Processes mouse wheel scrolling events inside the drawer panel.
 * 
 * @param mx Mouse X coordinate.
 * @param my Mouse Y coordinate.
 * @param ox Horizontal scroll offset.
 * @param oy Vertical scroll offset.
 * @return True if scroll event was consumed.
 */
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
        requestUIWakeup(0.05);
    }

    return true;
}

/**
 * @brief Renders the backdrop scrim overlay, drawer container surface, and child views.
 * 
 * @param renderer Reference to the Material UI shader renderer.
 * @param theme Reference to the active Material theme tokens.
 */
void NavigationDrawer::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (m_slideAnim <= 0.001f) return;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    GLint winHeight = viewport[3];

    float easeOutCubic = 1.0f - std::pow(1.0f - m_slideAnim, 3.0f);
    float panelW = std::min(std::round(dp(m_panelWidthBase)), width - std::round(dp(48.0f)));
    float slideDist = (m_animStyle == DrawerAnimStyle::Slide) ? panelW : std::round(dp(64.0f));
    float currentPanelX = std::round(x - slideDist * (1.0f - easeOutCubic));

    M3Color scrimColor = { 0.0f, 0.0f, 0.0f, 0.32f * m_slideAnim };
    renderer.drawM3UI(x, y, width, height, 0.0f, 0.0f, 0.0f, 0.0f, scrimColor);

    float bgFade = (m_animStyle == DrawerAnimStyle::SharedAxis) ? std::pow(m_slideAnim, 1.8f) : 1.0f;
    float bgRadius = std::round(dp(16.0f));
    M3Color surfaceColor = theme.surfaceContainerLow;
    surfaceColor.a *= bgFade;
    renderer.drawM3UI(currentPanelX, y, panelW, height, 0.0f, bgRadius, bgRadius, 0.0f, surfaceColor);

    glEnable(GL_SCISSOR_TEST);
    glScissor(
        (GLint)std::max(0.0f, currentPanelX),
        (GLint)std::round(winHeight - y - height),
        (GLint)panelW,
        (GLint)height
    );

    ViewGroup::render(renderer, theme);

    glDisable(GL_SCISSOR_TEST);
}