/**
 * @file MaterialDialog.cpp
 * @brief Implementation of modal dialog rendering, text word-wrapping, scrim overlays, and transition animations.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "MaterialDialog.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>
#include <sstream>

extern void requestUIWakeup(double seconds);

/**
 * @struct ScissorGuard
 * @brief RAII guard that safely manages OpenGL scissor state, intersecting with parent scissor and restoring on destruction.
 * 
 * Ensures rendering is clipped to the desired rectangle while respecting any existing scissor restrictions,
 * and automatically restores the previous state when the guard goes out of scope.
 */
struct ScissorGuard {
    GLboolean wasEnabled = GL_FALSE; /**< Saved enabled state of GL_SCISSOR_TEST. */
    GLint prevBox[4] = { 0, 0, 0, 0 }; /**< Saved scissor box [x, y, width, height]. */

    /**
     * @brief Constructs the guard and sets the scissor box to the intersection of the given rect and the parent scissor.
     * @param x Left coordinate (screen space).
     * @param y Bottom coordinate (screen space).
     * @param w Width.
     * @param h Height.
     */
    ScissorGuard(int x, int y, int w, int h) {
        wasEnabled = glIsEnabled(GL_SCISSOR_TEST);
        glGetIntegerv(GL_SCISSOR_BOX, prevBox);

        if (w < 0) w = 0;
        if (h < 0) h = 0;

        int finalX = x;
        int finalY = y;
        int finalW = w;
        int finalH = h;

        if (wasEnabled) {
            int pX1 = prevBox[0];
            int pY1 = prevBox[1];
            int pX2 = prevBox[0] + prevBox[2];
            int pY2 = prevBox[1] + prevBox[3];

            int cX1 = x;
            int cY1 = y;
            int cX2 = x + w;
            int cY2 = y + h;

            int iX1 = std::max(pX1, cX1);
            int iY1 = std::max(pY1, cY1);
            int iX2 = std::min(pX2, cX2);
            int iY2 = std::min(pY2, cY2);

            finalX = iX1;
            finalY = iY1;
            finalW = std::max(0, iX2 - iX1);
            finalH = std::max(0, iY2 - iY1);
        }

        glEnable(GL_SCISSOR_TEST);
        glScissor(finalX, finalY, finalW, finalH);
    }

    /**
     * @brief Destructor restores the original scissor state (enable/disable and box).
     */
    ~ScissorGuard() {
        if (wasEnabled) {
            glScissor(prevBox[0], prevBox[1], prevBox[2], prevBox[3]);
        } else {
            glDisable(GL_SCISSOR_TEST);
        }
    }
};

// --- DialogButton implementation ---

DialogButton::DialogButton(const std::string& text, bool isPrimary, std::function<void()> action)
    : text(text), isPrimary(isPrimary), action(std::move(action)) {
}

void DialogButton::onClick() { 
    if (action) action(); 
}

void DialogButton::render(MaterialShader& renderer, MaterialTheme& theme) {
    float pX = std::round(x);
    float pY = std::round(y);
    float pW = std::round(width);
    float pH = std::round(height);
    float radius = pH * 0.5f;

    float stateLayerAlpha = hoverAnim * 0.08f + pressAnim * 0.12f;

    M3Color contentColor = isPrimary ? theme.primary : theme.onSurfaceVariant;
    contentColor.a *= m_dialogAlpha;

    if (stateLayerAlpha > 0.001f) {
        M3Color hoverColor = contentColor;
        hoverColor.a = stateLayerAlpha * m_dialogAlpha;
        renderer.drawM3UI(pX, pY, pW, pH, radius, radius, radius, radius, hoverColor);
    }

    float fontSize = dp(14.0f);
    float textW = renderer.getTextWidth(text, fontSize);
    float textX = std::round(pX + (pW - textW) * 0.5f);
    float textY = std::round(pY + (pH - fontSize) * 0.5f);

    renderer.drawText(text, textX, textY, fontSize, contentColor);
}

// --- MaterialDialog implementation ---

MaterialDialog::MaterialDialog(const std::string& title, const std::string& content)
    : m_title(title), m_content(content) {
    layout_width = MATCH_PARENT; 
    layout_height = MATCH_PARENT;
}

