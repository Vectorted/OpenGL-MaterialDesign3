/**
 * @file Activity.cpp
 * @brief Implementation of Activity lifecycle execution and context binding.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "Activity.hpp"
#include "../event/ApplicationLooper.hpp"
#include <iostream>

namespace content {

/**
 * @brief Starts the Activity lifecycle pipeline and enters the main event loop.
 * 
 * Lifecycle Execution Pipeline:
 * 1. Executes onInit() for metadata configuration prior to window creation.
 * 2. Initializes GLFW window and OpenGL ES context via ApplicationLooper::init().
 * 3. Binds Activity reference to looper for event routing.
 * 4. Executes onCreate() for resource initialization within an active OpenGL context.
 * 5. Calls compose() to assemble the UI tree.
 * 6. Enters blocking render loop via ApplicationLooper::run().
 * 7. Calls onDestroy() on application termination.
 * 8. Deallocates the mounted UI tree while the OpenGL context is still valid.
 * 
 * @param looper Application looper instance driving rendering and event dispatch.
 * @return 0 on standard graceful termination, -1 on graphics initialization failure.
 */
int Activity::start(ApplicationLooper& looper) {
    m_appContext = &looper;

    this->onInit();
    if (!looper.init(this->getWidth(), this->getHeight(), this->getTitle().c_str())) {
        std::cerr << "[Activity] Failed to initialize graphics context for: " << this->getTitle() << std::endl;
        m_appContext = nullptr;
        return -1;
    }

    looper.attachActivity(this);

    this->onCreate(looper);

    View* root = this->compose(looper);
    if (!root) {
        root = this->getContentView();
    } else {
        this->setContentView(root);
    }

    if (!root) {
        std::cerr << "[Activity] Error: compose() returned nullptr and no contentView was set." << std::endl;
        m_appContext = nullptr;
        return -1;
    }

    looper.run(root);
    this->onDestroy(looper);

    // Ensure all UI node hierarchies are safely deallocated while OpenGL context is still valid
    if (m_contentView) {
        delete m_contentView;
        m_contentView = nullptr;
    }

    m_appContext = nullptr;
    return 0;
}

/**
 * @brief Requests graceful termination of the running Activity and closes the application window.
 */
void Activity::finish() {
    if (m_appContext) {
        m_appContext->exit();
    }
}

/**
 * @brief Alias for finish() to gracefully terminate the application main loop.
 */
void Activity::exit() {
    this->finish();
}

} // namespace content