/**
 * @file MaterialTheme.cpp
 * @brief Implementation of Material Design 3 theme presets, utilities, and ThemeManager.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "MaterialTheme.hpp"

/**
 * @brief Linearly interpolates between two RGBA colors.
 * 
 * @param a The start color.
 * @param b The end color.
 * @param t The interpolation factor, clamped to [0.0, 1.0].
 * @return Interpolated color.
 */
M3Color lerpColor(M3Color a, M3Color b, float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    return {
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t
    };
}

namespace MaterialColorScheme {
    /**
     * @brief Baseline Purple Light Theme (Default M3 Standard).
     */
    const MaterialTheme BASELINE_LIGHT = {
        { 0.404f, 0.314f, 0.643f, 1.0f }, { 1.000f, 1.000f, 1.000f, 1.0f }, { 0.918f, 0.867f, 1.000f, 1.0f }, { 0.129f, 0.000f, 0.365f, 1.0f },
        { 0.384f, 0.392f, 0.463f, 1.0f }, { 1.000f, 1.000f, 1.000f, 1.0f }, { 0.910f, 0.898f, 0.980f, 1.0f }, { 0.114f, 0.098f, 0.165f, 1.0f },
        { 0.482f, 0.325f, 0.471f, 1.0f }, { 1.000f, 1.000f, 1.000f, 1.0f }, { 1.000f, 0.843f, 0.980f, 1.0f }, { 0.208f, 0.071f, 0.208f, 1.0f },
        { 0.996f, 0.969f, 1.000f, 1.0f }, { 0.906f, 0.878f, 0.925f, 1.0f },
        { 0.965f, 0.941f, 0.984f, 1.0f }, { 0.949f, 0.925f, 0.973f, 1.0f }, { 0.918f, 0.898f, 0.949f, 1.0f },
        { 0.114f, 0.098f, 0.169f, 1.0f }, { 0.286f, 0.271f, 0.310f, 1.0f },
        { 0.475f, 0.455f, 0.494f, 1.0f }, { 0.769f, 0.753f, 0.788f, 1.0f },
        { 0.192f, 0.180f, 0.200f, 1.0f }, { 0.950f, 0.940f, 0.960f, 1.0f }
    };

    /**
     * @brief Baseline Purple Dark Theme (Default M3 Standard).
     */
    const MaterialTheme BASELINE_DARK = {
        { 0.816f, 0.737f, 1.000f, 1.0f }, { 0.220f, 0.118f, 0.447f, 1.0f }, { 0.310f, 0.216f, 0.545f, 1.0f }, { 0.918f, 0.867f, 1.000f, 1.0f },
        { 0.800f, 0.761f, 0.863f, 1.0f }, { 0.200f, 0.176f, 0.255f, 1.0f }, { 0.290f, 0.267f, 0.345f, 1.0f }, { 0.910f, 0.898f, 0.980f, 1.0f },
        { 0.867f, 0.710f, 0.855f, 1.0f }, { 0.325f, 0.180f, 0.318f, 1.0f }, { 0.408f, 0.259f, 0.400f, 1.0f }, { 1.000f, 0.843f, 0.980f, 1.0f },
        { 0.078f, 0.071f, 0.094f, 1.0f }, { 0.286f, 0.271f, 0.310f, 1.0f },
        { 0.110f, 0.106f, 0.129f, 1.0f }, { 0.133f, 0.122f, 0.157f, 1.0f }, { 0.169f, 0.157f, 0.188f, 1.0f },
        { 0.902f, 0.882f, 0.898f, 1.0f }, { 0.792f, 0.769f, 0.816f, 1.0f },
        { 0.576f, 0.561f, 0.600f, 1.0f }, { 0.286f, 0.271f, 0.310f, 1.0f },
        { 0.900f, 0.890f, 0.910f, 1.0f }, { 0.192f, 0.180f, 0.200f, 1.0f }
    };

