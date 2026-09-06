/**
 * @file MaterialTheme.hpp
 * @brief Material Design 3 color system, theme tokens, and dynamic theme manager.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include <algorithm>

/**
 * @enum M3ButtonType
 * @brief Button variant types conforming to Material Design 3 specifications.
 */
enum class M3ButtonType { 
    Filled,    /**< High-emphasis filled button. */
    Tonal,     /**< Medium-emphasis tonal filled button. */
    Outlined,  /**< Medium-emphasis bordered button with transparent background. */
    Text,      /**< Low-emphasis text-only button. */
    Elevated   /**< Elevated button with shadow projection. */
};

/**
 * @struct M3Color
 * @brief Represents a 4-component RGBA color with normalized floating-point values [0.0, 1.0].
 */
struct M3Color {
    float r; /**< Red color channel component. */
    float g; /**< Green color channel component. */
    float b; /**< Blue color channel component. */
    float a; /**< Alpha transparency component. */
};

/**
 * @brief Linearly interpolates between two RGBA colors.
 * 
 * @param a The start color.
 * @param b The end color.
 * @param t The interpolation factor, clamped automatically within [0.0, 1.0].
 * @return The interpolated M3Color result.
 */
M3Color lerpColor(M3Color a, M3Color b, float t);

/**
 * @struct MaterialTheme
 * @brief Complete set of Material Design 3 color tokens.
 */
struct MaterialTheme {
    M3Color primary;               /**< High-emphasis fills and dominant interactive elements. */
    M3Color onPrimary;             /**< Text and icon color on top of primary color. */
    M3Color primaryContainer;      /**< Lower-emphasis container fill for primary elements. */
    M3Color onPrimaryContainer;    /**< Text and icon color on top of primary container. */

    M3Color secondary;             /**< Less prominent UI components such as filter chips. */
    M3Color onSecondary;           /**< Text and icon color on top of secondary color. */
    M3Color secondaryContainer;    /**< Tonal fill color for secondary containers. */
    M3Color onSecondaryContainer;  /**< Text and icon color on top of secondary container. */

    M3Color tertiary;              /**< Balanced contrasting accents and tertiary elements. */
    M3Color onTertiary;            /**< Text and icon color on top of tertiary color. */
    M3Color tertiaryContainer;     /**< Container fill for tertiary elements. */
    M3Color onTertiaryContainer;   /**< Text and icon color on top of tertiary container. */

    M3Color surface;               /**< Base surface color for standard backgrounds. */
    M3Color surfaceVariant;        /**< Alternative surface color for variant containment. */
    M3Color surfaceContainerLow;   /**< Low-elevation surface container background. */
    M3Color surfaceContainer;      /**< Standard elevation surface container background. */
    M3Color surfaceContainerHigh;  /**< High-elevation surface container background. */
    M3Color onSurface;             /**< High-contrast text and icons on surface backgrounds. */
    M3Color onSurfaceVariant;      /**< Medium-contrast text and glyphs on surface backgrounds. */

    M3Color outline;               /**< High-contrast outlines and decorative borders. */
    M3Color outlineVariant;        /**< Low-contrast dividers and subtle boundary lines. */

    M3Color inverseSurface;        /**< Background fill for inverted surfaces (e.g. snackbars). */
    M3Color inverseOnSurface;      /**< Content color on top of inverted surface. */
};

/**
 * @namespace MaterialColorScheme
 * @brief Preset color palettes compliant with Material Design 3 guidelines.
 */
namespace MaterialColorScheme {
    extern const MaterialTheme BASELINE_LIGHT;   /**< Baseline default purple light theme. */
    extern const MaterialTheme BASELINE_DARK;    /**< Baseline default purple dark theme. */
    extern const MaterialTheme FOREST_LIGHT;     /**< Forest green light theme. */
    extern const MaterialTheme FOREST_DARK;      /**< Forest green dark theme. */
    extern const MaterialTheme TERRACOTTA_LIGHT; /**< Terracotta orange light theme. */
    extern const MaterialTheme TERRACOTTA_DARK;  /**< Terracotta orange dark theme. */
    extern const MaterialTheme OCEAN_LIGHT;      /**< Ocean blue light theme. */
    extern const MaterialTheme OCEAN_DARK;       /**< Ocean blue dark theme. */
}

/**
 * @class ThemeManager
 * @brief Static manager responsible for global theme state and smooth transitions.
 */
class ThemeManager {
private:
    static MaterialTheme s_activeTheme;    /**< Currently rendered theme with active transition state. */
    static MaterialTheme s_targetTheme;    /**< Target theme to transition towards. */
    static float s_transitionSpeed;        /**< Speed multiplier for color interpolation transitions. */

public:
    /**
     * @brief Sets the target theme and optionally forces an immediate switch.
     * 
     * @param theme The new theme token set to apply.
     * @param immediate If true, skips interpolation and updates active theme instantly.
     */
    static void setTheme(const MaterialTheme& theme, bool immediate = false);

    /**
     * @brief Retrieves the current active theme tokens.
     * 
     * @return Const reference to the active MaterialTheme.
     */
    static const MaterialTheme& getActiveTheme();

    /**
     * @brief Updates color transitions based on elapsed frame time.
     * 
     * @param dt Delta time in seconds since the last update frame.
     */
    static void update(float dt);
};