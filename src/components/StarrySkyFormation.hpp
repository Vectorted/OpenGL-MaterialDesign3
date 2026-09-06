/**
 * @file StarrySkyFormation.hpp
 * @brief Dynamic geometric magic array formation component featuring dual rotating squares, concentric rings, and Greek glyph runes.
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
#include <functional>
#include <string>
#include <cmath>

/**
 * @enum FormationLevel
 * @brief State intensity and rotation speed level for the magic formation array.
 */
enum class FormationLevel {
    Inactive = 0, /**< Idle state with subtle rotation and scaling. */
    Active = 1,   /**< Active operational state with moderate rotation and pulsing. */
    Alert = 2     /**< High-energy alert state with rapid rotation and maximum scale. */
};

/**
 * @class StarrySkyFormation
 * @brief Interactive draggable vector graphic array component with continuous rotating geometries and gesture detection.
 */
class StarrySkyFormation : public View {
public:
    /**
     * @brief Constructs a StarrySkyFormation component with a specified initial diameter size.
     * 
     * @param size Bounding diameter size in dp.
     */
    StarrySkyFormation(float size = 80.0f);

    /** @brief Virtual destructor. */
    virtual ~StarrySkyFormation() override = default;

    /** @brief Sets activity level using an integer index [0, 2]. */
    void setLevel(int level);

    /** @brief Sets activity level using the FormationLevel enumeration. */
    void setLevel(FormationLevel level);

    /** @brief Retrieves the integer value of the current level. */
    int getLevel() const { return static_cast<int>(m_level); }

    /** @brief Retrieves the current FormationLevel enumeration. */
    FormationLevel getFormationLevel() const { return m_level; }

    /** @brief Sets the bounding diameter size in dp. */
    void setSize(float size);

    /** @brief Retrieves the configured base diameter size. */
    float getSize() const { return m_size; }

    /** @brief Triggers a scale zoom perturbation animation. */
    void setMove(bool isZoomIn);

    /** @brief Sets single-click interaction callback. */
    void setOnClick(std::function<void()> cb) { m_onClick = cb; }

    /** @brief Sets double-click interaction callback. */
    void setOnDoubleClick(std::function<void()> cb) { m_onDoubleClick = cb; }

    /** @brief Sets long-press interaction callback (triggered after 0.6s). */
    void setOnLongClick(std::function<void()> cb) { m_onLongClick = cb; }

    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    /**
     * @brief Evaluates circular collision bounds including interaction padding and active drag locks.
     */
    bool isInside(float mx, float my) const {
        if (m_isPressed || m_isDragging) return true; 
        float cx = x + width * 0.5f;
        float cy = y + height * 0.5f;
        float r = (m_size * 0.5f) * m_scale;
        return std::hypot(mx - cx, my - cy) <= (r + dp(16.0f));
    }

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;

    /**
     * @brief Static utility method to render the multi-layered magic formation geometry.
     * 
     * @param renderer MaterialShader renderer instance.
     * @param cx Center X screen coordinate.
     * @param cy Center Y screen coordinate.
     * @param radius Bounding outer radius.
     * @param level Formation activity intensity level.
     * @param outerAngle Rotation angle for outer octagonal squares.
     * @param greekAngle Rotation angle for Greek glyph constellation ring.
     * @param pulseProgress Normalized energy pulse timer [0.0, 1.0].
     * @param baseColor Primary stroke color.
     */
    static void drawMagicArray(MaterialShader& renderer, float cx, float cy, float radius,
                               FormationLevel level, float outerAngle, float greekAngle,
                               float pulseProgress, const M3Color& baseColor);

private:
    FormationLevel m_level = FormationLevel::Inactive;
    float m_size = 80.0f;
    float m_scale = 1.0f;
    float m_targetScale = 1.0f;

    float m_outerSpinAngle = 0.0f;
    float m_greekRotateAngle = 0.0f;
    float m_pulseTimer = 0.0f;

    bool m_isPressed = false;
    bool m_isDragging = false;
    bool m_hasFiredLongClick = false;
    float m_downX = 0.0f;
    float m_downY = 0.0f;
    float m_dragOffsetX = 0.0f;
    float m_dragOffsetY = 0.0f;
    float m_pressDuration = 0.0f;

    int m_clickCount = 0;
    float m_clickTimer = 0.0f;

    std::function<void()> m_onClick;
    std::function<void()> m_onDoubleClick;
    std::function<void()> m_onLongClick;
};