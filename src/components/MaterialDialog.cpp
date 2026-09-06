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
#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>

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
}

void MaterialDialog::dismiss() { 
    m_isOpen = false; 
}

void MaterialDialog::update(float dt) {
    ViewGroup::update(dt);
    float speed = (m_isOpen ? 6.0f : 8.0f) * dt;
    if (m_isOpen) m_animProgress = std::min(1.0f, m_animProgress + speed);
    else m_animProgress = std::max(0.0f, m_animProgress - speed);

    for (auto* child : children) {
        if (DialogButton* btn = dynamic_cast<DialogButton*>(child)) {
            btn->m_dialogAlpha = m_animProgress;
        }
    }
}

void MaterialDialog::doLayout(float parentX, float parentY, float parentW, float parentH) {
    x = parentX; 
    y = parentY; 
    width = parentW; 
    height = parentH;
    m_cardW = std::clamp(parentW * 0.8f, dp(280.0f), dp(560.0f));
    m_wrappedLines.clear();

    float availableTextWidth = m_cardW - dp(48.0f);
    float approxCharWidth = dp(14.0f) * 0.55f;
    int maxCharsPerLine = std::max(10, (int)(availableTextWidth / approxCharWidth));

    std::istringstream words(m_content);
    std::string word, currentLine;
    while (words >> word) {
        if (currentLine.empty()) {
            currentLine = word;
        } else if (currentLine.length() + 1 + word.length() <= static_cast<size_t>(maxCharsPerLine)) {
            currentLine += " " + word;
        } else { 
            m_wrappedLines.push_back(currentLine); 
            currentLine = word; 
        }
    }
    if (!currentLine.empty()) m_wrappedLines.push_back(currentLine);

    m_cardH = dp(80.0f) + (m_wrappedLines.size() * dp(22.0f)) + dp(64.0f);
    m_cardX = x + (width - m_cardW) / 2.0f;
    m_cardY = y + (height - m_cardH) / 2.0f;

    if (children.size() >= 2) {
        float btnY = m_cardY + m_cardH - dp(48.0f);
        m_btnCancel->doLayout(m_cardX + m_cardW - dp(180.0f), btnY, dp(80.0f), dp(40.0f));
        m_btnConfirm->doLayout(m_cardX + m_cardW - dp(90.0f), btnY, dp(80.0f), dp(40.0f));
    }
}

bool MaterialDialog::handleMouseMove(float mx, float my) {
    if (m_animProgress < 0.01f) return false;
    ViewGroup::handleMouseMove(mx, my);
    return true;
}

bool MaterialDialog::handleMouseButton(int button, int action, float mx, float my) {
    if (m_animProgress < 0.01f) return false;
    bool hitChild = ViewGroup::handleMouseButton(button, action, mx, my);
    if (!hitChild && action == 1) {
        if (mx < m_cardX || mx > m_cardX + m_cardW || my < m_cardY || my > m_cardY + m_cardH) {
            dismiss();
        }
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
    else {
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

    renderer.drawText(m_title, currentX + dp(24.0f) * textScale, currentY + dp(24.0f) * textScale, dp(22.0f), titleCol);

    float textY = currentY + dp(68.0f) * textScale;
    for (size_t i = 0; i < m_wrappedLines.size(); ++i) {
        renderer.drawText(m_wrappedLines[i], currentX + dp(24.0f) * textScale, textY, dp(14.0f), bodyCol);
        textY += dp(22.0f) * textScale;
    }

    if (!children.empty()) {
        ViewGroup::render(renderer, theme);
        for (size_t i = 0; i < children.size(); ++i) {
            children[i]->x = btnBaseX[i];
            children[i]->y = btnBaseY[i];
        }
    }
}