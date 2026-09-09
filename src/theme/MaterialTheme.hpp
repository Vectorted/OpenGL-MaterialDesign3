/**
 * @file MaterialTheme.hpp
 * @brief Material Design 3 color system, theme tokens, and dynamic theme manager.
 * 
 * Provides a complete set of M3 color tokens, interpolation utilities, and a global
 * theme manager for smooth transitions between color schemes.
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
    Filled,    /**< High-emphasis filled button with solid background. */
    Tonal,     /**< Medium-emphasis tonal filled button using secondary container. */
    Outlined,  /**< Medium-emphasis bordered button with transparent background. */
    Text,      /**< Low-emphasis text-only button with no container. */
    Elevated   /**< Elevated button with shadow projection and raised surface. */
};

/**
 * @struct M3Color
 * @brief Represents a 4-component RGBA color with normalized floating-point values [0.0, 1.0].
 */
struct M3Color {
    float r; /**< Red channel (0.0 – 1.0). */
    float g; /**< Green channel (0.0 – 1.0). */
    float b; /**< Blue channel (0.0 – 1.0). */
    float a; /**< Alpha channel (0.0 – 1.0). */
};

/**
 * @brief Linearly interpolates between two RGBA colors.
 * @param a Start color.
 * @param b End color.
 * @param t Interpolation factor [0.0, 1.0] (clamped internally).
 * @return Interpolated M3Color.
 */
M3Color lerpColor(M3Color a, M3Color b, float t);

/**
 * @struct MaterialTheme
 * @brief Complete set of Material Design 3 color tokens for a single theme (light or dark).
 * 
 * All fields are M3Color structures. Use with ThemeManager for dynamic switching.
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

    M3Color inverseSurface;        /**< Background fill for inverted surfaces (e.g., snackbars). */
    M3Color inverseOnSurface;      /**< Content color on top of inverted surface. */

    /**
     * @brief Applies a global alpha scaling factor to all color tokens.
     * @param alpha Scaling factor [0.0, 1.0] to multiply each color's alpha channel.
     * @return A new theme instance with adjusted alpha values.
     * @note Useful for dialogs, fade animations, or unified opacity overlays.
     */
    MaterialTheme withAlpha(float alpha) const {
        MaterialTheme t = *this;
        t.primary.a *= alpha;
        t.onPrimary.a *= alpha;
        t.primaryContainer.a *= alpha;
        t.onPrimaryContainer.a *= alpha;

        t.secondary.a *= alpha;
        t.onSecondary.a *= alpha;
        t.secondaryContainer.a *= alpha;
        t.onSecondaryContainer.a *= alpha;

        t.tertiary.a *= alpha;
        t.onTertiary.a *= alpha;
        t.tertiaryContainer.a *= alpha;
        t.onTertiaryContainer.a *= alpha;

        t.surface.a *= alpha;
        t.surfaceVariant.a *= alpha;
        t.surfaceContainerLow.a *= alpha;
        t.surfaceContainer.a *= alpha;
        t.surfaceContainerHigh.a *= alpha;
        t.onSurface.a *= alpha;
        t.onSurfaceVariant.a *= alpha;

        t.outline.a *= alpha;
        t.outlineVariant.a *= alpha;

        t.inverseSurface.a *= alpha;
        t.inverseOnSurface.a *= alpha;
        return t;
    }
};

/**
 * @namespace MaterialColorScheme
 * @brief Preset color palettes compliant with Material Design 3 guidelines.
 * 
 * Provides a selection of light and dark themes with various accent colors.
 * All themes are fully defined MaterialTheme structures.
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
 * 
 * Maintains an active theme that can be transitioned to a target theme over time
 * using frame-based interpolation. All UI components should query the active theme
 * via getActiveTheme().
 */
class ThemeManager {
private:
    static MaterialTheme s_activeTheme;    /**< Currently rendered theme with active transition state. */
    static MaterialTheme s_targetTheme;    /**< Target theme to transition towards. */
    static float s_transitionSpeed;        /**< Speed multiplier for color interpolation transitions. */

public:
    /**
     * @brief Sets the target theme and optionally forces an immediate switch.
     * @param theme The new theme token set to apply.
     * @param immediate If true, skips interpolation and updates active theme instantly.
     */
    static void setTheme(const MaterialTheme& theme, bool immediate = false);

    /**
     * @brief Retrieves the current active theme tokens.
     * @return Const reference to the active MaterialTheme.
     */
    static const MaterialTheme& getActiveTheme();

    /**
     * @brief Updates color transitions based on elapsed frame time.
     * @param dt Delta time in seconds since the last update frame.
     */
    static void update(float dt);
};
