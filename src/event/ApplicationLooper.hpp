/**
 * @file ApplicationLooper.hpp
 * @brief Header definition for application rendering loop, window lifecycle, and asynchronous dispatch.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <thread>
#include <chrono>
#include "../shader/MaterialShader.hpp"
#include "../theme/MaterialTheme.hpp"
#include "../layout/View.hpp"

namespace content {
    class Activity;
}

/**
 * @class ApplicationLooper
 * @brief Core engine managing the GLFW window, OpenGL ES pipeline, event dispatch, and UI rendering loop.
 */
class ApplicationLooper {
private:
    GLFWwindow* m_window = nullptr;                       /**< Native GLFW window handle. */
    MaterialShader m_shader;                              /**< Core Material Design 3 shader pipeline instance. */
    MaterialTheme m_theme;                                /**< Active application color palette and typography theme tokens. */
    View* m_root = nullptr;                               /**< Pointer to the root view component of the active UI layout tree. */
    content::Activity* m_attachedActivity = nullptr;      /**< Pointer to the currently mounted Activity controller. */
    double m_lastTime = 0.0;                              /**< Timestamp of the preceding frame for delta time calculation. */
    int m_fbWidth = 0;                                    /**< Current width of the active framebuffer in pixels. */
    int m_fbHeight = 0;                                   /**< Current height of the active framebuffer in pixels. */
    std::string m_baseTitle = "";                         /**< Base window title prefix string. */
    bool m_enableDebugTitle = false;                      /**< Flag controlling real-time telemetry stats output in the window title. */
    double m_lastTitleUpdateTime = 0.0;                   /**< Timestamp of the last performance telemetry title update. */
    float m_currentFps = 60.0f;                           /**< Current calculated render frame rate (Frames Per Second). */

    /**
     * @brief Formats and updates the GLFW window title bar with real-time performance metrics.
     */
    void updateDebugTitle();

public:
    /**
     * @brief Default constructor.
     */
    ApplicationLooper() = default;

    /**
     * @brief Destructor releasing window resources and terminating the GLFW backend cleanly.
     */
    ~ApplicationLooper();

    /**
     * @brief Associates an Activity instance with this looper for event lifecycle routing.
     * @param activity Pointer to the Activity controller.
     */
    void attachActivity(content::Activity* activity) { m_attachedActivity = activity; }

    /**
     * @brief Retrieves the currently attached Activity instance.
     * @return Pointer to attached Activity, or nullptr if none mounted.
     */
    content::Activity* getAttachedActivity() const { return m_attachedActivity; }

    /**
     * @brief Requests graceful termination of the main OpenGL render loop and window closure.
     * 
     * Signals the GLFW window close flag and posts an empty event to wake up
     * the event looper immediately if blocked on glfwWaitEvents().
     */
    void exit();

    /**
     * @brief Dispatches a callable task to execute on a background worker thread.
     * @param task Callable task closure to be enqueued.
     */
    static void runAsyncThread(std::function<void()> task);

    /**
     * @brief Dispatches a callable task to execute safely on the main UI and OpenGL render thread.
     * @param task Callable task closure to be executed.
     */
    static void runOnUiThread(std::function<void()> task);

    /**
     * @brief Sets the icon for a specified GLFW window instance from an image file.
     * @param window Target GLFW window handle.
     * @param iconPath Path to the icon image asset.
     * @return true on success, false on failure.
     */
    static bool setWindowIcon(GLFWwindow* window, const std::string& iconPath);

    /**
     * @brief Sets the icon for the managed application window.
     * @param iconPath Path to the icon image asset.
     * @return true on success, false on failure.
     */
    bool setIcon(const std::string& iconPath);

    /**
     * @brief Configures whether performance telemetry is displayed in the window title bar.
     * @param enable If true, displays FPS, CPU, RAM, and GPU statistics in title.
     */
    void setEnableDebugTitle(bool enable) { m_enableDebugTitle = enable; }

    /**
     * @brief Checks if performance telemetry title updates are enabled.
     * @return true if enabled, false otherwise.
     */
    bool isDebugTitleEnabled() const { return m_enableDebugTitle; }

    /**
     * @brief Configures terminal console window visibility across desktop operating systems.
     * @param visible If true, displays console; otherwise hides it.
     */
    static void setConsoleVisible(bool visible);

    /**
     * @brief Toggles console window visibility state.
     */
    static void toggleConsole();

    /**
     * @brief Queries whether the system console window is currently visible.
     * @return true if visible, false otherwise.
     */
    static bool isConsoleVisible();

    /**
     * @brief Retrieves the raw GLFW window handle.
     * @return GLFWwindow pointer.
     */
    GLFWwindow* getWindow() { return m_window; }

    /**
     * @brief Retrieves a mutable reference to the internal MaterialShader pipeline.
     * @return Reference to MaterialShader.
     */
    MaterialShader& getShader() { return m_shader; }

    /**
     * @brief Retrieves a const reference to the internal MaterialShader pipeline.
     * @return Const reference to MaterialShader.
     */
    const MaterialShader& getShader() const { return m_shader; }

    /**
     * @brief Internal framebuffer size change callback handler.
     * @param width New framebuffer width in pixels.
     * @param height New framebuffer height in pixels.
     */
    void onFramebufferSize(int width, int height);

    /**
     * @brief Internal cursor position callback handler.
     * @param xpos Cursor X coordinate in window space.
     * @param ypos Cursor Y coordinate in window space.
     */
    void onMouseMove(double xpos, double ypos);

    /**
     * @brief Internal mouse button interaction callback handler.
     * @param button Mouse button index.
     * @param action Action state (e.g., GLFW_PRESS or GLFW_RELEASE).
     */
    void onMouseButton(int button, int action);

    /**
     * @brief Internal mouse scroll callback handler.
     * @param xoffset Scroll offset along X axis.
     * @param yoffset Scroll offset along Y axis.
     */
    void onMouseScroll(double xoffset, double yoffset);

    /**
     * @brief Internal keyboard key event callback handler.
     * @param key GLFW key code.
     * @param action Key action state (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT).
     */
    void onKey(int key, int action);

    /**
     * @brief Internal Unicode character input callback handler.
     * @param codepoint Unicode code point.
     */
    void onChar(unsigned int codepoint);

    /**
     * @brief Executes UI layout updates, clears color buffers, and submits draw calls for the active frame.
     */
    void drawFrame();

    /**
     * @brief Initializes the GLFW window, GL context, and input callbacks.
     * @param width Initial window width in pixels.
     * @param height Initial window height in pixels.
     * @param title Initial window title bar text.
     * @return true on successful initialization, false on error.
     */
    bool init(int width, int height, const char* title);

    /**
     * @brief Enters the main event polling and rendering loop until application close is requested.
     * @param root Pointer to the root View of the UI tree.
     */
    void run(View* root);
};