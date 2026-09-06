/**
 * @file DataTable.hpp
 * @brief Material Design 3 data table component supporting multidimensional header grouping, sorting, and row selection.
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
#include <functional>

/**
 * @struct TableColumn
 * @brief Configuration descriptor for a single data table column.
 */
struct TableColumn {
    std::string id;              /**< Unique programmatic column identifier. */
    std::string title;           /**< Display title rendered in the table header. */
    float weight = 1.0f;         /**< Proportional layout weight for horizontal width distribution. */
    std::string parentGroup = "";/**< Parent header group name if part of a multidimensional header. */
    bool sortable = true;        /**< Indicates whether user can sort by this column. */
};

/**
 * @enum TableSortOrder
 * @brief Sorting direction states for table columns.
 */
enum class TableSortOrder { 
    None,       /**< No active sorting applied. */
    Ascending,  /**< Ascending sort order (lowest to highest). */
    Descending  /**< Descending sort order (highest to lowest). */
};

/**
 * @class DataTable
 * @brief Structured tabular data view featuring row hover animations, selections, and hierarchical headers.
 */
class DataTable : public View {
public:
    using Column = TableColumn;

    /**
     * @struct Row
     * @brief Runtime state descriptor for a single table data row.
     */
    struct Row {
        std::vector<std::string> cells; /**< Cell text string data. */
        bool selected = false;          /**< Row selection state. */
        float hoverAnim = 0.0f;         /**< Normalized hover transition progress [0.0, 1.0]. */
        float pressAnim = 0.0f;         /**< Normalized press transition progress [0.0, 1.0]. */
    };

    /** @brief Constructs a DataTable with default layout constraints. */
    DataTable();

    /** @brief Virtual destructor. */
    virtual ~DataTable() override = default;

    /**
     * @brief Appends a column configuration.
     * 
     * @param title Display title.
     * @param weight Layout width weight.
     */
    void addColumn(const std::string& title, float weight = 1.0f);

    /**
     * @brief Sets all column configurations.
     * 
     * @param cols Vector of TableColumn descriptors.
     */
    void setColumns(const std::vector<TableColumn>& cols);

    /**
     * @brief Appends a grouped set of columns under a common multi-level header.
     * 
     * @param groupName Header title for the multidimensional group.
     * @param subColumns Collection of sub-column descriptors.
     */
    void addDimensionGroup(const std::string& groupName, const std::vector<TableColumn>& subColumns);

    /**
     * @brief Appends a data row.
     * 
     * @param cells String values for each column cell.
     */
    void addRow(const std::vector<std::string>& cells);

    /**
     * @brief Clears all data rows and resets interaction states.
     */
    void clearRows();

    /**
     * @brief Sets the callback listener for row click interactions.
     * 
     * @param callback Function receiving the zero-based clicked row index.
     */
    void setOnRowClicked(std::function<void(int)> callback);

    float getPreferredWidth() override;
    float getPreferredHeight() override;

    void update(float dt) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
    
    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;

    /** @brief Resets row hover and press state indices when cursor leaves view. */
    void onMouseLeave() { m_hoveredRow = -1; m_pressedRow = -1; }

private:
    std::vector<TableColumn> m_columns; /**< Registered column descriptors. */
    std::vector<Row> m_rows;            /**< Active row data records. */
    
    /**
     * @struct HeaderGroup
     * @brief Span mapping for multidimensional column group headers.
     */
    struct HeaderGroup {
        std::string name; /**< Group category name. */
        int startIndex;   /**< Starting column index. */
        int colSpan;      /**< Number of contiguous columns spanned. */
    };
    std::vector<HeaderGroup> m_headerGroups;
    bool m_isMultiDimension = false;

    int m_hoveredRow = -1;
    int m_pressedRow = -1;
    int m_sortedColIndex = -1;
    TableSortOrder m_sortOrder = TableSortOrder::None;

    std::function<void(int)> m_onRowClicked;

    /**
     * @brief Computes top Y screen coordinate for a specific row index in physical pixels.
     */
    float getRowY(int index) const;
};