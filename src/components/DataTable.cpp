/**
 * @file DataTable.cpp
 * @brief Implementation of DataTable layout calculations, header grouping, and rendering passes.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "DataTable.hpp"
#include <algorithm>
#include <cmath>

DataTable::DataTable() {
    layout_width = MATCH_PARENT;
    layout_height = WRAP_CONTENT; 
}

void DataTable::addColumn(const std::string& title, float weight) {
    m_columns.push_back({ title, title, weight, "", true });
}

void DataTable::setColumns(const std::vector<TableColumn>& cols) {
    m_columns = cols;
    m_isMultiDimension = false;
    m_headerGroups.clear();
}

void DataTable::addDimensionGroup(const std::string& groupName, const std::vector<TableColumn>& subColumns) {
    m_isMultiDimension = true;
    int startIdx = static_cast<int>(m_columns.size());
    int span = static_cast<int>(subColumns.size());

    for (auto col : subColumns) {
        col.parentGroup = groupName;
        m_columns.push_back(col);
    }
    m_headerGroups.push_back({ groupName, startIdx, span });
}

void DataTable::addRow(const std::vector<std::string>& cells) {
    Row r;
    r.cells = cells;
    m_rows.push_back(r);
}

void DataTable::clearRows() {
    m_rows.clear();
    m_hoveredRow = -1;
    m_pressedRow = -1;
}

void DataTable::setOnRowClicked(std::function<void(int)> callback) {
    m_onRowClicked = std::move(callback);
}

float DataTable::getPreferredWidth() { 
    return 600.0f; 
}

float DataTable::getPreferredHeight() {
    float headerH = m_isMultiDimension ? dp(72.0f) : dp(56.0f);
    return headerH + m_rows.size() * dp(52.0f);
}

float DataTable::getRowY(int index) const {
    float headerH = m_isMultiDimension ? dp(72.0f) : dp(56.0f);
    return y + headerH + index * dp(52.0f);
}

void DataTable::update(float dt) {
    View::update(dt);
    float animSpeed = 15.0f * std::min(dt, 0.033f);

    for (int i = 0; i < (int)m_rows.size(); ++i) {
        float targetHover = (i == m_hoveredRow) ? 1.0f : 0.0f;
        float targetPress = (i == m_pressedRow) ? 1.0f : 0.0f;
        
        m_rows[i].hoverAnim += (targetHover - m_rows[i].hoverAnim) * animSpeed;
        m_rows[i].pressAnim += (targetPress - m_rows[i].pressAnim) * animSpeed;
    }
}

bool DataTable::handleMouseMove(float mx, float my) {
    bool inside = isInside(mx, my);
    m_hoveredRow = -1;

    float headerH = m_isMultiDimension ? dp(72.0f) : dp(56.0f);
    if (inside && my > y + headerH) { 
        int index = static_cast<int>((my - (y + headerH)) / dp(52.0f));
        if (index >= 0 && index < (int)m_rows.size()) {
            m_hoveredRow = index;
            return true;
        }
    }
    return inside;
}

bool DataTable::handleMouseButton(int button, int action, float mx, float my) {
    bool inside = isInside(mx, my);

    if (action == 1) { 
        if (inside && m_hoveredRow != -1) {
            m_pressedRow = m_hoveredRow;
            return true;
        }
    } else if (action == 0) { 
        if (m_pressedRow != -1) {  
            if (inside && m_pressedRow == m_hoveredRow) {
                m_rows[m_pressedRow].selected = !m_rows[m_pressedRow].selected;
                if (m_onRowClicked) m_onRowClicked(m_pressedRow);
            }
            m_pressedRow = -1;
            return true;
        }
    }
    return false;
}

void DataTable::render(MaterialShader& renderer, MaterialTheme& theme) {
    if (m_columns.empty()) return;

    float r = dp(12.0f);
    renderer.drawM3UI(x, y, width, height, r, r, r, r, theme.surface);
    renderer.drawM3UI(x, y, width, height, r, r, r, r, {0,0,0,0}, 0,0,0,0, {0,0,0,0}, 0,0, {0,0,0,0}, theme.outlineVariant, dp(1.0f));

    float totalWeight = 0;
    for (auto& col : m_columns) totalWeight += col.weight;
    
    std::vector<float> startXs(m_columns.size());
    std::vector<float> colWidths(m_columns.size());
    float currentX = x + dp(16.0f);
    float availableW = width - dp(32.0f);
    
    for (size_t c = 0; c < m_columns.size(); ++c) {
        startXs[c] = currentX;
        colWidths[c] = (m_columns[c].weight / totalWeight) * availableW;
        currentX += colWidths[c];
    }

    float headerH = m_isMultiDimension ? dp(72.0f) : dp(56.0f);
    M3Color headerTx = theme.onSurfaceVariant;

    if (m_isMultiDimension) {
        M3Color headerBg = theme.surfaceContainerHigh;
        renderer.drawM3UI(x, y, width, headerH, r, r, 0, 0, headerBg);

        for (const auto& group : m_headerGroups) {
            float gX = x + dp(16.0f);
            for (int i = 0; i < group.startIndex; ++i) gX += colWidths[i];
            float gW = 0.0f;
            for (int i = 0; i < group.colSpan; ++i) gW += colWidths[group.startIndex + i];

            renderer.drawText(group.name, gX, y + dp(14.0f), dp(12.0f), theme.primary);
            renderer.drawM3UI(gX, y + dp(32.0f), gW - dp(8.0f), dp(1.0f), 0, 0, 0, 0, theme.outlineVariant);
        }

        for (size_t c = 0; c < m_columns.size(); ++c) {
            renderer.drawText(m_columns[c].title, startXs[c], y + dp(40.0f), dp(12.0f), headerTx);
        }
    } else {
        for (size_t c = 0; c < m_columns.size(); ++c) {
            renderer.drawText(m_columns[c].title, startXs[c], y + headerH / 2.0f - dp(12.0f)*0.45f, dp(12.0f), headerTx);
        }
    }
    
    M3Color divCol = theme.outlineVariant; 
    divCol.a = 0.5f;
    renderer.drawM3UI(x, y + headerH - dp(1.0f), width, dp(1.0f), 0,0,0,0, divCol);

    float rowH = dp(52.0f);
    for (int i = 0; i < (int)m_rows.size(); ++i) {
        float ry = getRowY(i);

        M3Color rowBg = {0,0,0,0};
        float stateAlpha = m_rows[i].hoverAnim * 0.08f + m_rows[i].pressAnim * 0.12f;
        
        if (m_rows[i].selected) {
            rowBg = theme.secondaryContainer;
            if (stateAlpha > 0) rowBg = lerpColor(rowBg, theme.onSecondaryContainer, stateAlpha);
        } else if (stateAlpha > 0) {
            rowBg = theme.onSurface;
            rowBg.a = stateAlpha;
        }

        if (rowBg.a > 0.001f) {
            float rB = (i == (int)m_rows.size()-1) ? r : 0.0f;
            renderer.drawM3UI(x, ry, width, rowH, 0,0, rB, rB, rowBg);
        }

        M3Color cellTx = theme.onSurface;
        for (size_t c = 0; c < std::min(m_rows[i].cells.size(), m_columns.size()); ++c) {
            renderer.drawText(m_rows[i].cells[c], startXs[c], ry + rowH / 2.0f - dp(14.0f)*0.45f, dp(14.0f), cellTx);
        }

        if (i < (int)m_rows.size() - 1) {
            renderer.drawM3UI(x, ry + rowH - dp(1.0f), width, dp(1.0f), 0,0,0,0, divCol);
        }
    }
}