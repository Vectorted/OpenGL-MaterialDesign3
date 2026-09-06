/**
 * @file TextField.hpp
 * @brief Material Design 3 TextField component supporting floating label animations, smooth subpixel cursor kinetics, text selection, and multiple visual styles.
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
 * @enum TextFieldStyle
 * @brief Visual presentation style variants for the text field.
 */
enum class TextFieldStyle {
    Outlined,   /**< Outlined border container with dynamic notch cutout for floating labels. */
    Filled,     /**< Solid container fill with active bottom indicator stroke. */
    Underlined  /**< Minimalist borderless style featuring only a bottom indicator underline. */
};

/**
 * @enum TextFieldState
 * @brief Visual interaction and validation states.
 */
enum class TextFieldState {
    Normal,   /**< Idle default state. */
    Hover,    /**< Pointer hover state. */
    Focused,  /**< Active keyboard focus state. */
    Error     /**< Validation failure error state. */
};

/**
 * @class TextField
 * @brief Comprehensive single-line text input control conforming to Material Design 3 guidelines.
 */
class TextField : public View {
public:
    /** @brief Constructs a default TextField instance. */
    TextField();

    /** @brief Virtual destructor. */
    virtual ~TextField() override = default;

    /** @brief Sets input string content. */
    void setText(const std::string& text);

    /** @brief Sets companion floating label text. */
    void setLabel(const std::string& label);

    /** @brief Sets placeholder hint text. */
    void setHint(const std::string& hint);

    /** @brief Sets error validation message and switches state to Error. */
    void setError(const std::string& error);

    /** @brief Configures visual container style variant. */
    void setStyle(TextFieldStyle style);

    /** @brief Sets text font size in dp. */
    void setTextSize(float sizeDp);

    /** @brief Sets label font size in dp. */
    void setLabelSize(float sizeDp);

    /** @brief Sets container corner rounding radius in dp. */
    void setCornerRadius(float radius);

    /** @brief Sets maximum allowed UTF-8 character length (-1 for unrestricted). */
    void setMaxLength(int maxLen);

    /** @brief Enables or disables read-only mode. */
    void setReadOnly(bool readonly);

    /** @brief Enables or disables password masking mode. */
    void setPassword(bool password);

    /** @brief Sets leading icon descriptor. */
    void setLeadingIcon(const Icon& icon);

    /** @brief Sets leading icon by token or asset path string. */
    void setLeadingIcon(const std::string& str);

    /** @brief Sets trailing icon descriptor. */
    void setTrailingIcon(const Icon& icon);

    /** @brief Sets trailing icon by token or asset path string. */
    void setTrailingIcon(const std::string& str);

    /** @brief Sets clear-action icon descriptor. */
    void setClearIcon(const Icon& icon);

    /** @brief Sets clear-action icon by token or asset path string. */
    void setClearIcon(const std::string& str);

    /** @brief Sets icon bounding box dimension in dp. */
    void setIconSize(float sizeDp);

    /** @brief Sets callback listener invoked on text modification. */
    void setOnTextChanged(std::function<void(const std::string&)> callback);

    /** @brief Sets submission callback listener invoked on Enter key press. */
    void setOnSubmit(std::function<void(const std::string&)> callback);

    /** @brief Sets focus change callback listener. */
    void setOnFocusChange(std::function<void(bool)> callback);

    /** @brief Sets callback listener invoked when trailing icon is clicked. */
    void setOnTrailingIconClick(std::function<void()> callback);

    /** @brief Sets callback listener invoked when clear icon is clicked. */
    void setOnClearClick(std::function<void()> callback);

    /** @brief Appends a single ASCII character at current cursor position. */
    void appendChar(char c);

    /** @brief Appends a UTF-8 string at current cursor position. */
    void appendString(const std::string& str);

    /** @brief Deletes character before cursor or removes active selection. */
    void backspace();

    /** @brief Clears entire text buffer. */
    void clear();

    /** @brief Checks if a text range is actively highlighted. */
    bool hasSelection() const { return m_selectionStart >= 0 && m_selectionEnd >= 0 && m_selectionStart != m_selectionEnd; }

    /** @brief Retrieves the highlighted text substring. */
    std::string getSelectedText() const;

    /** @brief Deletes the highlighted text substring. */
    void deleteSelection();

    /** @brief Selects entire text content. */
    void selectAll();

    /** @brief Clears active selection highlight. */
    void clearSelection();

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

    /** @brief Resets hover states when cursor leaves view bounds. */
    void onMouseLeave() { 
        m_isHovering = false; 
        if (!m_hasFocus && m_error.empty()) m_state = TextFieldState::Normal; 
    }

    bool isFocusable() const override { return !m_readOnly; }

    /** @brief Retrieves current raw text string. */
    const std::string& getText() const { return m_text; }

    /** @brief Checks whether the component currently holds keyboard focus. */
    bool isFocused() const { return m_hasFocus; }

private:
    void renderOutlined(MaterialShader& shader, MaterialTheme& theme);
    void renderFilled(MaterialShader& shader, MaterialTheme& theme);
    void renderUnderlined(MaterialShader& shader, MaterialTheme& theme);

