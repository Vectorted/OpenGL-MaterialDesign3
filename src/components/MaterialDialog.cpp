/**
 * @file MaterialDialog.cpp
 * @brief Implementation of MaterialDialog with scrolling, wrapping, and animation.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "MaterialDialog.hpp"
#include <GLFW/glfw3.h>

namespace {
    /**
     * @brief Creates a dialog‑specific theme by applying an alpha to the surface container.
     * @param theme Base theme.
     * @param alpha Opacity factor [0..1].
     * @return Modified theme.
     */
    static MaterialTheme createDialogTheme(const MaterialTheme& theme, float alpha) {
        MaterialTheme t = theme;
        t.surface = theme.surfaceContainerHigh;
        return t.withAlpha(alpha);
    }

    /**
     * @struct ScissorGuard
     * @brief RAII guard that safely intersects the current scissor box with a new region.
     * 
     * Enables scissor test, calculates intersection with any existing scissor,
     * and restores the previous state on destruction.
     */
    struct ScissorGuard {
        GLboolean wasEnabled;
        GLint lastScissor[4];

        /**
         * @brief Constructs the guard and sets the scissor box to the intersection.
         * @param x Left coordinate (screen space).
         * @param y Bottom coordinate (screen space).
         * @param w Width.
         * @param h Height.
         */
        ScissorGuard(int x, int y, int w, int h) {
            wasEnabled = glIsEnabled(GL_SCISSOR_TEST);
            glGetIntegerv(GL_SCISSOR_BOX, lastScissor);
            glEnable(GL_SCISSOR_TEST);

            if (wasEnabled) {
                int nx = std::max(x, lastScissor[0]);
                int ny = std::max(y, lastScissor[1]);
                int nr = std::min(x + w, lastScissor[0] + lastScissor[2]);
                int nt = std::min(y + h, lastScissor[1] + lastScissor[3]);
                glScissor(nx, ny, std::max(0, nr - nx), std::max(0, nt - ny));
            } else {
                glScissor(x, y, std::max(0, w), std::max(0, h));
            }
        }

        /** @brief Restores the original scissor state. */
        ~ScissorGuard() {
            if (wasEnabled) {
                glScissor(lastScissor[0], lastScissor[1], lastScissor[2], lastScissor[3]);
            } else {
                glDisable(GL_SCISSOR_TEST);
            }
        }
    };
}

DialogButton::DialogButton(const std::string& text, DialogButtonStyle style)
    : m_text(text), m_style(style) {
    setLayoutParams(MATCH_PARENT, MATCH_PARENT);
}

void DialogButton::doLayout(float parentX, float parentY, float parentW, float parentH) {
    x = parentX;
    y = parentY;
    width = parentW;
    height = parentH;
}

void DialogButton::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (!isVisible() || m_parentAlpha <= 0.001f) return;

    float r = dp(20.0f);
    M3Color bg = { 0.0f, 0.0f, 0.0f, 0.0f };
    M3Color textCol;

    if (m_style == DialogButtonStyle::Filled) {
        bg = theme.primary;
        textCol = theme.onPrimary;
        if (hoverAnim > 0.001f) {
            bg.r = std::min(1.0f, bg.r + 0.08f * hoverAnim);
            bg.g = std::min(1.0f, bg.g + 0.08f * hoverAnim);
            bg.b = std::min(1.0f, bg.b + 0.08f * hoverAnim);
        }
    } else {
        bg = theme.primary;
        bg.a = 0.0f;
        textCol = theme.primary;
        if (hoverAnim > 0.001f) bg.a = 0.08f * hoverAnim;
    }

    bg.a *= m_parentAlpha;
    textCol.a *= m_parentAlpha;

    if (bg.a > 0.001f) {
        renderer.drawM3UI(std::round(x), std::round(y), std::round(width), std::round(height), r, r, r, r, bg);
    }

    float fontSize = dp(14.0f);
    float textW = renderer.getTextWidth(m_text, fontSize);
    float textX = x + (width - textW) * 0.5f;
    float textY = y + (height - fontSize) * 0.5f;
    renderer.drawText(m_text, std::round(textX), std::round(textY), fontSize, textCol);
}

