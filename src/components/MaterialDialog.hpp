/**
 * @file MaterialDialog.hpp
 * @brief Material Design 3 modal dialog component and transition animation definitions.
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
#include <string>
#include <functional>
#include <vector>
#include <sstream>

/**
 * @enum DialogAnimationStyle
 * @brief Transition animation styles for modal dialog appearance and dismissal.
 */
enum class DialogAnimationStyle {
    Scale,   /**< Expands from the center point of the screen. */
    SlideUp, /**< Slides upward from the bottom of the screen with a fade-in. */
    FadeZoom /**< Smooth asymptotic scale zoom coupled with opacity fading. */
};

/**
 * @class DialogButton
 * @brief Action button rendered within the bottom action bar of a MaterialDialog.
 * 
 * Supports hover/click state layers and primary style coloring.
 */
class DialogButton : public View {
public:
    std::string text;             /**< Display label text. */
    bool isPrimary;               /**< Indicates if this button represents the primary confirmation action. */
    std::function<void()> action; /**< Callback action executed upon clicking. */

    float m_dialogAlpha = 1.0f;   /**< Current opacity factor synchronized with the parent dialog animation. */

    /**
     * @brief Constructs a dialog button with label, style flag, and action callback.
     * @param text Button label.
     * @param isPrimary True for primary (accent) style.
     * @param action Callback invoked on click.
     */
    DialogButton(const std::string& text, bool isPrimary, std::function<void()> action);

    float getPreferredWidth() override { return dp(80.0f); }
    float getPreferredHeight() override { return dp(40.0f); }
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
    void onClick() override;
};

/**
 * @class MaterialDialog
 * @brief Modal dialog overlay container rendering a semi-transparent scrim, elevated card, and action buttons.
 * 
 * Features include configurable animation styles, scrollable content with a custom scrollbar,
 * and automatic word‑wrapping for multi‑line text with support for CJK and emoji characters.
 */
class MaterialDialog : public ViewGroup {
private:
    std::string m_title;          /**< Dialog title text. */
    std::string m_content;        /**< Dialog content text (supports newline characters). */
    std::vector<std::string> m_wrappedLines; /**< Cached wrapped lines after word‑break layout. */

    float m_animProgress = 0.0f;  /**< Animation progress [0..1]. */
    bool m_isOpen = false;        /**< Dialog open state flag. */

    DialogAnimationStyle m_animStyle = DialogAnimationStyle::FadeZoom; /**< Current animation style. */

    DialogButton* m_btnCancel = nullptr;  /**< Cancel button (optional). */
    DialogButton* m_btnConfirm = nullptr; /**< Confirm button (optional). */

    float m_cardX = 0.0f;         /**< Dialog card X position. */
    float m_cardY = 0.0f;         /**< Dialog card Y position. */
    float m_cardW = 0.0f;         /**< Dialog card width. */
    float m_cardH = 0.0f;         /**< Dialog card height. */

    // --- Smooth scrolling and scrollbar state ---
    float m_targetScrollY = 0.0f;          /**< Target vertical scroll offset (dp). */
    float m_currentScrollY = 0.0f;         /**< Current interpolated scroll offset (dp). */
    float m_contentViewportH = 0.0f;       /**< Height of the content viewport (visible area) in dp. */
    float m_totalContentH = 0.0f;          /**< Total height of all wrapped content lines in dp. */
    float m_lastLayoutW = 0.0f;            /**< Cached card width used for layout validation. */
    bool m_textDirty = true;               /**< Flag indicating content has changed and needs re‑wrapping. */

    // --- Thumb and content drag state ---
    bool m_isThumbDragging = false;        /**< Whether the scrollbar thumb is being dragged. */
    bool m_isContentDragging = false;      /**< Whether the content area is being dragged. */
    float m_dragStartY = 0.0f;             /**< Mouse Y position at drag start. */
    float m_dragStartScrollY = 0.0f;       /**< Scroll position at drag start. */

    /**
     * @brief Performs word‑wrapping of content text into lines fitting within the given width.
     * @param maxTextWidth Maximum line width in dp.
     */
    void updateWrapLines(float maxTextWidth);

public:
    /**
     * @brief Constructs a MaterialDialog with a title and content.
     * @param title Dialog title.
     * @param content Dialog content (plain text, supports newlines).
     */
    MaterialDialog(const std::string& title, const std::string& content);
    virtual ~MaterialDialog() override = default;

    /**
     * @brief Sets the dialog title.
     * @param title New title string.
     */
    void setTitle(const std::string& title) { m_title = title; }

    /**
     * @brief Sets the dialog content and marks layout as dirty.
     * @param content New content string.
     */
    void setContent(const std::string& content) { 
        m_content = content; 
        m_textDirty = true; 
        m_targetScrollY = 0.0f;
        m_currentScrollY = 0.0f;
    }

    /**
     * @brief Sets the animation style used for opening and closing.
     * @param style Desired style.
     */
    void setAnimationStyle(DialogAnimationStyle style) { m_animStyle = style; }

    /**
     * @brief Configures the two action buttons (cancel and confirm).
     * @param cancelText Label for cancel button.
     * @param onCancel Callback for cancel action.
     * @param confirmText Label for confirm button.
     * @param onConfirm Callback for confirm action.
     */
    void setActions(const std::string& cancelText, std::function<void()> onCancel,
                    const std::string& confirmText, std::function<void()> onConfirm);

    /** @brief Opens the dialog (starts show animation). */
    void show();

    /** @brief Closes the dialog (starts dismiss animation). */
    void dismiss();

    /**
     * @brief Checks if the dialog is currently visible (animation progress > threshold).
     * @return True if the dialog is rendered.
     */
    bool isVisible() const { return m_animProgress > 0.001f; }

    // --- View overrides ---
    void update(float dt) override;
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;

    /**
     * @brief Handles scroll wheel input (float version).
     * @param xoffset Horizontal scroll delta (ignored).
     * @param yoffset Vertical scroll delta.
     * @return True if the event was consumed.
     */
    bool handleScroll(float xoffset, float yoffset);

    /**
     * @brief Handles scroll wheel input (double version, for GLFW compatibility).
     * @param xoffset Horizontal scroll delta.
     * @param yoffset Vertical scroll delta.
     * @return True if the event was consumed.
     */
    bool handleScroll(double xoffset, double yoffset) { return handleScroll((float)xoffset, (float)yoffset); }

    /**
     * @brief Overloaded scroll handler that ignores mouse position.
     * @param mx Mouse X (unused).
     * @param my Mouse Y (unused).
     * @param xoffset Horizontal delta.
     * @param yoffset Vertical delta.
     * @return True if consumed.
     */
    bool handleScroll(float mx, float my, float xoffset, float yoffset) { (void)mx; (void)my; return handleScroll(xoffset, yoffset); }
};
