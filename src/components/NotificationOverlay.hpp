/**
 * @file NotificationOverlay.hpp
 * @brief Toast notification overlay manager rendering stacked transient in-app banners.
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
#include "../shader/MaterialShader.hpp"
#include "../theme/MaterialTheme.hpp"
#include <string>
#include <list>

/**
 * @enum NotificationStyle
 * @brief Visual severity style for notifications.
 */
enum class NotificationStyle {
    Normal, /**< Standard information notification. */
    Error   /**< Error notification with warning color theme. */
};

/**
 * @struct Notification
 * @brief Runtime descriptor and state container for an individual notification toast.
 */
struct Notification {
    int id;                         /**< Unique identifier. */
    std::string title;              /**< Optional headline title text. */
    std::string message;            /**< Primary notification body message. */
    float duration;                 /**< Display timeout duration in seconds. */
    float timer = 0.0f;             /**< Elapsed active time in seconds. */

    NotificationStyle style = NotificationStyle::Normal; /**< Severity style variant. */

    /**
     * @enum State
     * @brief Transition lifecycle states of the notification.
     */
    enum class State { 
        Entering, /**< Sliding in and fading in. */
        Holding,  /**< Fully visible and counting down timer. */
        Exiting   /**< Sliding out and fading out. */
    };
    
    State state = State::Entering;  /**< Current lifecycle state. */
    float animPhase = 0.0f;         /**< Normalized entrance/exit animation progress. */

    float currentY = 0.0f;          /**< Interpolated vertical position. */
    float startY = 0.0f;            /**< Starting vertical position for stack reorganization. */
    float targetY = 0.0f;           /**< Target vertical position for stack reorganization. */
    float yProgress = 1.0f;         /**< Vertical repositioning animation progress factor. */
    bool hasInitY = false;          /**< Initial placement flag. */

    float width = 0.0f;             /**< Card width in pixels. */
    float height = 0.0f;            /**< Card height in pixels. */

    bool isHovered = false;         /**< Pointer hover state flag. */
    bool isCloseHovered = false;    /**< Close button hover state flag. */
};

/**
 * @class NotificationOverlay
 * @brief Overlay container managing stacked, animated toast notifications.
 */
class NotificationOverlay : public View {
public:
    /** @brief Constructs a NotificationOverlay spanning parent bounds. */
    NotificationOverlay();

    /**
     * @brief Pushes a standard notification banner to the display queue.
     * 
     * @param title Headline title text.
     * @param message Body message text.
     * @param duration Display timeout duration in seconds.
     * @param style Notification style variant.
     */
    void push(const std::string& title, const std::string& message, float duration = 3.5f, NotificationStyle style = NotificationStyle::Normal);

    /**
     * @brief Pushes an error notification banner.
     * 
     * @param title Error headline text.
     * @param message Error body message.
     * @param duration Display timeout duration in seconds.
     */
    void pushError(const std::string& title, const std::string& message, float duration = 4.5f);

    void update(float dt) override;
    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

private:
    std::list<Notification> m_notifications;
    int m_nextId = 1;
};