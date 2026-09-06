/**
 * @file Activity.hpp
 * @brief Android-style Activity lifecycle controller and application context manager.
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

class ApplicationLooper;

namespace content {

/**
 * @class Activity
 * @brief Base controller representing a single application screen with lifecycle callbacks and UI composition.
 */
class Activity {
    friend class ::ApplicationLooper;

private:
    /**
     * @brief Pointer to the underlying ApplicationLooper context managing event loop and rendering.
     */
    ApplicationLooper* m_appContext = nullptr;

    /**
     * @brief Root view of the layout hierarchy mounted onto this Activity.
     */
    View* m_contentView = nullptr;

    /**
     * @brief Configured initial window title.
     */
    std::string m_title = "Material 3 Application";

    /**
     * @brief Configured initial window width in pixels.
     */
    int m_defaultWidth = 1280;

    /**
     * @brief Configured initial window height in pixels.
     */
    int m_defaultHeight = 720;

public:
    /**
     * @brief Default constructor.
     */
    Activity() = default;

    /**
     * @brief Virtual destructor ensuring clean deallocation of mounted content views.
     */
    virtual ~Activity() {
        if (m_contentView) {
            delete m_contentView;
            m_contentView = nullptr;
        }
    }

    /**
     * @brief Pre-initialization lifecycle hook executed before window or graphics context creation.
     * @note OpenGL context is NOT available at this stage. Use this method to configure window metadata via setSize() or setTitle().
     */
    virtual void onInit() {}

    /**
     * @brief Creation lifecycle hook executed immediately after GLFW window and OpenGL context are initialized.
     * @note Safe for loading OpenGL resources such as custom TrueType fonts, window icons, or shaders.
     * @param app Reference to the active application looper context.
     */
    virtual void onCreate(ApplicationLooper& app) {}

    /**
     * @brief Declarative UI composition hook responsible for building the component hierarchy.
     * @param app Reference to the active application looper context.
     * @return Pointer to the root View container (e.g., ViewGroup or ScrollView).
     */
    virtual View* compose(ApplicationLooper& app) = 0;

    /**
     * @brief Callback triggered when the window framebuffer size changes.
     * @param app Reference to the active application looper context.
     * @param width New framebuffer width in pixels.
     * @param height New framebuffer height in pixels.
     */
    virtual void onResize(ApplicationLooper& app, int width, int height) {}

    /**
     * @brief Destruction lifecycle hook executed when the window closes before application exit.
     * @note Ideal for persisting user states, closing file descriptors, and releasing background resources.
     * @param app Reference to the active application looper context.
     */
    virtual void onDestroy(ApplicationLooper& app) {}

    /**
     * @brief Requests graceful termination of the running Activity and closes the application window.
     * 
     * Signals the underlying ApplicationLooper to exit the OpenGL ES main loop.
     */
    void finish();

    /**
     * @brief Alias for finish() to gracefully terminate the application main loop.
     */
    void exit();

    /**
     * @brief Retrieves the application looper context associated with this Activity.
     * @return Pointer to ApplicationLooper, or nullptr if not yet started.
     */
    ApplicationLooper* getApplicationLooper() const { return m_appContext; }

    /**
     * @brief Alias for getApplicationLooper().
     * @return Pointer to ApplicationLooper, or nullptr if not yet started.
     */
    ApplicationLooper* getLooper() const { return m_appContext; }

    /**
     * @brief Sets the title string for the application window.
     * @param title Window title text.
     */
    void setTitle(const std::string& title) { m_title = title; }

    /**
     * @brief Retrieves the currently configured window title.
     * @return Const reference to title string.
     */
    const std::string& getTitle() const { return m_title; }

    /**
     * @brief Sets the initial window dimensions.
     * @param width Target window width in pixels.
     * @param height Target window height in pixels.
     */
    void setSize(int width, int height) { m_defaultWidth = width; m_defaultHeight = height; }

    /**
     * @brief Retrieves the configured initial window width.
     * @return Width in pixels.
     */
    int getWidth() const { return m_defaultWidth; }

    /**
     * @brief Retrieves the configured initial window height.
     * @return Height in pixels.
     */
    int getHeight() const { return m_defaultHeight; }

    /**
     * @brief Mounts an explicit root View to this Activity.
     * @param root Pointer to root View component.
     */
    void setContentView(View* root) { m_contentView = root; }

    /**
     * @brief Retrieves the currently mounted content root View.
     * @return Pointer to the root View, or nullptr if not set.
     */
    View* getContentView() const { return m_contentView; }

    /**
     * @brief Drives the complete lifecycle sequence and enters the blocking rendering loop.
     * @param looper Reference to the ApplicationLooper engine instance.
     * @return Exit status code (0 for success, non-zero on error).
     */
    int start(ApplicationLooper& looper);
};

} // namespace content

/**
 * @def MAIN_ACTIVITY
 * @brief Macro entry helper defining a standard main function that instantiates and launches an Activity.
 * @param ActivityClass Concrete subclass type inheriting from content::Activity.
 */
#define MAIN_ACTIVITY(ActivityClass) \
int main(int argc, char** argv) { \
    ApplicationLooper app; \
    ActivityClass activity; \
    return activity.start(app); \
}