void MaterialDialog::setActions(const std::string& cancelText, std::function<void()> onCancel,
                                const std::string& confirmText, std::function<void()> onConfirm) {
    m_btnCancel = new DialogButton(cancelText, false, [this, onCancel]() { 
        if (onCancel) onCancel(); 
        dismiss(); 
    });
    m_btnConfirm = new DialogButton(confirmText, true, [this, onConfirm]() { 
        if (onConfirm) onConfirm(); 
        dismiss(); 
    });
    addView(m_btnCancel); 
    addView(m_btnConfirm);
}

void MaterialDialog::show() { 
    m_isOpen = true; 
    m_targetScrollY = 0.0f;
    m_currentScrollY = 0.0f;
    m_isThumbDragging = false;
    m_isContentDragging = false;
    requestUIWakeup(0.1);
}

void MaterialDialog::dismiss() { 
    m_isOpen = false; 
    m_isThumbDragging = false;
    m_isContentDragging = false;
    requestUIWakeup(0.1);
}

void MaterialDialog::update(float dt) {
    ViewGroup::update(dt);
    float dtSafe = std::min(dt, 0.033f);
    bool needFrame = false;

    // 1. Dialog open/close animation
    float speed = (m_isOpen ? 6.0f : 8.0f) * dtSafe;
    if (m_isOpen) {
        if (m_animProgress < 1.0f) {
            m_animProgress = std::min(1.0f, m_animProgress + speed);
            needFrame = true;
        }
    } else {
        if (m_animProgress > 0.0f) {
            m_animProgress = std::max(0.0f, m_animProgress - speed);
            needFrame = true;
        }
    }

    // 2. Inertial smooth damping scrolling
    if (std::abs(m_targetScrollY - m_currentScrollY) > 0.2f) {
        m_currentScrollY += (m_targetScrollY - m_currentScrollY) * (1.0f - std::exp(-22.0f * dtSafe));
        needFrame = true;
    } else {
        m_currentScrollY = m_targetScrollY;
    }

    for (auto* child : children) {
        if (DialogButton* btn = dynamic_cast<DialogButton*>(child)) {
            btn->m_dialogAlpha = m_animProgress;
        }
    }

    if (needFrame) {
        requestUIWakeup(0.1);
    }
}

/**
 * @brief High-performance UTF-8 adaptive word-wrapping algorithm supporting CJK characters, long word truncation, and paragraph handling.
 * 
 * Measures character widths using heuristics: Latin/numbers ~0.55×fontSize, CJK/emojis ~1.05×fontSize.
 * Lines are broken at spaces or when the accumulated width exceeds the given maximum.
 */
void MaterialDialog::updateWrapLines(float maxTextWidth) {
    m_wrappedLines.clear();
    if (m_content.empty() || maxTextWidth <= 0.0f) return;

    float fontSize = dp(14.0f);
    float latinCharWidth = fontSize * 0.55f; // Approximate width for Latin letters/digits
    float cjkCharWidth = fontSize * 1.05f;    // Approximate width for CJK/emoji characters

    std::istringstream stream(m_content);
    std::string paragraph;

    while (std::getline(stream, paragraph)) {
        if (paragraph.empty()) {
            m_wrappedLines.push_back("");
            continue;
        }

        std::string currentLine;
        float currentLineWidth = 0.0f;

        for (size_t i = 0; i < paragraph.length(); ) {
            unsigned char c = static_cast<unsigned char>(paragraph[i]);
            size_t charLen = 1;
            float charW = latinCharWidth;

            // UTF-8 multi-byte character detection (CJK / Emoji)
            if (c >= 0x80) {
                if ((c & 0xE0) == 0xC0) charLen = 2;
                else if ((c & 0xF0) == 0xE0) charLen = 3;
                else if ((c & 0xF8) == 0xF0) charLen = 4;
                charW = cjkCharWidth;
            } else {
                if (c == '\t') charW = latinCharWidth * 4;
            }

            if (i + charLen > paragraph.length()) break;
            std::string ch = paragraph.substr(i, charLen);
            i += charLen;

            // Handle inline line breaks
            if (ch == "\r") continue;
            if (ch == "\n") {
                m_wrappedLines.push_back(currentLine);
                currentLine.clear();
                currentLineWidth = 0.0f;
                continue;
            }

            // Force line break if width exceeds limit
            if (currentLineWidth + charW > maxTextWidth) {
                if (!currentLine.empty()) {
                    m_wrappedLines.push_back(currentLine);
                    currentLine.clear();
                    currentLineWidth = 0.0f;
                }
            }

            currentLine += ch;
            currentLineWidth += charW;
        }

        if (!currentLine.empty()) {
            m_wrappedLines.push_back(currentLine);
        }
    }

    m_totalContentH = m_wrappedLines.size() * dp(22.0f);
}