MaterialDialog::MaterialDialog(const std::string& title, const std::string& content)
    : m_title(title), m_content(content) {
    setVisibility(Visibility::Gone);
}

void MaterialDialog::setContent(const std::string& content) { 
    m_content = content; 
    m_textDirty = true; 
}

void MaterialDialog::setCustomView(View* view) {
    if (m_customView && m_customView != view) {
        removeView(m_customView);
    }
    m_customView = view;
    if (m_customView) {
        addView(m_customView);
    }
}

void MaterialDialog::setActions(const std::string& cancelText, std::function<void()> onCancel,
                                const std::string& confirmText, std::function<void()> onConfirm) {
    if (!cancelText.empty()) {
        if (!m_btnCancel) {
            m_btnCancel = new DialogButton(cancelText, DialogButtonStyle::Text);
            addView(m_btnCancel);
        } else {
            m_btnCancel->setText(cancelText);
        }
        m_btnCancel->setOnClick([this, onCancel]() {
            if (onCancel) onCancel();
            dismiss();
        });
    }

    if (!confirmText.empty()) {
        if (!m_btnConfirm) {
            m_btnConfirm = new DialogButton(confirmText, DialogButtonStyle::Filled);
            addView(m_btnConfirm);
        } else {
            m_btnConfirm->setText(confirmText);
        }
        m_btnConfirm->setOnClick([this, onConfirm]() {
            if (onConfirm) onConfirm();
            dismiss();
        });
    }
}

void MaterialDialog::show() {
    setVisibility(Visibility::Visible);
    m_isOpen = true;
    m_targetScrollY = 0.0f;
    m_currentScrollY = 0.0f;
    m_isPressedInsideCard = false;
}

void MaterialDialog::dismiss() {
    m_isOpen = false;
}

void MaterialDialog::update(float dt) {
    if (isGone()) return;

    ViewGroup::update(dt);

    if (m_isOpen) {
        float diff = 1.0f - m_animProgress;
        if (diff > 0.001f) {
            m_animProgress += diff * (1.0f - std::exp(-22.0f * dt));
        } else {
            m_animProgress = 1.0f;
        }
    } else {
        float diff = 0.0f - m_animProgress;
        m_animProgress += diff * (1.0f - std::exp(-30.0f * dt));
        if (m_animProgress <= 0.02f) {
            m_animProgress = 0.0f;
            setVisibility(Visibility::Gone);
        }
    }

    if (std::abs(m_currentScrollY - m_targetScrollY) > 0.1f) {
        m_currentScrollY += (m_targetScrollY - m_currentScrollY) * (1.0f - std::exp(-20.0f * dt));
    } else {
        m_currentScrollY = m_targetScrollY;
    }
}