    /**
     * @brief Forest Green Light Theme.
     */
    const MaterialTheme FOREST_LIGHT = {
        { 0.180f, 0.427f, 0.231f, 1.0f }, { 1.000f, 1.000f, 1.000f, 1.0f }, { 0.647f, 0.882f, 0.690f, 1.0f }, { 0.000f, 0.133f, 0.035f, 1.0f },
        { 0.322f, 0.384f, 0.329f, 1.0f }, { 1.000f, 1.000f, 1.000f, 1.0f }, { 0.843f, 0.910f, 0.847f, 1.0f }, { 0.090f, 0.149f, 0.098f, 1.0f },
        { 0.220f, 0.392f, 0.443f, 1.0f }, { 1.000f, 1.000f, 1.000f, 1.0f }, { 0.725f, 0.902f, 0.953f, 1.0f }, { 0.000f, 0.122f, 0.157f, 1.0f },
        { 0.965f, 0.992f, 0.957f, 1.0f }, { 0.875f, 0.922f, 0.867f, 1.0f },
        { 0.933f, 0.965f, 0.925f, 1.0f }, { 0.906f, 0.941f, 0.898f, 1.0f }, { 0.878f, 0.914f, 0.871f, 1.0f },
        { 0.098f, 0.129f, 0.098f, 1.0f }, { 0.263f, 0.306f, 0.259f, 1.0f },
        { 0.447f, 0.490f, 0.443f, 1.0f }, { 0.741f, 0.784f, 0.737f, 1.0f },
        { 0.176f, 0.208f, 0.176f, 1.0f }, { 0.933f, 0.965f, 0.925f, 1.0f }
    };

    /**
     * @brief Forest Green Dark Theme.
     */
    const MaterialTheme FOREST_DARK = {
        { 0.486f, 0.765f, 0.529f, 1.0f }, { 0.000f, 0.224f, 0.075f, 1.0f }, { 0.000f, 0.322f, 0.122f, 1.0f }, { 0.647f, 0.882f, 0.690f, 1.0f },
        { 0.686f, 0.749f, 0.690f, 1.0f }, { 0.165f, 0.224f, 0.173f, 1.0f }, { 0.243f, 0.302f, 0.251f, 1.0f }, { 0.843f, 0.910f, 0.847f, 1.0f },
        { 0.557f, 0.737f, 0.788f, 1.0f }, { 0.000f, 0.224f, 0.267f, 1.0f }, { 0.035f, 0.306f, 0.353f, 1.0f }, { 0.725f, 0.902f, 0.953f, 1.0f },
        { 0.063f, 0.090f, 0.067f, 1.0f }, { 0.263f, 0.306f, 0.259f, 1.0f },
        { 0.086f, 0.114f, 0.090f, 1.0f }, { 0.114f, 0.141f, 0.114f, 1.0f }, { 0.141f, 0.173f, 0.141f, 1.0f },
        { 0.871f, 0.902f, 0.867f, 1.0f }, { 0.741f, 0.784f, 0.737f, 1.0f },
        { 0.529f, 0.573f, 0.525f, 1.0f }, { 0.263f, 0.306f, 0.259f, 1.0f }, { 0.871f, 0.902f, 0.867f, 1.0f }, { 0.086f, 0.114f, 0.090f, 1.0f }
    };

    /**
     * @brief Terracotta Orange Light Theme.
     */
    const MaterialTheme TERRACOTTA_LIGHT = {
        { 0.596f, 0.255f, 0.129f, 1.0f }, { 1.000f, 1.000f, 1.000f, 1.0f }, { 1.000f, 0.855f, 0.792f, 1.0f }, { 0.224f, 0.055f, 0.000f, 1.0f },
        { 0.463f, 0.357f, 0.314f, 1.0f }, { 1.000f, 1.000f, 1.000f, 1.0f }, { 1.000f, 0.871f, 0.824f, 1.0f }, { 0.176f, 0.098f, 0.071f, 1.0f },
        { 0.384f, 0.384f, 0.200f, 1.0f }, { 1.000f, 1.000f, 1.000f, 1.0f }, { 0.910f, 0.910f, 0.698f, 1.0f }, { 0.122f, 0.125f, 0.000f, 1.0f },
        { 1.000f, 0.973f, 0.957f, 1.0f }, { 0.957f, 0.886f, 0.855f, 1.0f },
        { 0.976f, 0.941f, 0.925f, 1.0f }, { 0.953f, 0.914f, 0.894f, 1.0f }, { 0.925f, 0.886f, 0.867f, 1.0f },
        { 0.141f, 0.098f, 0.082f, 1.0f }, { 0.333f, 0.267f, 0.243f, 1.0f },
        { 0.525f, 0.447f, 0.416f, 1.0f }, { 0.827f, 0.741f, 0.710f, 1.0f },
        { 0.224f, 0.176f, 0.157f, 1.0f }, { 0.976f, 0.941f, 0.925f, 1.0f }
    };