void MaterialDialog::doLayout(float parentX, float parentY, float parentW, float parentH) {
    x = parentX; 
    y = parentY; 
    width = parentW; 
    height = parentH;
    m_cardW = std::clamp(parentW * 0.82f, dp(280.0f), dp(560.0f));

    // Content available width: 24dp margins on each side, plus 12dp scrollbar spacing on the right.
    float contentAvailableW = m_cardW - dp(24.0f) * 2.0f - dp(12.0f);

    if (m_textDirty || std::abs(m_lastLayoutW - m_cardW) > 1.0f) {
        m_lastLayoutW = m_cardW;
        m_textDirty = false;
        updateWrapLines(contentAvailableW);
    }

    float maxAllowedH = parentH * 0.82f;
    float desiredH = dp(76.0f) + m_totalContentH + dp(64.0f);
    m_cardH = std::clamp(desiredH, dp(160.0f), maxAllowedH);

    m_cardX = x + (width - m_cardW) / 2.0f;
    m_cardY = y + (height - m_cardH) / 2.0f;

    // Content viewport height (title area ~68dp, action bar ~56dp)
    m_contentViewportH = m_cardH - dp(68.0f) - dp(56.0f);
    if (m_contentViewportH < dp(30.0f)) m_contentViewportH = dp(30.0f);

    float maxScroll = std::max(0.0f, m_totalContentH - m_contentViewportH);
    m_targetScrollY = std::clamp(m_targetScrollY, 0.0f, maxScroll);
    m_currentScrollY = std::clamp(m_currentScrollY, 0.0f, maxScroll);

    if (m_btnCancel && m_btnConfirm) {
        float btnY = m_cardY + m_cardH - dp(48.0f);
        m_btnCancel->doLayout(m_cardX + m_cardW - dp(180.0f), btnY, dp(80.0f), dp(40.0f));
        m_btnConfirm->doLayout(m_cardX + m_cardW - dp(90.0f), btnY, dp(80.0f), dp(40.0f));
    }
}

bool MaterialDialog::handleMouseMove(float mx, float my) {
    if (m_animProgress < 0.01f) return false;

    float maxScroll = std::max(0.0f, m_totalContentH - m_contentViewportH);

    // 1. Handle scrollbar thumb dragging
    if (m_isThumbDragging && maxScroll > 0.0f) {
        float contentTopY = m_cardY + dp(68.0f);
        float trackH = m_contentViewportH;
        float thumbH = std::max(dp(28.0f), (m_contentViewportH / m_totalContentH) * trackH);
        float travelH = trackH - thumbH;

        if (travelH > 0.0f) {
            float deltaY = my - m_dragStartY;
            float scrollDelta = (deltaY / travelH) * maxScroll;
            m_targetScrollY = std::clamp(m_dragStartScrollY + scrollDelta, 0.0f, maxScroll);
            m_currentScrollY = m_targetScrollY;
            requestUIWakeup(0.1);
        }
        return true;
    }

    // 2. Handle content area dragging
    if (m_isContentDragging && maxScroll > 0.0f) {
        float deltaY = my - m_dragStartY;
        m_targetScrollY = std::clamp(m_dragStartScrollY - deltaY, 0.0f, maxScroll);
        m_currentScrollY = m_targetScrollY;
        requestUIWakeup(0.1);
        return true;
    }

    ViewGroup::handleMouseMove(mx, my);
    return true;
}