void MaterialDialog::doLayout(float parentX, float parentY, float parentW, float parentH) {
    x = parentX; 
    y = parentY; 
    width = parentW; 
    height = parentH;

    // 1. Determine card width: prefer custom, then layout_width, then custom view’s preferred, then fallback.
    float targetCardW = 0.0f;
    if (m_customCardW > 0.0f) {
        targetCardW = dp(m_customCardW);
    } else if (layout_width > 0.0f && layout_width != MATCH_PARENT) {
        targetCardW = dp(layout_width);
    } else if (m_customView && m_customView->getPreferredWidth() > 0.0f) {
        targetCardW = m_customView->getPreferredWidth() + dp(48.0f);
    } else {
        targetCardW = parentW * 0.82f;
    }
    m_cardW = std::clamp(targetCardW, dp(240.0f), std::max(dp(240.0f), parentW - dp(32.0f)));

    float contentAvailableW = m_cardW - dp(24.0f) * 2.0f;
    if (!m_customView && std::abs(m_lastLayoutW - m_cardW) > 1.0f) {
        m_lastLayoutW = m_cardW;
        m_textDirty = true;
    }

    float headerH = m_title.empty() ? dp(24.0f) : dp(76.0f);
    float actionsH = (m_btnCancel || m_btnConfirm) ? dp(88.0f) : dp(24.0f);

    // 2. Determine card height: prefer custom, then layout_height, else compute from content.
    float targetCardH = 0.0f;
    if (m_customCardH > 0.0f) {
        targetCardH = dp(m_customCardH);
    } else if (layout_height > 0.0f && layout_height != MATCH_PARENT) {
        targetCardH = dp(layout_height);
    } else {
        float bodyH = 0.0f;
        if (m_customView) {
            bodyH = m_customView->getPreferredHeight();
            if (bodyH <= 0.0f) bodyH = dp(240.0f);
        } else {
            bodyH = m_totalContentH;
        }
        targetCardH = headerH + bodyH + actionsH;
    }

    float maxAllowedH = std::max(dp(160.0f), parentH - dp(40.0f));
    m_cardH = std::clamp(targetCardH, dp(140.0f), maxAllowedH);

    m_cardX = x + (width - m_cardW) / 2.0f;
    m_cardY = y + (height - m_cardH) / 2.0f;

    m_contentViewportH = m_cardH - headerH - actionsH;
    if (m_contentViewportH < dp(30.0f)) m_contentViewportH = dp(30.0f);

    if (m_customView) {
        m_customView->doLayout(m_cardX + dp(24.0f), m_cardY + headerH, contentAvailableW, m_contentViewportH);
    } else {
        float maxScroll = std::max(0.0f, m_totalContentH - m_contentViewportH);
        m_targetScrollY = std::clamp(m_targetScrollY, 0.0f, maxScroll);
        m_currentScrollY = std::clamp(m_currentScrollY, 0.0f, maxScroll);
    }
}

void MaterialDialog::updateWrapLines(MaterialShader& renderer, float maxTextWidth) {
    m_wrappedLines.clear();
    if (m_content.empty() || maxTextWidth <= 0.0f) return;

    const float fontSize = dp(14.0f);
    std::istringstream stream(m_content);
    std::string paragraph;

    while (std::getline(stream, paragraph)) {
        if (paragraph.empty()) {
            m_wrappedLines.push_back("");
            continue;
        }

        std::string curLine = "";
        for (size_t i = 0; i < paragraph.length(); ) {
            unsigned char c = static_cast<unsigned char>(paragraph[i]);
            if (c == '\r') { i++; continue; }

            size_t cLen = 1;
            if (c >= 0x80) {
                if ((c & 0xE0) == 0xC0) cLen = 2;
                else if ((c & 0xF0) == 0xE0) cLen = 3;
                else if ((c & 0xF8) == 0xF0) cLen = 4;
            }

            if (i + cLen > paragraph.length()) break;
            std::string ch = paragraph.substr(i, cLen);
            i += cLen;

            if (ch == "\n") {
                m_wrappedLines.push_back(curLine);
                curLine.clear();
                continue;
            }

            std::string testLine = curLine + ch;
            if (renderer.getTextWidth(testLine, fontSize) > maxTextWidth) {
                if (!curLine.empty()) {
                    m_wrappedLines.push_back(curLine);
                    curLine.clear();
                }
                if (ch != " ") curLine = ch;
            } else {
                curLine = testLine;
            }
        }
        if (!curLine.empty()) m_wrappedLines.push_back(curLine);
    }
    m_totalContentH = m_wrappedLines.size() * dp(22.0f);
}

