/**
 * @file MaterialSwitch.cpp
 * @brief Implementation of MaterialSwitch track geometry, thumb animation kinetics, halo projections, and post-landing icon pop-in.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma warning(disable: 4819)
#include "MaterialSwitch.hpp"
#include "../shader/TextureLoader.hpp"
#include <algorithm>
#include <cmath>

extern void requestUIWakeup(double seconds);

/**
 * @brief Linear interpolation helper for floating-point scalar values.
 * @param x Start value.
 * @param y End value.
 * @param a Interpolation factor [0.0, 1.0].
 * @return Interpolated result.
 */
static inline float mix(float x, float y, float a) {
    return x * (1.0f - a) + y * a;
}

/**
 * @brief Evaluates Material 3 Emphasized deceleration cubic easing curve.
 * @param t Normalized linear time progress [0.0, 1.0].
 * @return Eased animation progress [0.0, 1.0].
 */
static inline float m3EmphasizedDecelerate(float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    float inv = 1.0f - t;
    return 1.0f - inv * inv * inv; // Smooth zero-velocity landing
}

MaterialSwitch::MaterialSwitch() {
    layout_width = 52.0f;
    layout_height = 32.0f;
    m_anim = 0.0f;
    m_animStartVal = 0.0f;
    m_animTargetVal = 0.0f;
    m_iconAnim = 1.0f;
    m_iconPopAnimating = false;
}

float MaterialSwitch::getPreferredWidth() {
    return 52.0f;
}

float MaterialSwitch::getPreferredHeight() {
    return 32.0f;
}

void MaterialSwitch::setChecked(bool checked, bool animate) {
    m_checked = checked;
    float target = m_checked ? 1.0f : 0.0f;

    if (!animate) {
        m_anim = target;
        m_animStartVal = target;
        m_animTargetVal = target;
        m_isAnimating = false;
        m_animTime = m_animDuration;
        m_iconAnim = 1.0f;
        m_iconPopAnimating = false;
        return;
    }

    if (std::abs(m_anim - target) < 0.0001f) {
        m_anim = target;
        m_isAnimating = false;
        m_iconAnim = 1.0f;
        m_iconPopAnimating = false;
        return;
    }

    // Start slide: hide icon immediately during transit to eliminate translation jitter
    m_animStartVal = m_anim;
    m_animTargetVal = target;
    m_animTime = 0.0f;
    m_isAnimating = true;
    m_iconAnim = 0.0f;
    m_iconPopAnimating = false;
    requestUIWakeup(m_animDuration + m_iconPopDuration + 0.05);
}

void MaterialSwitch::update(float dt) {
    View::update(dt);

    if (m_isAnimating) {
        m_animTime += (std::min)(dt, 0.033f);
        float t = m_animTime / m_animDuration;

        if (t >= 1.0f) {
            m_anim = m_animTargetVal;
            m_isAnimating = false;
            // Thumb reached final resting position: start icon pop-in expansion immediately
            m_iconPopAnimating = true;
            m_iconPopTime = 0.0f;
            m_iconAnim = 0.0f;
            requestUIWakeup(m_iconPopDuration + 0.02);
        } else {
            float eased = m3EmphasizedDecelerate(t);
            m_anim = mix(m_animStartVal, m_animTargetVal, eased);
            m_iconAnim = 0.0f; // Remain hidden in transit
            requestUIWakeup(0.016);
        }
    } else if (m_iconPopAnimating) {
        m_iconPopTime += (std::min)(dt, 0.033f);
        float t = m_iconPopTime / m_iconPopDuration;

        if (t >= 1.0f) {
            m_iconAnim = 1.0f;
            m_iconPopAnimating = false;
        } else {
            // Cubic ease-out pop-in scaling (from 0.0 to 1.0)
            float inv = 1.0f - t;
            m_iconAnim = 1.0f - inv * inv * inv;
            requestUIWakeup(0.016);
        }
    }
}

void MaterialSwitch::onClick() {
    View::onClick();
    setChecked(!m_checked, true);
    if (m_onCheckedChangeListener) {
        m_onCheckedChangeListener(m_checked);
    }
}

