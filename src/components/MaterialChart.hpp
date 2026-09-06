/**
 * @file MaterialChart.hpp
 * @brief Material Design 3 Chart component supporting line, bar, area, grouped, and step chart visualizations.
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
#include <vector>
#include <string>

/**
 * @enum ChartType
 * @brief Chart visualization types.
 */
enum class ChartType { 
    Bar,         /**< Standard vertical bar chart. */
    Line,        /**< Standard polyline line chart. */
    SmoothLine,  /**< Smooth interpolated line chart. */
    AreaLine,    /**< Area line chart filled under data curve. */
    StepLine,    /**< Stepped line chart. */
    GroupedBar,  /**< Multi-series side-by-side grouped bar chart. */
    StackedBar,  /**< Stacked bar chart. */
    PillBar      /**< Bar chart with fully rounded pill caps. */
};

/**
 * @enum ChartLineStyle
 * @brief Line segment connection style.
 */
enum class ChartLineStyle { 
    Seamless, /**< Continuous uninterrupted line segments. */
    Gapped    /**< Gapped line segments surrounding data node points. */
};

/**
 * @enum ChartBarStyle
 * @brief Bar cap rounding style.
 */
enum class ChartBarStyle { 
    Standard, /**< Slightly rounded top corners. */
    Pill      /**< Fully rounded semicircular top caps. */
};

/**
 * @enum AxisPointerStyle
 * @brief Vertical tracking cursor pointer line style.
 */
enum class AxisPointerStyle {
    None,   /**< No tracking line displayed. */
    Solid,  /**< Solid vertical line indicator. */
    Dashed  /**< Dashed vertical line indicator. */
};

/**
 * @struct ChartSeries
 * @brief Multi-series data descriptor.
 */
struct ChartSeries {
    std::string name;             /**< Series label name. */
    std::vector<float> data;      /**< Numeric data point values. */
    M3Color color = { 0, 0, 0, 0 }; /**< Custom series line/bar color. */
};

/**
 * @class MaterialChart
 * @brief Interactive Material Design 3 chart supporting smooth kinetic tooltips, multi-series data, and spring animations.
 */
class MaterialChart : public View {
public:
    /**
     * @brief Constructs a MaterialChart with a specified chart type.
     * @param type Chart visualization variant.
     */
    MaterialChart(ChartType type = ChartType::Bar);

    /**
     * @brief Virtual destructor.
     */
    virtual ~MaterialChart() override = default;

    /**
     * @brief Sets single-series data values and corresponding X-axis labels.
     * @param values Vector of numeric data points.
     * @param labels Vector of X-axis label strings.
     */
    void setData(const std::vector<float>& values, const std::vector<std::string>& labels);

    /**
     * @brief Sets multi-series data sets and shared X-axis labels.
     * @param seriesList Vector of ChartSeries descriptors.
     * @param xLabels Vector of X-axis label strings.
     */
    void setMultiSeries(const std::vector<ChartSeries>& seriesList, const std::vector<std::string>& xLabels);

    /**
     * @brief Configures X-axis label descriptions.
     * @param labels Label strings.
     */
    void setXAxisLabels(const std::vector<std::string>& labels);
    
    /**
     * @brief Sets active chart visualization type.
     * @param type Chart type variant.
     */
    void setType(ChartType type) { setChartType(type); }

    /**
     * @brief Sets active chart visualization type.
     * @param type Chart type variant.
     */
    void setChartType(ChartType type) { m_type = type; m_chartType = type; }

    /**
     * @brief Configures line connectivity style.
     * @param style Line connection style.
     */
    void setLineStyle(ChartLineStyle style) { m_lineStyle = style; }

    /**
     * @brief Configures bar corner rounding style.
     * @param style Bar rounding style.
     */
    void setBarStyle(ChartBarStyle style) { m_barStyle = style; }

    /**
     * @brief Configures vertical cursor indicator line style.
     * @param style Axis indicator line style.
     */
    void setAxisPointerStyle(AxisPointerStyle style) { m_pointerStyle = style; }

    /**
     * @brief Computes preferred intrinsic width for the chart component.
     * @return Ideal preferred width in physical/density-scaled pixels.
     */
    float getPreferredWidth() override { return 400.0f; }

    /**
     * @brief Computes preferred intrinsic height for the chart component.
     * @return Ideal preferred height in physical/density-scaled pixels.
     */
    float getPreferredHeight() override { return 240.0f; }

    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
    bool handleMouseMove(float mx, float my) override;

    /**
     * @brief Resets hovered data point index when cursor leaves chart bounds.
     */
    void onMouseLeave() { m_hoveredIndex = -1; }

private:
    ChartType m_type;
    ChartType m_chartType;
    ChartLineStyle m_lineStyle = ChartLineStyle::Seamless; 
    ChartBarStyle m_barStyle = ChartBarStyle::Standard;    
    AxisPointerStyle m_pointerStyle = AxisPointerStyle::Dashed;

    std::vector<float> m_values;
    std::vector<std::string> m_labels;
    std::vector<std::string> m_xLabels;
    std::vector<ChartSeries> m_seriesList;
    float m_maxValue = 1.0f;

    std::vector<float> m_animProgress;
    std::vector<float> m_hoverAnims;
    int m_hoveredIndex = -1;

    float m_masterAnim = 0.0f; 

    // Time-based Cubic Easing Kinetics
    float m_tipProgress = 1.0f;
    float m_tipStartX = 0.0f;
    float m_tipStartY = 0.0f;
    float m_tipStartVal = 0.0f;
    float m_tipTargetX = 0.0f;
    float m_tipTargetY = 0.0f;
    float m_tipTargetVal = 0.0f;
    bool  m_hasTipInit = false;

    float m_animPointerX = 0.0f;
    float m_animTipRelX = 0.0f;
    float m_animTipRelY = 0.0f;
    float m_animTipAlpha = 0.0f;
    float m_animTipValue = 0.0f;

    /**
     * @brief Generates standard Material 3 categorical palette colors.
     * @param theme Active MaterialTheme palette.
     * @return Vector of M3Color descriptors.
     */
    std::vector<M3Color> getM3Palette(const MaterialTheme& theme);

    /**
     * @brief Draws an anti-aliased dashed line between two points.
     * @param renderer MaterialShader renderer instance.
     * @param x1 Start X coordinate.
     * @param y1 Start Y coordinate.
     * @param x2 End X coordinate.
     * @param y2 End Y coordinate.
     * @param dashLen Length of a single dash stroke.
     * @param gapLen Length of gap between dash strokes.
     * @param color Line stroke color.
     * @param thickness Line stroke thickness in pixels.
     */
    void drawDashedLine(MaterialShader& renderer, float x1, float y1, float x2, float y2, float dashLen, float gapLen, const M3Color& color, float thickness);
};