void MaterialDialog::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (isGone() || m_animProgress < 0.001f) return;

    float contentAvailableW = m_cardW - dp(24.0f) * 2.0f;
    if (!m_customView && m_textDirty) {
        m_textDirty = false;
        updateWrapLines(renderer, contentAvailableW);
    }

    float alpha = m_isOpen ? m_animProgress : (m_animProgress * m_animProgress);
    MaterialTheme dialogTheme = createDialogTheme(theme, alpha);

    // 1. Full‑screen scrim
    M3Color scrim = { 0.0f, 0.0f, 0.0f, 0.32f * alpha };
    renderer.drawM3UI(x, y, width, height, 0.0f, 0.0f, 0.0f, 0.0f, scrim);

    // 2. Animation scaling
    float scale = 1.0f;
    if (m_animStyle == DialogAnimationStyle::Scale) {
        float ease = (m_animProgress < 1.0f) ? (1.0f - std::pow(1.0f - m_animProgress, 3.0f)) : 1.0f;
        scale = 0.88f + 0.12f * ease;
    } else if (m_animStyle == DialogAnimationStyle::FadeZoom) {
        float ease = 1.0f - std::pow(1.0f - m_animProgress, 4.0f);
        scale = 0.94f + 0.06f * ease;
    }

    float currentW = std::round(m_cardW * scale);
    float currentH = std::round(m_cardH * scale);
    float currentX = std::round(m_cardX + (m_cardW - currentW) * 0.5f);
    float currentY = std::round(m_cardY + (m_cardH - currentH) * 0.5f);
    float radius = dp(28.0f) * scale;

    // 3. Card background
    renderer.drawM3UI(currentX, currentY, currentW, currentH, radius, radius, radius, radius, dialogTheme.surface);

    // 4. Title
    float headerH = m_title.empty() ? dp(24.0f) : dp(76.0f);
    if (!m_title.empty()) {
        renderer.drawText(m_title, std::round(currentX + dp(24.0f)), std::round(currentY + dp(28.0f)), dp(22.0f), dialogTheme.onSurface);
    }

    // 5. Custom view or wrapped text with scissor clipping
    if (m_customView && m_customView->isVisible()) {
        float cvX = currentX + dp(24.0f);
        float cvY = currentY + headerH;
        m_customView->doLayout(cvX, cvY, contentAvailableW, m_contentViewportH);

        GLFWwindow* win = glfwGetCurrentContext();
        int fbW = 0, fbH = 0;
        if (win) glfwGetFramebufferSize(win, &fbW, &fbH);

        int scissorX = static_cast<int>(std::round(currentX + dp(8.0f)));
        int scissorY = fbH - static_cast<int>(std::round(currentY + currentH - dp(8.0f)));
        int scissorW = static_cast<int>(std::round(currentW - dp(16.0f)));
        int scissorH = static_cast<int>(std::round(currentH - dp(16.0f)));

        if (scissorW > 0 && scissorH > 0) {
            ScissorGuard guard(scissorX, scissorY, scissorW, scissorH);
            m_customView->render(renderer, dialogTheme);
        }
    } else if (!m_customView) {
        float contentTopY = currentY + headerH;
        float contentLeftX = currentX + dp(24.0f);

        if (m_contentViewportH > 0.0f && !m_wrappedLines.empty()) {
            GLFWwindow* win = glfwGetCurrentContext();
            int fbW = 0, fbH = 0;
            if (win) glfwGetFramebufferSize(win, &fbW, &fbH);

            int scissorX = static_cast<int>(std::round(contentLeftX));
            int scissorY = fbH - static_cast<int>(std::round(contentTopY + m_contentViewportH));
            int scissorW = static_cast<int>(std::round(contentAvailableW));
            int scissorH = static_cast<int>(std::round(m_contentViewportH));

            ScissorGuard guard(scissorX, scissorY, scissorW, scissorH);

            float lineStep = dp(22.0f);
            float bodyFontSize = dp(14.0f);
            int startLine = std::max(0, static_cast<int>(std::floor(m_currentScrollY / lineStep)));
            int visibleCount = static_cast<int>(std::ceil(m_contentViewportH / lineStep)) + 2;
            int endLine = std::min(static_cast<int>(m_wrappedLines.size()) - 1, startLine + visibleCount);

            for (int i = startLine; i <= endLine; ++i) {
                if (m_wrappedLines[i].empty()) continue;
                float lineY = contentTopY + i * lineStep - m_currentScrollY;
                renderer.drawText(m_wrappedLines[i], std::round(contentLeftX), std::round(lineY), bodyFontSize, dialogTheme.onSurfaceVariant);
            }
        }
    }

    // 6. Bottom action buttons
    float btnH = dp(40.0f);
    float btnY = currentY + currentH - dp(24.0f) - btnH;
    float minBtnW = dp(80.0f);
    float cancelW = minBtnW, confirmW = minBtnW;

    if (m_btnCancel && !m_btnCancel->getText().empty()) {
        cancelW = std::max(minBtnW, dp(11.0f) * m_btnCancel->getText().length() + dp(32.0f));
    }
    if (m_btnConfirm && !m_btnConfirm->getText().empty()) {
        confirmW = std::max(minBtnW, dp(11.0f) * m_btnConfirm->getText().length() + dp(32.0f));
    }

    float rightMargin = dp(24.0f);
    float gap = dp(12.0f);

    if (m_btnCancel && m_btnConfirm) {
        float confirmX = currentX + currentW - rightMargin - confirmW;
        float cancelX = confirmX - gap - cancelW;
        m_btnCancel->doLayout(cancelX, btnY, cancelW, btnH);
        m_btnConfirm->doLayout(confirmX, btnY, confirmW, btnH);
    } else if (m_btnConfirm) {
        float confirmX = currentX + currentW - rightMargin - confirmW;
        m_btnConfirm->doLayout(confirmX, btnY, confirmW, btnH);
    } else if (m_btnCancel) {
        float cancelX = currentX + currentW - rightMargin - cancelW;
        m_btnCancel->doLayout(cancelX, btnY, cancelW, btnH);
    }

    if (m_btnCancel && m_btnCancel->isVisible()) {
        m_btnCancel->setParentAlpha(alpha);
        m_btnCancel->render(renderer, theme);
    }
    if (m_btnConfirm && m_btnConfirm->isVisible()) {
        m_btnConfirm->setParentAlpha(alpha);
        m_btnConfirm->render(renderer, theme);
    }
}

