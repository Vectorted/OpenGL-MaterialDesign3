/**
 * @file StarrySkyNotification.hpp
 * @brief High-precision notification toast queue supporting standalone desktop overlay windows and in-app rendering.
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
#include "../theme/MaterialTheme.hpp"
#include "StarrySkyFormation.hpp"
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

/**
 * @enum StarryNotifType
 * @brief Color theme variants for Starry Sky notification banners.
 */
enum class StarryNotifType {
    Green,     /**< Emerald green success style. */
    Red,       /**< Red alert/warning style. */
    Blue,      /**< Sky blue informational style. */
    Pink,      /**< Sakura pink accent style. */
    White,     /**< Minimalist bright white style. */
    Primary,   /**< Follows Material Theme primary color. */
    Secondary, /**< Follows Material Theme secondary color. */
    Tertiary,  /**< Follows Material Theme tertiary color. */
    Error      /**< High-severity error notification. */
};

/**
 * @struct StarryNotifItem
 * @brief State container for an individual notification toast entry.
 */
struct StarryNotifItem {
    std::string id;                                     /**< Unique string identifier. */
    StarryNotifType type = StarryNotifType::Green;      /**< Color role type. */
    std::string title;                                  /**< Notification headline title. */
    std::string desc;                                   /**< Notification body description text. */
    float duration = 4.0f;                              /**< Display timeout duration in seconds. */
    float timer = 0.0f;                                 /**< Elapsed display timer. */

    float animSlideX = 1.0f;                            /**< Normalized horizontal slide transition [0.0, 1.0]. */
    float enterProgress = 0.0f;                         /**< Entrance animation progress. */
    float exitProgress = 0.0f;                          /**< Dismissal exit animation progress. */

    float animSlot = 0.0f;                              /**< Interpolated vertical slot index. */
    float startSlot = 0.0f;                             /**< Starting slot index for smooth reorganization. */
    float targetSlot = 0.0f;                            /**< Target vertical slot index. */
    float slotProgress = 1.0f;                          /**< Vertical reorganization animation progress. */

    bool dismissing = false;                            /**< Flag indicating if toast is exiting. */
    float outerAngle = 0.0f;                            /**< Magic array rotation angle. */
    float greekAngle = 0.0f;                            /**< Greek runes ring rotation angle. */
};

/**
 * @class StarrySkyNotification
 * @brief Notification queue manager rendering stacked banners with vector magic arrays across in-app and desktop modes.
 */
class StarrySkyNotification : public View {
public:
    /**
     * @brief Constructs a StarrySkyNotification instance.
     * 
     * @param maxCount Maximum concurrent visible notifications.
     * @param duration Default notification duration in seconds.
     */
    StarrySkyNotification(int maxCount = 3, float duration = 4.0f);

    /** @brief Destructor that closes secondary desktop overlay window resources. */
    virtual ~StarrySkyNotification() override;

    /**
     * @brief Shows a notification with a typed color variant.
     * 
     * @param type Color style type.
     * @param title Headline title text.
     * @param desc Body description text.
     */
    void show(StarryNotifType type, const std::string& title, const std::string& desc = "");

    /**
     * @brief Shows a notification using a string token for type identification (e.g. "red", "blue", "error").
     * 
     * @param typeStr Type token string.
     * @param title Headline title text.
     * @param desc Body description text.
     */
    void show(const std::string& typeStr, const std::string& title, const std::string& desc = "");

    /** @brief Sets maximum simultaneous visible toasts. */
    void setMaxCount(int maxCount) { m_maxCount = maxCount; }

    /** @brief Sets default display duration in seconds. */
    void setDuration(float seconds) { m_defaultDuration = seconds; }

    /** @brief Configures custom notification icon path. */
    void setIcon(const std::string& iconPath);

    /** @brief Retrieves custom notification icon path. */
    const std::string& getIcon() const { return m_customIconPath; }

    /** @brief Configures custom font asset path for desktop overlay mode. */
    void setFont(const std::string& fontPath);

    /** @brief Retrieves custom font asset path. */
    const std::string& getFont() const { return m_customFontPath; }

    /**
     * @brief Sets rendering mode (true for in-app canvas, false for standalone desktop OS overlay window).
     * 
     * @param insideWindow If true, renders inside main window.
     */
    void setWindowMode(bool insideWindow) { m_isWindowMode = insideWindow; }

    /** @brief Checks whether the component is in in-app window mode. */
    bool isWindowMode() const { return m_isWindowMode; }

    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;

private:
    void spawnOSOverlayWindow();
    void closeOSOverlayWindow();
    void applyWindowIcon();
    void applyWindowFont();

    int m_maxCount = 3;
    float m_defaultDuration = 4.0f;
    bool m_isWindowMode = false;

    std::string m_customIconPath;
    std::string m_customFontPath;

    std::vector<StarryNotifItem> m_items;

    GLFWwindow* m_osWin = nullptr;
    MaterialShader* m_osRenderer = nullptr;
    bool m_wasOSClickPressed = false;

    float m_mouseX = -1.0f;
    float m_mouseY = -1.0f;

    struct HitBox {
        float x, y, w, h;
        float closeX, closeY, closeW, closeH;
        size_t index;
    };
    std::vector<HitBox> m_hitBoxes;
};