bool MaterialDialog::handleMouseButton(int button, int action, float mx, float my) {
    if (m_animProgress < 0.01f) return false;

    bool hitChild = ViewGroup::handleMouseButton(button, action, mx, my);

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            float contentTopY = m_cardY + dp(68.0f);
            float contentBottomY = m_cardY + m_cardH - dp(56.0f);
            float maxScroll = std::max(0.0f, m_totalContentH - m_contentViewportH);

            // Precise scrollbar hit detection (with 18dp hot zone)
            if (maxScroll > 0.0f) {
                float scrollbarW = dp(6.0f);
                float thumbX = m_cardX + m_cardW - dp(10.0f) - scrollbarW;
                float trackH = m_contentViewportH;
                float thumbH = std::max(dp(28.0f), (m_contentViewportH / m_totalContentH) * trackH);
                float scrollRatio = (maxScroll > 0.0f) ? (m_targetScrollY / maxScroll) : 0.0f;
                float thumbY = contentTopY + scrollRatio * (trackH - thumbH);

                if (mx >= thumbX - dp(10.0f) && mx <= thumbX + scrollbarW + dp(10.0f) &&
                    my >= contentTopY && my <= contentBottomY) {
                    
                    if (my < thumbY) {
                        m_targetScrollY = std::clamp(m_targetScrollY - m_contentViewportH * 0.8f, 0.0f, maxScroll);
                    } else if (my > thumbY + thumbH) {
                        m_targetScrollY = std::clamp(m_targetScrollY + m_contentViewportH * 0.8f, 0.0f, maxScroll);
                    }

                    m_isThumbDragging = true;
                    m_dragStartY = my;
                    m_dragStartScrollY = m_targetScrollY;
                    requestUIWakeup(0.1);
                    return true;
                }
            }

            // Content area drag detection
            bool inContentArea = (mx >= m_cardX + dp(24.0f) && mx <= m_cardX + m_cardW - dp(24.0f) &&
                                  my >= contentTopY && my <= contentBottomY);

            if (inContentArea && maxScroll > 0.0f && !hitChild) {
                m_isContentDragging = true;
                m_dragStartY = my;
                m_dragStartScrollY = m_targetScrollY;
                return true;
            }

            // Dismiss if clicking outside the card area (on the scrim)
            if (!hitChild) {
                if (mx < m_cardX || mx > m_cardX + m_cardW || my < m_cardY || my > m_cardY + m_cardH) {
                    dismiss();
                }
            }
        }
        else if (action == GLFW_RELEASE) {
            m_isThumbDragging = false;
            m_isContentDragging = false;
        }
    }
    return true;
}

bool MaterialDialog::handleScroll(float xoffset, float yoffset) {
    if (m_animProgress < 0.01f) return false;

    float maxScroll = std::max(0.0f, m_totalContentH - m_contentViewportH);
    if (maxScroll > 0.0f) {
        m_targetScrollY -= yoffset * dp(44.0f);
        m_targetScrollY = std::clamp(m_targetScrollY, 0.0f, maxScroll);
        requestUIWakeup(0.1);
    }
    return true;
}

