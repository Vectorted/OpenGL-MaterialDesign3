/**
 * @file ListItem.hpp
 * @brief Material Design 3 List Item component providing structured single or two-line list entries.
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
#include <string>
#include <functional>

/**
 * @class ListItem
 * @brief Represents a standard Material Design 3 list item row supporting leading/trailing icons and supporting text.
 */
class ListItem : public View {
private:
    std::string m_headline;          /**< Primary headline text label. */
    std::string m_supportingText;    /**< Optional secondary supporting text label. */
    std::string m_leadingIcon;       /**< Optional leading icon descriptor. */
    std::string m_trailingIcon;      /**< Optional trailing icon descriptor. */
    std::string m_trailingText;      /**< Optional trailing metadata text string. */

    bool m_isHovered = false;        /**< Pointer hover state flag. */
    bool m_isPressed = false;        /**< Mouse press state flag. */
    bool m_interactive = true;       /**< Flag indicating if item responds to click events. */

    std::function<void()> m_onclick; /**< Click event callback handler. */

public:
    /**
     * @brief Constructs a ListItem with a specified headline text.
     * 
     * @param headline Primary headline string.
     */
    ListItem(const std::string& headline);

    /** @brief Sets the primary headline text. */
    ListItem* setHeadline(const std::string& text);

    /** @brief Sets the secondary supporting text. */
    ListItem* setSupportingText(const std::string& text);

    /** @brief Sets the leading icon by token or asset path. */
    ListItem* setLeadingIcon(const std::string& icon);

    /** @brief Sets the trailing icon by token or asset path. */
    ListItem* setTrailingIcon(const std::string& icon);

    /** @brief Sets the trailing metadata text label. */
    ListItem* setTrailingText(const std::string& text);

    /** @brief Enables or disables interactive click behavior. */
    ListItem* setInteractive(bool interactive);

    /** @brief Sets the click interaction callback listener. */
    ListItem* setOnClickListener(std::function<void()> listener);

    float getPreferredHeight() override;
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;
    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
};