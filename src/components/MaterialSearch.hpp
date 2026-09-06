/**
 * @file MaterialSearch.hpp
 * @brief Material Design 3 Search Bar with symmetrical breathing cursor, smooth navigation, ellipsis truncation, and full clipboard/selection support.
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
#include "../shader/Icon.hpp"
#include "../theme/MaterialTheme.hpp"
#include <string>
#include <vector>
#include <functional>

/**
 * @class MaterialSearch
 * @brief Search Bar input component conforming to Material Design 3 guidelines.
 */
class MaterialSearch : public View {
public:
    /** @brief Constructs a default MaterialSearch instance. */
    MaterialSearch();

    /** @brief Virtual destructor. */
    virtual ~MaterialSearch() override = default;

    /** @brief Sets input search query text. */
    void setText(const std::string& text);

    /** @brief Sets placeholder hint text. */
    void setHint(const std::string& hint);

    /** @brief Retrieves current search text. */
    const std::string& getText() const { return m_text; }

    /** @brief Sets leading icon by token or asset path. */
    void setLeadingIcon(const std::string& str);

    /** @brief Sets trailing icon by token or asset path. */
    void setTrailingIcon(const std::string& str);

    /** @brief Sets text modification callback listener. */
    void setOnTextChanged(std::function<void(const std::string&)> callback);

    /** @brief Sets submission callback listener (invoked on Enter key). */
    void setOnSubmit(std::function<void(const std::string&)> callback);

    /** @brief Sets callback listener for trailing icon click interactions. */
    void setOnTrailingIconClick(std::function<void()> callback);

    /** @brief Checks if a text range is actively highlighted. */
    bool hasSelection() const { return m_selectionStart >= 0 && m_selectionEnd >= 0 && m_selectionStart != m_selectionEnd; }

    /** @brief Retrieves the highlighted text substring. */
    std::string getSelectedText() const;

    /** @brief Deletes the highlighted text substring. */
    void deleteSelection();

    /** @brief Selects all characters. */
    void selectAll();

    /** @brief Clears active text selection highlight. */
    void clearSelection();

    /** @brief Appends a string at current cursor position. */
    void appendString(const std::string& str);

    float getPreferredWidth() override;
    float getPreferredHeight() override;
    void doLayout(float parentX, float parentY, float parentW, float parentH) override;
    void update(float dt) override;
    void render(MaterialShader& shader, MaterialTheme& theme) override;

    bool handleMouseMove(float mx, float my) override;
    bool handleMouseButton(int button, int action, float mx, float my) override;
    bool handleKey(int key, int action) override;
    bool handleChar(unsigned int codepoint) override;

    void onFocusGained() override;
    void onFocusLost() override;
    bool isFocusable() const override { return true; }

private:
    std::string m_text;
    std::string m_hint = "Search...";
    std::string m_displayText;

    Icon m_leadingIcon;
    Icon m_trailingIcon;
    Icon m_clearIcon;

    bool m_hasFocus = false;
    bool m_isHovered = false;
    bool m_isClearHovered = false;
    bool m_isTrailingHovered = false;

    float m_focusAnim = 0.0f;
    float m_hoverAnim = 0.0f;
    float m_cursorBlink = 0.0f;
    int m_cursorPos = 0;

    float m_targetCursorX = -1.0f;
    float m_smoothCursorX = -1.0f;

    int m_selectionStart = -1;
    int m_selectionEnd = -1;
    bool m_isMouseSelecting = false;
    float m_cachedTextRenderX = 0.0f;
    std::vector<std::pair<int, float>> m_charByteOffsets;

    std::function<void(const std::string&)> m_onTextChanged;
    std::function<void(const std::string&)> m_onSubmit;
    std::function<void()> m_onTrailingIconClick;

    size_t getUtf8Length(const std::string& str) const;
    void updateDisplayText();
    int getCharIndexAtX(float localTextX) const;
    bool isInsideIconPos(float mx, float my, float iconCenterX, float iconCenterY, float hitRadius);
};

/**
 * @class MaterialSearchBuilder
 * @brief Fluent builder helper for configuring and creating MaterialSearch instances.
 */
class MaterialSearchBuilder {
private:
    std::string m_hint = "Search";
    std::string m_text = "";
    std::string m_leadingIcon = "m3:search";
    std::string m_trailingIcon = "";
    float m_width = MATCH_PARENT;
    float m_marginLeft = 0.0f, m_marginTop = 0.0f, m_marginRight = 0.0f, m_marginBottom = 0.0f;

    std::function<void(const std::string&)> m_onTextChanged;
    std::function<void(const std::string&)> m_onSubmit;
    std::function<void()> m_onTrailingIconClick;

public:
    MaterialSearchBuilder& hint(const std::string& h) { m_hint = h; return *this; }
    MaterialSearchBuilder& text(const std::string& t) { m_text = t; return *this; }
    MaterialSearchBuilder& trailingIcon(const std::string& i) { m_trailingIcon = i; return *this; }
    MaterialSearchBuilder& leadingIcon(const std::string& i) { m_leadingIcon = i; return *this; }
    MaterialSearchBuilder& width(float w) { m_width = w; return *this; }
    MaterialSearchBuilder& margins(float l, float t, float r, float b) { 
        m_marginLeft = l; m_marginTop = t; m_marginRight = r; m_marginBottom = b; 
        return *this; 
    }

    MaterialSearchBuilder& onTextChanged(std::function<void(const std::string&)> cb) { m_onTextChanged = std::move(cb); return *this; }
    MaterialSearchBuilder& onSubmit(std::function<void(const std::string&)> cb) { m_onSubmit = std::move(cb); return *this; }
    MaterialSearchBuilder& onTrailingClick(std::function<void()> cb) { m_onTrailingIconClick = std::move(cb); return *this; }

    /**
     * @brief Allocates and initializes the configured MaterialSearch instance.
     * 
     * @return MaterialSearch* Pointer to heap-allocated instance.
     */
    MaterialSearch* build();
};