void MaterialDialog::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (m_animProgress < 0.01f) return;

    float alpha = m_animProgress;
    M3Color scrim = { 0.0f, 0.0f, 0.0f, 0.32f * alpha };
    renderer.drawM3UI(x, y, width, height, 0.0f, 0.0f, 0.0f, 0.0f, scrim);

    float currentW = m_cardW, currentH = m_cardH, currentX = m_cardX, currentY = m_cardY;
    float radius = dp(28.0f);
    float textScale = 1.0f; 

    // Save original button positions for animation
    std::vector<float> btnBaseX(children.size()), btnBaseY(children.size());
    for (size_t i = 0; i < children.size(); ++i) {
        btnBaseX[i] = children[i]->x; 
        btnBaseY[i] = children[i]->y;
    }

    if (m_animStyle == DialogAnimationStyle::Scale) {
        float scale = (m_animProgress < 1.0f) ? (1.0f - std::pow(1.0f - m_animProgress, 3.0f)) : 1.0f;
        currentW = m_cardW * scale; 
        currentH = m_cardH * scale;
        currentX = m_cardX + (m_cardW - currentW) / 2.0f; 
        currentY = m_cardY + (m_cardH - currentH) / 2.0f;
        radius *= scale; 
        textScale = scale;

        for (size_t i = 0; i < children.size(); ++i) {
            children[i]->x = currentX + (btnBaseX[i] - m_cardX) * scale;
            children[i]->y = currentY + (btnBaseY[i] - m_cardY) * scale;
        }
    }
    else if (m_animStyle == DialogAnimationStyle::FadeZoom) {
        float ease = 1.0f - std::pow(1.0f - m_animProgress, 4.0f);
        float scale = 0.85f + 0.15f * ease; 

        currentW = m_cardW * scale; 
        currentH = m_cardH * scale;
        currentX = m_cardX + (m_cardW - currentW) / 2.0f; 
        currentY = m_cardY + (m_cardH - currentH) / 2.0f;
        radius *= scale; 
        textScale = scale;

        for (size_t i = 0; i < children.size(); ++i) {
            children[i]->x = currentX + (btnBaseX[i] - m_cardX) * scale;
            children[i]->y = currentY + (btnBaseY[i] - m_cardY) * scale;
        }
    }
    else { // SlideUp
        float ease = 1.0f - std::pow(1.0f - m_animProgress, 4.0f);
        float offsetY = dp(24.0f) * (1.0f - ease);
        currentY = m_cardY + offsetY;

        for (size_t i = 0; i < children.size(); ++i) {
            children[i]->y += offsetY;
        }
    }

    M3Color surfaceColor = theme.surfaceContainerHigh; 
    surfaceColor.a *= alpha;
    renderer.drawM3UI(currentX, currentY, currentW, currentH, radius, radius, radius, radius, surfaceColor);

    M3Color titleCol = theme.onSurface; 
    titleCol.a *= alpha;
    M3Color bodyCol = theme.onSurfaceVariant; 
    bodyCol.a *= alpha;

    // 1. Render title
    renderer.drawText(m_title, currentX + dp(24.0f) * textScale, currentY + dp(24.0f) * textScale, dp(22.0f) * textScale, titleCol);

    // 2. Render content with scissor clipping
    float contentTopY = currentY + dp(68.0f) * textScale;
    float contentAreaH = (m_cardH - dp(68.0f) - dp(56.0f)) * textScale;
    float contentLeftX = currentX + dp(24.0f) * textScale;
    float contentRightLimit = currentX + currentW - dp(24.0f) * textScale;
    float contentAreaW = contentRightLimit - contentLeftX;

    if (contentAreaH > 0.0f && !m_wrappedLines.empty()) {
        GLFWwindow* win = glfwGetCurrentContext();
        int fbW = 0, fbH = 0;
        if (win) glfwGetFramebufferSize(win, &fbW, &fbH);

        int scissorX = static_cast<int>(std::round(contentLeftX));
        int scissorY = fbH - static_cast<int>(std::round(contentTopY + contentAreaH));
        int scissorW = static_cast<int>(std::round(contentAreaW));
        int scissorH = static_cast<int>(std::round(contentAreaH));

        ScissorGuard guard(scissorX, scissorY, scissorW, scissorH);

        float lineStep = dp(22.0f) * textScale;
        float bodyFontSize = dp(14.0f) * textScale;

        // O(1) viewport virtualization: render only the visible lines
        int startLine = static_cast<int>(std::floor(m_currentScrollY / dp(22.0f)));
        int visibleCount = static_cast<int>(std::ceil(m_contentViewportH / dp(22.0f))) + 2;
        int endLine = std::min(static_cast<int>(m_wrappedLines.size()) - 1, startLine + visibleCount);

        if (startLine < 0) startLine = 0;

        for (int i = startLine; i <= endLine; ++i) {
            if (m_wrappedLines[i].empty()) continue;
            float lineY = contentTopY + i * lineStep - m_currentScrollY * textScale;
            renderer.drawText(m_wrappedLines[i], contentLeftX, lineY, bodyFontSize, bodyCol);
        }
    }

    // 3. Draw Material 3 floating rounded scrollbar
    float maxScroll = std::max(0.0f, m_totalContentH - m_contentViewportH);
    if (maxScroll > 0.0f && contentAreaH > 0.0f) {
        float scrollbarW = dp(4.0f);
        float trackH = contentAreaH;
        float thumbH = std::max(dp(28.0f), (m_contentViewportH / m_totalContentH) * trackH);
        float scrollRatio = (maxScroll > 0.0f) ? (m_currentScrollY / maxScroll) : 0.0f;
        float thumbY = contentTopY + scrollRatio * (trackH - thumbH);
        float thumbX = currentX + currentW - dp(10.0f) - scrollbarW;
        float thumbRadius = scrollbarW * 0.5f;

        M3Color thumbColor = m_isThumbDragging ? theme.primary : theme.onSurfaceVariant;
        thumbColor.a = (m_isThumbDragging ? 0.85f : 0.40f) * alpha;
        renderer.drawM3UI(thumbX, thumbY, scrollbarW, thumbH, thumbRadius, thumbRadius, thumbRadius, thumbRadius, thumbColor);
    }

    // 4. Render bottom action buttons
    if (!children.empty()) {
        ViewGroup::render(renderer, theme);
        // Restore original button positions (they were temporarily modified for animation)
        for (size_t i = 0; i < children.size(); ++i) {
            children[i]->x = btnBaseX[i];
            children[i]->y = btnBaseY[i];
        }
    }
}
