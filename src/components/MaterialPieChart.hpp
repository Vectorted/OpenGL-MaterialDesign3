/**
 * @file MaterialPieChart.hpp
 * @brief High-precision hardware-accelerated polar bar chart with range slices, average lines, and custom units.
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
#include <vector>
#include <string>

/**
 * @struct PolarBarItem
 * @brief Data entry descriptor for a polar bar slice (label, minimum, maximum, average, and optional color string).
 */
struct PolarBarItem {
    std::string label;       /**< Item category label. */
    float minVal = 0.0f;     /**< Lower range boundary value. */
    float maxVal = 0.0f;     /**< Upper range boundary value. */
    float avgVal = 0.0f;     /**< Average indicator value. */
    std::string color = "";  /**< Optional color string (e.g. "#2196F3", "rgba(33,150,243,0.88)"). */
};

/**
 * @class MaterialPieChart
 * @brief Polar bar chart component with radial grid rings, range bars, average arcs, and kinetic tooltips.
 */
class MaterialPieChart : public View {
public:
    /**
     * @brief Constructs a default MaterialPieChart instance.
     */
    MaterialPieChart();

    /**
     * @brief Virtual destructor.
     */
    virtual ~MaterialPieChart() override = default;

    /**
     * @brief Sets polar data item entries.
     * @param items Vector of PolarBarItem descriptors.
     */
    void setData(const std::vector<PolarBarItem>& items);

    /**
     * @brief Sets display unit suffix string.
     * @param unit Unit label string (e.g. "k", "ms").
     */
    void setUnit(const std::string& unit) { m_unit = unit; }

    /**
     * @brief Sets custom legend label strings.
     * @param rangeLabel Legend label for the range slice bars.
     * @param avgLabel Legend label for the average value line.
     */
    void setLegend(const std::string& rangeLabel, const std::string& avgLabel) {
        m_rangeLegend = rangeLabel;
        m_avgLegend = avgLabel;
    }

    /**
     * @brief Sets bar and average indicator colors using hex color strings.
     * @param barHex Hex string for range bars (e.g. "#3F51B5").
     * @param avgHex Hex string for average line (e.g. "#FF9800").
     */
    void setColors(const std::string& barHex, const std::string& avgHex);

    /**
     * @brief Sets bar color from a hex string.
     * @param barHex Hex string for range bars.
     */
    void setBarColor(const std::string& barHex);

    /**
     * @brief Sets average indicator color from a hex string.
     * @param avgHex Hex string for average line.
     */
    void setAvgColor(const std::string& avgHex);

    /**
     * @brief Sets bar and average indicator colors using integer hex values.
     * @param barHex Integer hex for range bars (e.g. 0x3F51B5).
     * @param avgHex Integer hex for average line (e.g. 0xFF9800).
     * @param alpha Bar opacity factor.
     */
    void setColors(uint32_t barHex, uint32_t avgHex, float alpha = 0.88f);

    /**
     * @brief Sets color palette using a collection of hex strings.
     * @param hexColors Vector of hex color strings.
     */
    void setPalette(const std::vector<std::string>& hexColors);

    /**
     * @brief Sets bar and average indicator colors directly using M3Color structs.
     * @param barColor Base color structure for range bars.
     * @param avgColor Base color structure for average line.
     */
    void setColors(const M3Color& barColor, const M3Color& avgColor) {
        m_barColor = barColor;
        m_avgColor = avgColor;
    }

    /**
     * @brief Clears all data entries and resets component internal states.
     */
    void clearData();

    /**
     * @brief Computes preferred intrinsic width for the polar chart.
     * @return Ideal preferred width in density-independent pixels.
     */
    float getPreferredWidth() override { return 450.0f; }

    /**
     * @brief Computes preferred intrinsic height for the polar chart.
     * @return Ideal preferred height in density-independent pixels.
     */
    float getPreferredHeight() override { return 350.0f; }

    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
    bool handleMouseMove(float mx, float my) override;

    /**
     * @brief Resets hovered sector index when cursor leaves view bounds.
     */
    void onMouseLeave() { m_hoveredIndex = -1; }

private:
    std::vector<PolarBarItem> m_items;
    std::string m_unit = "";
    std::string m_rangeLegend = "Range";
    std::string m_avgLegend = "Average";

    M3Color m_barColor = { 0.29f, 0.49f, 0.86f, 0.90f };
    M3Color m_avgColor = { 0.65f, 0.85f, 0.22f, 1.00f };
    std::vector<M3Color> m_palette;

    float m_maxScale = 100.0f;

    std::vector<float> m_hoverAnims;
    int m_hoveredIndex = -1;

    float m_animSweep = 0.0f; 
    float m_tipRelX = 0.0f;
    float m_tipRelY = 0.0f;
    float m_targetTipRelX = 0.0f;
    float m_targetTipRelY = 0.0f;
    float m_tipAlpha = 0.0f;

    /**
     * @brief Initializes dedicated polar coordinate OpenGL shader program and quad VAO.
     */
    void initPolarShader();
    bool m_shaderReady = false;

    static M3Color parseColor(const std::string& str, float defaultAlpha = 0.88f);
    static M3Color hexToColor(uint32_t hex, float alpha = 0.88f);
};