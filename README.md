# Material 3 OpenGL ES C++ UI Library

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg?style=flat-square&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/20)
[![OpenGL ES](https://img.shields.io/badge/OpenGL%20ES-3.0%2B-green.svg?style=flat-square&logo=opengl)](https://www.khronos.org/opengles/)
[![Material Design](https://img.shields.io/badge/Design-Material%203%20(M3)-blueviolet.svg?style=flat-square&logo=materialdesign)](https://m3.material.io/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Embedded-lightgrey.svg?style=flat-square)]()
[![License](https://img.shields.io/badge/License-MIT-orange.svg?style=flat-square)](LICENSE)

A high-performance, lightweight, and hardware-accelerated **Material Design 3 (M3)** UI component framework written in modern C++17/20 and OpenGL ES 3.0+. 

Engineered for desktop applications, embedded systems, and custom rendering engines, this library features single-pass SDF (Signed Distance Field) anti-aliased geometry, second-order spring dynamics, multi-byte UTF-8 / Emoji handling, full touch/mouse ripple mechanics, and native desktop overlay integration.

## UI Showcase

<p align="center">
  <img src="./docs/images/image3.png" alt="Material 3 Component Catalog Overview" width="100%"/>
  <br/>
</p>

---

## 📑 Table of Contents

- [🖼️ UI Showcase](#-ui-showcase)
- [🏛️ Key Architecture Highlights](#️-key-architecture-highlights)
- [📦 Component Showcase](#-component-showcase)
  - [1. Actions & Inputs](#1-actions--inputs)
  - [2. Navigation & Containers](#2-navigation--containers)
  - [3. Feedback & Indicators](#3-feedback--indicators)
  - [4. Media, Data & Complex Pickers](#4-media-data--complex-pickers)
- [Directory Structure](#-directory-structure)
- [🛠️ Build & Installation](#️-build--installation)
  - [MSVC Build (Visual Studio)](#msvc-build-visual-studio)
  - [MinGW-w64 Build (GCC/Clang)](#mingw-w64-build-gccclang)
- [🚀 Quick Start (Minimal Activity Example)](#start-designing-your-first-awesome-ui)
- [📄 License & Credits](#-license--credits)

---
## 🛠️ Build Instructions
### The build system relies on standard CMake with in-tree vendored dependencies.

## MSVC Build (Visual Studio)
```bash
# Open "x64 Native Tools Command Prompt for VS"
cmake -B build
cmake --build build --config Release --parallel
```

## MinGW-w64 Build (GCC/Clang)
```bash
# Ensure gcc, g++, and make/ninja are in your PATH
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```

---
## Start designing your first awesome UI
```cpp
/**
 * @file main.cpp
 * @brief Minimal runnable entry point demonstrating the Material 3 OpenGL ES UI Framework.
 * 
 * Sets up an Activity with a dual-axis centered Button that triggers an
 * interactive Material 3 Snackbar notification upon user interaction.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */
#include "content/Activity.hpp"
#include "components/MaterialComponents.hpp"
#include "event/ApplicationLooper.hpp"
#include "theme/MaterialTheme.hpp"

using namespace content;

/**
 * @class MainActivity
 * @brief Minimal application activity showcasing centered layout alignment and Snackbar integration.
 * 
 * Inherits from content::Activity to manage the lifecycle, configure window parameters,
 * load font typography assets, and construct the visual UI component tree.
 */
class MainActivity : public Activity {
private:
    Snackbar* snackbar = nullptr; /**< Global transient snackbar feedback layer. */

public:
    /**
     * @brief Configures initial window title and default viewport dimensions.
     */
    void onInit() override {
        setTitle("Material 3 OpenGL ES — Minimal Example");
        setSize(800, 600);
    }

    /**
     * @brief Performs pre-composition configuration such as font asset loading.
     * 
     * @param app Reference to the running ApplicationLooper runtime context.
     */
    void onCreate(ApplicationLooper& app) override {
        app.setConsoleVisible(false);
        app.setEnableDebugTitle(true);

        /* Load typeface asset for text and icon rasterization */
        app.getShader().loadFont("./resources/Edition.ttf");
    }

    /**
     * @brief Constructs and binds the minimal component hierarchy.
     * 
     * Creates a full-screen layout centered on both axes (Gravity::CENTER), adds a
     * Filled Button, and binds a click listener to trigger the Snackbar.
     * 
     * @param app Reference to the active ApplicationLooper.
     * @return View* Root ViewGroup containing the layout and overlay components.
     */
    View* compose(ApplicationLooper& app) override {
        /* 1. Root container holding both layout tree and top-level overlays */
        ViewGroup* root = new ViewGroup();
        root->setLayoutParams(MATCH_PARENT, MATCH_PARENT);

        /* 2. Initialize the transient Snackbar component */
        snackbar = new Snackbar();
        Snackbar* sb = snackbar;

        /* 3. Full-screen container centered on both horizontal and vertical axes */
        LinearLayout* centerContainer = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        centerContainer->setLayoutParams(MATCH_PARENT, MATCH_PARENT);
        centerContainer->setGravity(Gravity::CENTER);

        /* 4. Construct a Material 3 Filled Button using the fluent builder API */
        Button* helloButton = Components::button()
            .filled()
            .text("Click Me")
            .textSize(16.0f)
            .padding(24.0f, 14.0f, 24.0f, 14.0f)
            .setOnClickListener([sb]() {
                sb->show("Hello Material3 OpenGL ES", "DISMISS", nullptr, 3.0f);
            })
            .build();

        /* 5. Assemble the visual tree */
        centerContainer->addView(helloButton);

        root->addView(centerContainer);
        root->addView(snackbar); // Added last to ensure overlay renders on top

        return root;
    }

    /**
     * @brief Handles window resize events.
     * 
     * @param app Reference to the running ApplicationLooper.
     * @param width New framebuffer width in pixels.
     * @param height New framebuffer height in pixels.
     */
    void onResize(ApplicationLooper& app, int width, int height) override {}

    /**
     * @brief Cleanup hook invoked when the activity lifecycle terminates.
     * 
     * @param app Reference to the terminating ApplicationLooper.
     */
    void onDestroy(ApplicationLooper& app) override {}
};

/* Register the main application entry */
MAIN_ACTIVITY(MainActivity)
```