void MaterialSwitch::render(MaterialShader& shader, MaterialTheme& theme) {
    float trackRadius = height / 2.0f;

    // Track and Thumb color interpolations
    M3Color baseTrackUnchecked = theme.surfaceVariant;
    M3Color baseTrackChecked = theme.primary;
    M3Color baseThumbUnchecked = theme.outline;
    M3Color baseThumbChecked = theme.onPrimary;

    if (state == UIState::Hover) {
        baseTrackUnchecked = lerpColor(theme.surfaceVariant, theme.onSurface, 0.04f);
        baseThumbUnchecked = theme.onSurfaceVariant;
        baseTrackChecked = lerpColor(theme.primary, theme.onPrimary, 0.08f);
    } else if (state == UIState::Pressed) {
        baseTrackUnchecked = lerpColor(theme.surfaceVariant, theme.onSurface, 0.10f);
        baseThumbUnchecked = theme.onSurfaceVariant;
        baseTrackChecked = lerpColor(theme.primary, theme.onPrimary, 0.12f);
    }

    M3Color currentTrackColor = lerpColor(baseTrackUnchecked, baseTrackChecked, m_anim);
    M3Color currentThumbColor = lerpColor(baseThumbUnchecked, baseThumbChecked, m_anim);

    // Outline stroke interpolation for unchecked state
    M3Color borderCol = (state == UIState::Hover ? theme.onSurfaceVariant : theme.outline);
    borderCol.a *= (1.0f - m_anim);
    float borderWidth = dp(2.0f) * (1.0f - m_anim);

    // 1. Render Track Container
    shader.drawM3UI(
        x, y, width, height,
        trackRadius, trackRadius, trackRadius, trackRadius,
        currentTrackColor,
        0.0f, 0.0f, 0.0f, 0.0f, { 0,0,0,0 },
        0, 0.0f, { 0,0,0,0 },
        borderCol,
        borderWidth
    );

    // Resolve icon configurations
    Icon checkedIconDesc = m_checkedIcon;
    Icon uncheckedIconDesc = m_uncheckedIcon;
    bool isDualIcon = false;

    if (m_themeSelector) {
        checkedIconDesc = IconM3(M3Icon::Moon);
        uncheckedIconDesc = IconM3(M3Icon::Sun);
        isDualIcon = true;
    } else if (!isIconEmpty(m_checkedIcon) && !isIconEmpty(m_uncheckedIcon)) {
        isDualIcon = true;
    } else if (!isIconEmpty(m_uncheckedIcon)) {
        isDualIcon = true;
        if (isIconEmpty(checkedIconDesc)) checkedIconDesc = IconM3(M3Icon::Check);
    } else if (m_hasIcon) {
        if (isIconEmpty(checkedIconDesc)) checkedIconDesc = IconM3(M3Icon::Check);
        if (isIconEmpty(uncheckedIconDesc)) uncheckedIconDesc = IconM3(M3Icon::Close);
        isDualIcon = true;
    } else if (!isIconEmpty(m_checkedIcon)) {
        isDualIcon = false;
    }

    bool hasAnyIcon = (isDualIcon || m_hasIcon || !isIconEmpty(checkedIconDesc) || !isIconEmpty(uncheckedIconDesc));

    /*
     * Material Design 3 Thumb Geometry:
     * - Switches WITH icons: Base diameter is ALWAYS 24dp.
     * - Switches WITHOUT icons: Unchecked 16dp -> Checked 24dp.
     * - Press Animation: Symmetrically expands up to 28dp around actualCenter without center shifts.
     */
    float minThumbSize = hasAnyIcon ? dp(24.0f) : dp(16.0f);
    float baseThumbSize = mix(minThumbSize, dp(24.0f), m_anim);
    float currentThumbDiameter = mix(baseThumbSize, dp(28.0f), pressAnim);

    float centerUnchecked = dp(16.0f);
    float centerChecked = width - dp(16.0f);
    float actualCenter = mix(centerUnchecked, centerChecked, m_anim);

    // Symmetrical thumb coordinates (Subpixel continuous)
    float thumbX = x + actualCenter - currentThumbDiameter * 0.5f;
    float thumbY = y + (height - currentThumbDiameter) * 0.5f;
    float thumbRadius = currentThumbDiameter * 0.5f;

    // 2. Render Halo Projection
    float haloSize = dp(40.0f);
    float haloX = x + actualCenter - haloSize * 0.5f;
    float haloY = y + height * 0.5f - haloSize * 0.5f;
    float haloRadius = haloSize * 0.5f;

    float haloAlpha = (state == UIState::Pressed) ? 0.16f : ((state == UIState::Hover) ? 0.08f : 0.0f);
    if (haloAlpha > 0.001f) {
        M3Color haloColor = lerpColor(theme.onSurface, theme.primary, m_anim);
        haloColor.a = haloAlpha;
        shader.drawM3UI(
            haloX, haloY, haloSize, haloSize,
            haloRadius, haloRadius, haloRadius, haloRadius,
            haloColor
        );
    }

    // 3. Render Touch Ripple
    if (ripple.active) {
        M3Color activeRippleCol = lerpColor(theme.onSurface, theme.primary, m_anim);
        shader.drawM3UI(
            haloX, haloY, haloSize, haloSize,
            haloRadius, haloRadius, haloRadius, haloRadius,
            { 0,0,0,0 },
            ripple.localX, ripple.localY, ripple.radius, ripple.alpha, activeRippleCol
        );
    }

    // 4. Render Thumb Circle
    shader.drawM3UI(
        thumbX, thumbY, currentThumbDiameter, currentThumbDiameter,
        thumbRadius, thumbRadius, thumbRadius, thumbRadius,
        currentThumbColor
    );

    /*
     * 5. Post-Landing Stationary Icon Pop-In:
     * - Checked state: Pop-in checkedIcon at right thumb position.
     * - Unchecked state: Pop-in uncheckedIcon at left thumb position.
     * - When moving, m_iconAnim is 0, so no icon is drawn in transit (zero jitter/overlapping).
     * - Once thumb lands, the respective icon expands from 0.0 -> 1.0 smoothly.
     */
    if (m_iconAnim > 0.001f) {
        Icon activeIconDesc;
        M3Color iconCol{ 0,0,0,0 };

        if (m_checked) {
            activeIconDesc = checkedIconDesc;
            iconCol = theme.primary;
        } else if (!isIconEmpty(uncheckedIconDesc)) {
            activeIconDesc = uncheckedIconDesc;
            // High-contrast surface color on top of dark outline thumb
            iconCol = theme.surface;
        }

        if (!isIconEmpty(activeIconDesc)) {
            float baseIconSize = m_iconSizeDp > 0.0f ? dp(m_iconSizeDp) : dp(16.0f);
            float curSize = baseIconSize * m_iconAnim;

            // Centered on the stationary thumb center
            float iconX = x + actualCenter - curSize * 0.5f;
            float iconY = y + (height - curSize) * 0.5f;

            iconCol.a *= m_iconAnim;

            if (iconCol.a > 0.01f && curSize > 0.5f) {
                shader.drawIcon(iconX, iconY, curSize, activeIconDesc, iconCol);
            }
        }
    }
}

