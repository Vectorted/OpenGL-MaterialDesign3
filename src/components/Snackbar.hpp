/**
 * @file Snackbar.hpp
 * @brief Material Design 3 transient notification banner component with queue management.
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
#include <queue>

/**
 * @class Snackbar
 * @brief Transient, non-modal single-line notification banner following Material Design 3 specifications.
 */
class Snackbar : public View {
public:
    /**
     * @struct MessageData
     * @brief Encapsulates message payload and callback configuration for a single Snackbar notification.
     */
    struct MessageData {
        std::string text;               /**< Primary message text. */
        std::string actionText;         /**< Optional interactive action button label. */
        std::function<void()> onAction; /**< Action button execution callback. */
        float duration = 4.0f;          /**< Display timeout duration in seconds. */
    };

    /**
     * @brief Constructs a default Snackbar instance spanning full viewport layout parameters.
     */
    Snackbar();

    /**
     * @brief Virtual destructor.
     */
    virtual ~Snackbar() override = default;

    /**
     * @brief Queues a message notification for display.
     * 
     * @param text Primary message string.
     * @param actionText Action button label string.
     * @param onAction Action button callback handler.
     * @param duration Visible duration in seconds.
     */
    void show(const std::string& text, const std::string& actionText = "", std::function<void()> onAction = nullptr, float duration = 4.0f);

    /**
     * @brief Dismisses the active Snackbar immediately and advances the queue.
     */
    void dismiss();

    /**
     * @brief Updates animation states and timer lifecycles.
     * @param dt Delta time elapsed since last frame in seconds.
     */
    void update(float dt) override;

    /**
     * @brief Computes positioning bounds relative to parent container dimensions.
     * @param parentX Parent origin X coordinate.
     * @param parentY Parent origin Y coordinate.
     * @param parentW Parent bounding width.
     * @param parentH Parent bounding height.
     */
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;

    /**
     * @brief Renders the Snackbar bubble, text body, and action button via MaterialShader.
     * @param renderer Reference to the hardware-accelerated shader renderer.
     * @param theme Reference to current dynamic MaterialTheme palette.
     */
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    /**
     * @brief Handles cursor movements and hover state updates.
     * @param mx Mouse cursor X coordinate.
     * @param my Mouse cursor Y coordinate.
     * @return True if event was consumed by the component; false otherwise.
     */
    bool handleMouseMove(float mx, float my) override;

    /**
     * @brief Handles mouse click inputs and action triggers.
     * @param button Pressed mouse button code.
     * @param action Action state (e.g. GLFW_PRESS, GLFW_RELEASE).
     * @param mx Mouse cursor X coordinate.
     * @param my Mouse cursor Y coordinate.
     * @return True if event was consumed by the component; false otherwise.
     */
    bool handleMouseButton(int button, int action, float mx, float my) override;

    /**
     * @brief Indicates whether this component can acquire keyboard input focus.
     * @return Always returns false for non-modal transient overlays.
     */
    bool isFocusable() const override { return false; }

    /**
     * @brief Configures maximum safety character threshold prior to binary search truncation.
     * @param maxChars Maximum allowable character count.
     */
    void setMaxTextLength(size_t maxChars) { m_maxTextLength = maxChars; }

private:
    /**
     * @enum SnackState
     * @brief Lifecycle transition states for the Snackbar.
     */
    enum class SnackState { 
        Hidden,       /**< No banner currently displayed. */
        AnimatingIn,  /**< Sliding and scaling into viewport. */
        Visible,      /**< Fully visible and counting down timer. */
        AnimatingOut  /**< Sliding and scaling out of viewport. */
    };

    SnackState m_sstate = SnackState::Hidden;

    std::queue<MessageData> m_msgQueue;
    MessageData m_currentMsg;

    float m_stateTime = 0.0f;
    float m_progress = 0.0f;

    float m_bubbleX = 0.0f;
    float m_bubbleY = 0.0f;
    float m_bubbleW = 0.0f;
    float m_bubbleH = 0.0f;

    float m_actionReqX = 0.0f;
    float m_actionReqW = 0.0f;
    bool m_isActionHovered = false;
    bool m_wasMouseDown = false;

    size_t m_maxTextLength = 120;
    std::string m_cachedDisplayMsg;
    float m_cachedMsgWidth = 0.0f;
    float m_cachedActWidth = 0.0f;
    float m_lastLayoutW = -1.0f;

    /**
     * @brief Pops the next message from the queue and triggers its entrance animation.
     */
    void processNext();

    /**
     * @brief Formats text into strict single-line and applies binary-search ellipsis truncation.
     * @param renderer Reference to the text measurement renderer.
     * @param maxAvailableWidth Maximum horizontal space available for text.
     * @param fontSize Target font render size in physical pixels.
     */
    void formatSingleLineText(MaterialShader& renderer, float maxAvailableWidth, float fontSize);
};