    size_t getUtf8Length(const std::string& str) const;
    void updateDisplayText();
    M3Color getBorderColor(const MaterialTheme& theme) const;
    M3Color getLabelColor(const MaterialTheme& theme) const;

    int getCharIndexAtX(float localTextX) const;
    bool isInsideIconPos(float mx, float my, float iconCenterX, float iconCenterY, float hitRadius);

    std::string m_text;
    std::string m_label;
    std::string m_hint;
    std::string m_error;
    std::string m_displayText;
    
    TextFieldStyle m_style = TextFieldStyle::Outlined;
    TextFieldState m_state = TextFieldState::Normal;

    float m_textSizeDp = 16.0f;
    float m_labelSizeDp = 14.0f;
    float m_cornerRadius = 12.0f;
    float m_iconSizeDp = 20.0f;
    int m_maxLength = -1;

    Icon m_leadingIcon;
    Icon m_trailingIcon;
    Icon m_clearIcon;
    
    bool m_hasLeadingIcon = false;
    bool m_hasTrailingIcon = false;
    bool m_showClearIcon = false;
    bool m_isClearHovered = false;
    bool m_isTrailingHovered = false;

    bool m_readOnly = false;
    bool m_password = false;
    bool m_hasFocus = false;
    bool m_isHovering = false;
    
    float m_labelAnim = 0.0f;
    float m_indicatorAnim = 0.0f;
    float m_cursorBlink = 0.0f;
    int m_cursorPos = 0;

    float m_targetCursorX = -1.0f;
    float m_smoothCursorX = -1.0f;

    float m_preferredHeight = 56.0f;

    int m_selectionStart = -1;
    int m_selectionEnd = -1;
    bool m_isMouseSelecting = false;
    float m_cachedTextRenderX = 0.0f;
    std::vector<std::pair<int, float>> m_charByteOffsets;

    std::function<void(const std::string&)> m_onTextChanged;
    std::function<void(const std::string&)> m_onSubmit;
    std::function<void(bool)> m_onFocusChange;
    std::function<void()> m_onTrailingIconClick;
    std::function<void()> m_onClearClick;
};

/**
 * @class TextFieldBuilder
 * @brief Fluent builder helper for constructing and configuring TextField instances.
 */
class TextFieldBuilder {
public:
    TextFieldBuilder();

    TextFieldBuilder& text(const std::string& t);
    TextFieldBuilder& label(const std::string& l);
    TextFieldBuilder& hint(const std::string& h);
    TextFieldBuilder& error(const std::string& e);
    TextFieldBuilder& style(TextFieldStyle s);
    TextFieldBuilder& textSize(float sizeDp);
    TextFieldBuilder& labelSize(float sizeDp);
    TextFieldBuilder& iconSize(float sizeDp);
    TextFieldBuilder& cornerRadius(float radius);
    TextFieldBuilder& maxLength(int len);
    TextFieldBuilder& readOnly(bool val);
    TextFieldBuilder& password(bool val);
    TextFieldBuilder& width(float w);
    TextFieldBuilder& height(float h);
    TextFieldBuilder& margins(float l, float t, float r, float b);

    TextFieldBuilder& leadingIcon(const Icon& icon);
    TextFieldBuilder& leadingIcon(const std::string& str);
    TextFieldBuilder& trailingIcon(const Icon& icon);
    TextFieldBuilder& trailingIcon(const std::string& str);
    TextFieldBuilder& clearIcon(const Icon& icon);
    TextFieldBuilder& clearIcon(const std::string& str);

    TextFieldBuilder& onTextChanged(std::function<void(const std::string&)> cb);
    TextFieldBuilder& onSubmit(std::function<void(const std::string&)> cb);
    TextFieldBuilder& onFocusChange(std::function<void(bool)> cb);
    TextFieldBuilder& onTrailingClick(std::function<void()> cb);
    TextFieldBuilder& onClearClick(std::function<void()> cb);

    /**
     * @brief Allocates and initializes the configured TextField instance.
     * 
     * @return TextField* Pointer to heap-allocated instance.
     */
    TextField* build();

private:
    std::string m_text;
    std::string m_label;
    std::string m_hint;
    std::string m_error;
    TextFieldStyle m_style = TextFieldStyle::Outlined;
    float m_textSizeDp = 16.0f;
    float m_labelSizeDp = 14.0f;
    float m_iconSizeDp = 20.0f;
    float m_cornerRadius = 12.0f;
    int m_maxLength = -1;
    bool m_readOnly = false;
    bool m_password = false;
    
    float m_width = WRAP_CONTENT;
    float m_height = WRAP_CONTENT;
    float m_marginLeft = 0.0f;
    float m_marginTop = 0.0f;
    float m_marginRight = 0.0f;
    float m_marginBottom = 0.0f;

    Icon m_leadingIcon;
    Icon m_trailingIcon;
    Icon m_clearIcon;

    std::function<void(const std::string&)> m_onTextChanged;
    std::function<void(const std::string&)> m_onSubmit;
    std::function<void(bool)> m_onFocusChange;
    std::function<void()> m_onTrailingIconClick;
    std::function<void()> m_onClearClick;
};