bool MaterialDialog::handleMouseMove(float mx, float my) {
    if (isGone() || m_animProgress < 0.1f) return false;

    if (!m_customView) {
        float maxScroll = std::max(0.0f, m_totalContentH - m_contentViewportH);
        if (m_isContentDragging && maxScroll > 0.0f) {
            float deltaY = my - m_dragStartY;
            m_targetScrollY = std::clamp(m_dragStartScrollY - deltaY, 0.0f, maxScroll);
            m_currentScrollY = m_targetScrollY;
            return true;
        }
    }

    ViewGroup::handleMouseMove(mx, my);
    return true; // Modal full‑screen barrier
}

bool MaterialDialog::handleMouseButton(int button, int action, float mx, float my) {
    if (isGone() || m_animProgress < 0.2f || !m_isOpen) return false;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            m_isPressedInsideCard = (mx >= m_cardX && mx <= m_cardX + m_cardW &&
                                     my >= m_cardY && my <= m_cardY + m_cardH);

            ViewGroup::handleMouseButton(button, action, mx, my);
            return true;
        } 
        else if (action == GLFW_RELEASE) {
            bool hitChild = ViewGroup::handleMouseButton(button, action, mx, my);

            bool insideCard = (mx >= m_cardX && mx <= m_cardX + m_cardW &&
                               my >= m_cardY && my <= m_cardY + m_cardH);

            if (!m_isPressedInsideCard && !insideCard && !hitChild) {
                dismiss();
            }
            m_isPressedInsideCard = false;
            return true;
        }
    }

    ViewGroup::handleMouseButton(button, action, mx, my);
    return true;
}

bool MaterialDialog::handleScroll(float mx, float my, float ox, float oy) {
    if (isGone() || m_animProgress < 0.1f) return false;

    if (m_customView) {
        ViewGroup::handleScroll(mx, my, ox, oy);
    } else {
        float maxScroll = std::max(0.0f, m_totalContentH - m_contentViewportH);
        if (maxScroll > 0.0f) {
            m_targetScrollY -= oy * dp(44.0f);
            m_targetScrollY = std::clamp(m_targetScrollY, 0.0f, maxScroll);
        }
    }
    return true;
}