MaterialSwitchBuilder& MaterialSwitchBuilder::checked(bool val) { m_checked = val; return *this; }
MaterialSwitchBuilder& MaterialSwitchBuilder::hasIcon(bool val) { m_hasIcon = val; return *this; }
MaterialSwitchBuilder& MaterialSwitchBuilder::themeSelector(bool val) { m_themeSelector = val; return *this; }
MaterialSwitchBuilder& MaterialSwitchBuilder::iconSize(float sizeDp) { m_iconSizeDp = sizeDp; return *this; }
MaterialSwitchBuilder& MaterialSwitchBuilder::setOnCheckedChangeListener(std::function<void(bool)> cb) { m_onCheckedChangeListener = std::move(cb); return *this; }
MaterialSwitchBuilder& MaterialSwitchBuilder::checkedIcon(const Icon& icon) { m_checkedIcon = icon; return *this; }
MaterialSwitchBuilder& MaterialSwitchBuilder::checkedIcon(const std::string& str) { m_checkedIcon = iconFromString(str); return *this; }
MaterialSwitchBuilder& MaterialSwitchBuilder::uncheckedIcon(const Icon& icon) { m_uncheckedIcon = icon; return *this; }
MaterialSwitchBuilder& MaterialSwitchBuilder::uncheckedIcon(const std::string& str) { m_uncheckedIcon = iconFromString(str); return *this; }
MaterialSwitchBuilder& MaterialSwitchBuilder::margins(float left, float top, float right, float bottom) {
    m_marginLeft = left; m_marginTop = top; m_marginRight = right; m_marginBottom = bottom;
    return *this;
}

MaterialSwitch* MaterialSwitchBuilder::build() {
    MaterialSwitch* sw = new MaterialSwitch();
    sw->m_hasIcon = m_hasIcon;
    sw->m_themeSelector = m_themeSelector;
    sw->m_iconSizeDp = m_iconSizeDp;
    sw->m_onCheckedChangeListener = m_onCheckedChangeListener;
    sw->m_checkedIcon = m_checkedIcon;
    sw->m_uncheckedIcon = m_uncheckedIcon;
    sw->setMargins(m_marginLeft, m_marginTop, m_marginRight, m_marginBottom);
    sw->setChecked(m_checked, false); // Initialize exact state immediately without animation
    return sw;
}