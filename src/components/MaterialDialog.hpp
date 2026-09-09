/**
 * @file MaterialDialog.hpp
 * @brief Material Design 3 modal dialog with custom content, scrollable body, and action buttons.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include "View.hpp"
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <cmath>
#include <sstream>

/**
 * @enum DialogAnimationStyle
 * @brief Transition animation styles for dialog appearance and dismissal.
 */
enum class DialogAnimationStyle {
    FadeZoom,   /**< Material 3 classic elastic fade‑in with subtle zoom. */
    SlideUp,    /**< Slides upward from the bottom edge. */
    Scale       /**< Pure center‑origin scaling. */
};

/**
 * @enum DialogButtonStyle
 * @brief Visual prominence levels for dialog action buttons.
 */
enum class DialogButtonStyle {
    Text,   /**< Low‑emphasis text‑only button (e.g., Cancel). */
    Filled  /**< High‑emphasis filled button (e.g., Confirm). */
};

/**
 * @class DialogButton
 * @brief Action button used within MaterialDialog’s bottom button bar.
 * 
 * Supports both filled and text styles, hover effects, and parent alpha blending.
 */
class DialogButton : public View {
private:
    std::string m_text;                     /**< Button label. */
    DialogButtonStyle m_style;              /**< Visual style (Text or Filled). */
    std::function<void()> m_onClick;        /**< Callback invoked on button press. */
    float m_parentAlpha = 1.0f;             /**< Inherited opacity from the dialog. */

public:
    /**
     * @brief Constructs a dialog button.
     * @param text Button label.
     * @param style Visual style (defaults to Text).
     */
    DialogButton(const std::string& text, DialogButtonStyle style = DialogButtonStyle::Text);

    /** @brief Updates the button label. */
    void setText(const std::string& text) { m_text = text; }

    /** @brief Returns the current label. */
    const std::string& getText() const { return m_text; }

    /** @brief Sets the click callback. */
    void setOnClick(std::function<void()> cb) { m_onClick = cb; }

    /** @brief Triggers the click callback (called by dialog). */
    void onClick() override { if (m_onClick) m_onClick(); }

    /** @brief Sets the parent dialog’s alpha for blending. */
    void setParentAlpha(float a) { m_parentAlpha = a; }

    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
};

/**
 * @class MaterialDialog
 * @brief Modal dialog container with scrollable content, action buttons, and animation support.
 * 
 * Supports custom Views, automatic text word‑wrapping, scrolling with momentum,
 * and dismiss on outside‑click. Follows Material Design 3 guidelines.
 */
class MaterialDialog : public ViewGroup {
private:
    std::string m_title;                          /**< Dialog title text. */
    std::string m_content;                        /**< Body text (supports newlines). */
    std::vector<std::string> m_wrappedLines;      /**< Cached wrapped lines for scrollable body. */

    View* m_customView = nullptr;                 /**< Optional custom view replacing body text. */

    float m_animProgress = 0.0f;                  /**< Animation progress [0..1]. */
    bool m_isOpen = false;                        /**< Dialog open state. */
    DialogAnimationStyle m_animStyle = DialogAnimationStyle::FadeZoom; /**< Current animation style. */

    DialogButton* m_btnCancel = nullptr;          /**< Cancel button (optional). */
    DialogButton* m_btnConfirm = nullptr;         /**< Confirm button (optional). */

    // Custom dimensions (negative means auto‑sized)
    float m_customCardW = -1.0f;                  /**< Explicit card width in dp, or -1 for auto. */
    float m_customCardH = -1.0f;                  /**< Explicit card height in dp, or -1 for auto. */

    float m_cardX = 0.0f;                         /**< Card top‑left X. */
    float m_cardY = 0.0f;                         /**< Card top‑left Y. */
    float m_cardW = 0.0f;                         /**< Card width. */
    float m_cardH = 0.0f;                         /**< Card height. */

    // Scrolling and layout state
    float m_targetScrollY = 0.0f;                 /**< Target vertical scroll offset. */
    float m_currentScrollY = 0.0f;                /**< Current interpolated scroll offset. */
    float m_contentViewportH = 0.0f;              /**< Height of the scrollable content viewport. */
    float m_totalContentH = 0.0f;                 /**< Total height of wrapped content. */
    float m_lastLayoutW = 0.0f;                   /**< Previous card width for dirty detection. */
    bool m_textDirty = true;                      /**< Flag indicating content needs re‑wrapping. */

    bool m_isThumbDragging = false;               /**< Scrollbar thumb drag state. */
    bool m_isContentDragging = false;             /**< Content area drag state. */
    float m_dragStartY = 0.0f;                    /**< Mouse Y at start of drag. */
    float m_dragStartScrollY = 0.0f;              /**< Scroll position at drag start. */

    bool m_isPressedInsideCard = false;           /**< Whether a mouse press began inside the card. */

    /**
     * @brief Performs word‑wrapping of content text using the shader’s text width measurement.
     * @param renderer Shader used for width calculations.
     * @param maxTextWidth Maximum line width in dp.
     */
    void updateWrapLines(MaterialShader& renderer, float maxTextWidth);

public:
    /**
     * @brief Constructs a dialog with optional title and content.
     * @param title Title string (may be empty).
     * @param content Body text (supports newline characters).
     */
    MaterialDialog(const std::string& title = "", const std::string& content = "");

    virtual ~MaterialDialog() override = default;

    /** @brief Sets the title. */
    void setTitle(const std::string& title) { m_title = title; }

    /** @brief Sets the body content and marks layout as dirty. */
    void setContent(const std::string& content);

    /** @brief Sets a custom card width in dp (negative for auto). */
    void setDialogWidth(float w) { m_customCardW = w; }

    /** @brief Sets a custom card height in dp (negative for auto). */
    void setDialogHeight(float h) { m_customCardH = h; }

    /** @brief Sets both custom width and height at once. */
    void setDialogSize(float w, float h) { m_customCardW = w; m_customCardH = h; }

    /**
     * @brief Replaces the body text with a custom View.
     * @param view Pointer to the custom View (ownership transferred to the dialog).
     */
    void setCustomView(View* view);

    /** @brief Sets the animation style for open/close transitions. */
    void setAnimationStyle(DialogAnimationStyle style) { m_animStyle = style; }

    /**
     * @brief Configures the two action buttons.
     * @param cancelText Label for cancel (if empty, no cancel button).
     * @param onCancel Callback for cancel.
     * @param confirmText Label for confirm (if empty, no confirm button).
     * @param onConfirm Callback for confirm.
     */
    void setActions(const std::string& cancelText, std::function<void()> onCancel,
                    const std::string& confirmText, std::function<void()> onConfirm);

    /** @brief Opens the dialog (starts show animation). */
    void show();

    /** @brief Closes the dialog (starts dismiss animation). */
    void dismiss();

    // --- View overrides ---
    void update(float dt) override;
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
    bool handleScroll(float mx, float my, float ox, float oy) override;
};