    /**
     * @brief Terracotta Orange Dark Theme.
     */
    const MaterialTheme TERRACOTTA_DARK = {
        { 1.000f, 0.714f, 0.612f, 1.0f }, { 0.318f, 0.106f, 0.008f, 1.0f }, { 0.455f, 0.180f, 0.067f, 1.0f }, { 1.000f, 0.855f, 0.792f, 1.0f },
        { 0.898f, 0.737f, 0.690f, 1.0f }, { 0.259f, 0.176f, 0.141f, 1.0f }, { 0.361f, 0.263f, 0.224f, 1.0f }, { 1.000f, 0.871f, 0.824f, 1.0f },
        { 0.745f, 0.745f, 0.541f, 1.0f }, { 0.188f, 0.192f, 0.035f, 1.0f }, { 0.286f, 0.286f, 0.118f, 1.0f }, { 0.910f, 0.910f, 0.698f, 1.0f },
        { 0.110f, 0.075f, 0.059f, 1.0f }, { 0.333f, 0.267f, 0.243f, 1.0f },
        { 0.141f, 0.102f, 0.086f, 1.0f }, { 0.173f, 0.129f, 0.110f, 1.0f }, { 0.204f, 0.157f, 0.137f, 1.0f },
        { 0.925f, 0.886f, 0.867f, 1.0f }, { 0.827f, 0.741f, 0.710f, 1.0f },
        { 0.608f, 0.525f, 0.494f, 1.0f }, { 0.333f, 0.267f, 0.243f, 1.0f }, { 0.925f, 0.886f, 0.867f, 1.0f }, { 0.141f, 0.102f, 0.086f, 1.0f }
    };

    /**
     * @brief Ocean Blue Light Theme.
     */
    const MaterialTheme OCEAN_LIGHT = {
        { 0.000f, 0.384f, 0.533f, 1.0f }, { 1.000f, 1.000f, 1.000f, 1.0f }, { 0.718f, 0.902f, 1.000f, 1.0f }, { 0.000f, 0.118f, 0.180f, 1.0f },
        { 0.310f, 0.388f, 0.443f, 1.0f }, { 1.000f, 1.000f, 1.000f, 1.0f }, { 0.824f, 0.910f, 0.973f, 1.0f }, { 0.082f, 0.153f, 0.200f, 1.0f },
        { 0.365f, 0.357f, 0.482f, 1.0f }, { 1.000f, 1.000f, 1.000f, 1.0f }, { 0.886f, 0.871f, 1.000f, 1.0f }, { 0.137f, 0.125f, 0.243f, 1.0f },
        { 0.965f, 0.984f, 1.000f, 1.0f }, { 0.863f, 0.918f, 0.965f, 1.0f },
        { 0.933f, 0.957f, 0.984f, 1.0f }, { 0.906f, 0.933f, 0.961f, 1.0f }, { 0.875f, 0.906f, 0.937f, 1.0f },
        { 0.094f, 0.118f, 0.137f, 1.0f }, { 0.255f, 0.302f, 0.341f, 1.0f },
        { 0.439f, 0.486f, 0.529f, 1.0f }, { 0.725f, 0.776f, 0.824f, 1.0f },
        { 0.169f, 0.196f, 0.220f, 1.0f }, { 0.933f, 0.957f, 0.984f, 1.0f }
    };

    /**
     * @brief Ocean Blue Dark Theme.
     */
    const MaterialTheme OCEAN_DARK = {
        { 0.525f, 0.831f, 1.000f, 1.0f }, { 0.000f, 0.208f, 0.302f, 1.0f }, { 0.000f, 0.294f, 0.416f, 1.0f }, { 0.718f, 0.902f, 1.000f, 1.0f },
        { 0.710f, 0.808f, 0.875f, 1.0f }, { 0.149f, 0.227f, 0.278f, 1.0f }, { 0.231f, 0.306f, 0.361f, 1.0f }, { 0.824f, 0.910f, 0.973f, 1.0f },
        { 0.765f, 0.749f, 0.918f, 1.0f }, { 0.208f, 0.196f, 0.318f, 1.0f }, { 0.286f, 0.275f, 0.396f, 1.0f }, { 0.886f, 0.871f, 1.000f, 1.0f },
        { 0.055f, 0.082f, 0.098f, 1.0f }, { 0.255f, 0.302f, 0.341f, 1.0f }, { 0.078f, 0.106f, 0.125f, 1.0f }, { 0.106f, 0.133f, 0.153f, 1.0f }, { 0.133f, 0.161f, 0.180f, 1.0f },
        { 0.875f, 0.906f, 0.937f, 1.0f }, { 0.725f, 0.776f, 0.824f, 1.0f },
        { 0.522f, 0.569f, 0.612f, 1.0f }, { 0.255f, 0.302f, 0.341f, 1.0f }, { 0.875f, 0.906f, 0.937f, 1.0f }, { 0.078f, 0.106f, 0.125f, 1.0f }
    };
}

/**
 * @brief Current active theme instance undergoing transitions.
 */
