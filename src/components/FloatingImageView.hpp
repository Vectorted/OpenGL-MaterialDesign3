/**
 * @file FloatingImageView.hpp
 * @brief Dynamic kinetic image container with hardware-accelerated rendering,
 *        supporting in-app dragging and detachable desktop floating window.
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
#include "../shader/MaterialShader.hpp"
#include <string>
#include <functional>

struct GLFWwindow;

/**
 * @class FloatingImageView
 * @brief Draggable interactive media card supporting in-app dragging and seamless OS window detachment.
 */
class FloatingImageView : public View {
public:
    using ClickCallback = std::function<void()>;
    using LongPressCallback = std::function<void(bool isDetached)>;

    float floatingX = 1000.0f;  /**< Absolute floating X coordinate in pixels. */
    float floatingY = 80.0f;    /**< Absolute floating Y coordinate in pixels. */

    float targetWidth = 300.0f; /**< Target rendering width in pixels. */
    float targetHeight = 600.0f;/**< Target rendering height in pixels. */
    float cornerRadius = 32.0f; /**< Corner rounding radius in pixels. */

    unsigned int textureID = 0; /**< OpenGL 2D texture descriptor ID. */
    bool isHovered = false;     /**< Pointer hover state flag. */
    bool isDragging = false;    /**< In-app drag tracking flag. */
    float dragOffsetX = 0.0f;   /**< Cursor offset X relative to floating origin during drag. */
    float dragOffsetY = 0.0f;   /**< Cursor offset Y relative to floating origin during drag. */

    /** @brief Constructs a default FloatingImageView instance. */
    FloatingImageView();

    /**
     * @brief Constructs a FloatingImageView with an initial image and geometry dimensions.
     * 
     * @param path File path to image resource.
     * @param width Initial width.
     * @param height Initial height.
     * @param radius Corner radius.
     */
    FloatingImageView(const std::string& path, float width = 300.0f, float height = 600.0f, float radius = 32.0f);

    /** @brief Destructor that ensures window cleanup and GPU texture release. */
    ~FloatingImageView() override;

    /**
     * @brief Loads a high-resolution image asset into an OpenGL texture.
     * 
     * @param path File system path to the image.
     */
    void loadHighResImage(const std::string& path);

    /** @brief Alias for loadHighResImage. */
    void loadImage(const std::string& path) { loadHighResImage(path); }

    /** @brief Sets rendering dimensions. */
    void setSize(float width, float height);

    /** @brief Sets corner radius in pixels. */
    void setCornerRadius(float radius);

    /** @brief Sets absolute floating coordinates in pixels. */
    void setPosition(float px, float py);

    /**
     * @brief Detaches component into an independent topmost desktop window.
     */
    void detachToDesktop();

    /**
     * @brief Docks the detached desktop window back into the main application window.
     */
    void dockToApp();

    /**
     * @brief Checks whether the component is currently detached as an independent desktop window.
     * 
     * @return true if detached, false if docked in-app.
     */
    bool isDetached() const { return widgetWin != nullptr; }

    /**
     * @brief Enables or disables automatic window detachment on 0.8s long-press.
     * 
     * @param enable If true, enables automatic long-press detachment.
     */
    void setAutoDetachEnabled(bool enable) { m_autoDetachEnabled = enable; }

    /**
     * @brief Checks whether automatic detachment is enabled.
     * 
     * @return true if enabled, false otherwise.
     */
    bool isAutoDetachEnabled() const { return m_autoDetachEnabled; }

    /** @brief Sets click callback listener. */
    void setOnClickListener(ClickCallback cb) { m_onClick = cb; }

    /** @brief Sets right-click callback listener. */
    void setOnRightClickListener(ClickCallback cb) { m_onRightClick = cb; }

    /** @brief Sets long-press callback listener. */
    void setOnLongClickListener(LongPressCallback cb) { m_onLongPress = cb; }

    void update(float mouseX, float mouseY, bool isLeftMouseDown);
    void render(MaterialShader& shader);
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    bool handleMouseButton(int button, int action, float mx, float my) override;
    bool handleMouseMove(float mx, float my) override;
    bool handleScroll(float mx, float my, float ox, float oy) override;

private:
    /**
     * @brief Applies platform-specific window manager flags (e.g. WS_EX_TOOLWINDOW, _NET_WM_STATE_SKIP_TASKBAR).
     */
    void applyOSWindowHiding(GLFWwindow* subWin, GLFWwindow* mainWin);

    /**
     * @brief Closes the secondary detached GLFW desktop window and frees its resources.
     */
    void closeWidget();

    std::string m_imagePath;

    GLFWwindow* m_mainWin = nullptr;
    bool m_autoDetachEnabled = false;

    GLFWwindow* widgetWin = nullptr;
    FloatingImageView* widgetEngine = nullptr;
    MaterialShader* widgetRenderer = nullptr;

    double dragOffsetX_OS = 0.0;
    double dragOffsetY_OS = 0.0;
    bool isDraggingOS = false;

    bool isWidgetRightPressing = false;
    double widgetRightPressStart = 0.0;

    bool isMainRightPressing = false;
    double mainRightPressStart = 0.0;

    bool isMainLeftPressing = false;
    double mainLeftPressStart = 0.0;
    bool mainLeftDragged = false;

    ClickCallback m_onClick = nullptr;
    ClickCallback m_onRightClick = nullptr;
    LongPressCallback m_onLongPress = nullptr;
};