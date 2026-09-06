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
 */
class DialogButton : public View {
public:
    std::string text;             /**< Display label text. */
    bool isPrimary;               /**< Indicates if this button represents the primary confirmation action. */
    std::function<void()> action; /**< Callback action executed upon clicking. */

    float m_dialogAlpha = 1.0f;   /**< Current opacity factor synchronized with the parent dialog animation. */

    /**
     * @brief Constructs a DialogButton.
     * 
     * @param text Button label text.
     * @param isPrimary Primary action flag.
     * @param action Execution callback.
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
 */
class MaterialDialog : public ViewGroup {
private:
    std::string m_title;
    std::string m_content;
    std::vector<std::string> m_wrappedLines;

    float m_animProgress = 0.0f;
    bool m_isOpen = false;

    DialogAnimationStyle m_animStyle = DialogAnimationStyle::FadeZoom;

    DialogButton* m_btnCancel = nullptr;
    DialogButton* m_btnConfirm = nullptr;

    float m_cardX = 0.0f;
    float m_cardY = 0.0f;
    float m_cardW = 0.0f;
    float m_cardH = 0.0f;

public:
    /**
     * @brief Constructs a MaterialDialog with title and content strings.
     * 
     * @param title Dialog headline text.
     * @param content Dialog message body text.
     */
    MaterialDialog(const std::string& title, const std::string& content);

    /** @brief Virtual destructor. */
    virtual ~MaterialDialog() override = default;

    /** @brief Sets dialog headline text. */
    void setTitle(const std::string& title) { m_title = title; }

    /** @brief Sets dialog message body text. */
    void setContent(const std::string& content) { m_content = content; }

    /** @brief Sets transition animation style. */
    void setAnimationStyle(DialogAnimationStyle style) { m_animStyle = style; }

    /**
     * @brief Configures cancel and confirm action buttons.
     * 
     * @param cancelText Cancel button label.
     * @param onCancel Cancel callback handler.
     * @param confirmText Confirm button label.
     * @param onConfirm Confirm callback handler.
     */
    void setActions(const std::string& cancelText, std::function<void()> onCancel,
                    const std::string& confirmText, std::function<void()> onConfirm);

    /** @brief Opens the modal dialog and begins entrance animation. */
    void show();

    /** @brief Closes the modal dialog and begins exit animation. */
    void dismiss();

    /** @brief Checks if dialog is actively visible on screen. */
    bool isVisible() const { return m_animProgress > 0.001f; }

    void update(float dt) override;
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
};