MaterialTheme ThemeManager::s_activeTheme = MaterialColorScheme::BASELINE_LIGHT;

/**
 * @brief Target theme instance for dynamic interpolation.
 */
MaterialTheme ThemeManager::s_targetTheme = MaterialColorScheme::BASELINE_LIGHT;

/**
 * @brief Transition speed coefficient controlling interpolation responsiveness.
 */
float ThemeManager::s_transitionSpeed = 10.0f;

/**
 * @brief Sets the target theme and optionally switches immediately.
 * 
 * @param theme The new theme token set.
 * @param immediate If true, skips interpolation and assigns active theme immediately.
 */
void ThemeManager::setTheme(const MaterialTheme& theme, bool immediate) {
    s_targetTheme = theme;
    if (immediate) {
        s_activeTheme = theme;
    }
}

/**
 * @brief Gets the current active theme.
 * 
 * @return Const reference to active MaterialTheme.
 */
const MaterialTheme& ThemeManager::getActiveTheme() {
    return s_activeTheme;
}

/**
 * @brief Smoothly updates active theme tokens towards the target theme.
 * 
 * @param dt Delta time in seconds since the previous frame.
 */
void ThemeManager::update(float dt) {
    float factor = s_transitionSpeed * dt;

    s_activeTheme.primary              = lerpColor(s_activeTheme.primary, s_targetTheme.primary, factor);
    s_activeTheme.onPrimary            = lerpColor(s_activeTheme.onPrimary, s_targetTheme.onPrimary, factor);
    s_activeTheme.primaryContainer     = lerpColor(s_activeTheme.primaryContainer, s_targetTheme.primaryContainer, factor);
    s_activeTheme.onPrimaryContainer   = lerpColor(s_activeTheme.onPrimaryContainer, s_targetTheme.onPrimaryContainer, factor);

    s_activeTheme.secondary            = lerpColor(s_activeTheme.secondary, s_targetTheme.secondary, factor);
    s_activeTheme.onSecondary          = lerpColor(s_activeTheme.onSecondary, s_targetTheme.onSecondary, factor);
    s_activeTheme.secondaryContainer   = lerpColor(s_activeTheme.secondaryContainer, s_targetTheme.secondaryContainer, factor);
    s_activeTheme.onSecondaryContainer = lerpColor(s_activeTheme.onSecondaryContainer, s_targetTheme.onSecondaryContainer, factor);

    s_activeTheme.tertiary             = lerpColor(s_activeTheme.tertiary, s_targetTheme.tertiary, factor);
    s_activeTheme.onTertiary           = lerpColor(s_activeTheme.onTertiary, s_targetTheme.onTertiary, factor);
    s_activeTheme.tertiaryContainer    = lerpColor(s_activeTheme.tertiaryContainer, s_targetTheme.tertiaryContainer, factor);
    s_activeTheme.onTertiaryContainer  = lerpColor(s_activeTheme.onTertiaryContainer, s_targetTheme.onTertiaryContainer, factor);

    s_activeTheme.surface              = lerpColor(s_activeTheme.surface, s_targetTheme.surface, factor);
    s_activeTheme.surfaceVariant       = lerpColor(s_activeTheme.surfaceVariant, s_targetTheme.surfaceVariant, factor);

    s_activeTheme.surfaceContainerLow  = lerpColor(s_activeTheme.surfaceContainerLow, s_targetTheme.surfaceContainerLow, factor);
    s_activeTheme.surfaceContainer     = lerpColor(s_activeTheme.surfaceContainer, s_targetTheme.surfaceContainer, factor);
    s_activeTheme.surfaceContainerHigh = lerpColor(s_activeTheme.surfaceContainerHigh, s_targetTheme.surfaceContainerHigh, factor);

    s_activeTheme.onSurface            = lerpColor(s_activeTheme.onSurface, s_targetTheme.onSurface, factor);
    s_activeTheme.onSurfaceVariant     = lerpColor(s_activeTheme.onSurfaceVariant, s_targetTheme.onSurfaceVariant, factor);
    
    s_activeTheme.outline              = lerpColor(s_activeTheme.outline, s_targetTheme.outline, factor);
    s_activeTheme.outlineVariant       = lerpColor(s_activeTheme.outlineVariant, s_targetTheme.outlineVariant, factor);

    s_activeTheme.inverseSurface       = lerpColor(s_activeTheme.inverseSurface, s_targetTheme.inverseSurface, factor);
    s_activeTheme.inverseOnSurface     = lerpColor(s_activeTheme.inverseOnSurface, s_targetTheme.inverseOnSurface, factor);
}