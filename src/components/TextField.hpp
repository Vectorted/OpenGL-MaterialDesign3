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
 * 
 * Supports floating labels, hint text, error state, leading/trailing icons, clear button,
 * password masking, character limits, text selection, and smooth cursor animations.
 * All rendering is performed via OpenGL ES through the MaterialShader interface.
 */
class TextField : public View {
public:
    /** @brief Constructs a default TextField instance with default property values. */
    TextField();

    /** @brief Virtual destructor. */
    virtual ~TextField() override = default;

    /**
     * @brief Sets the raw text content, respecting maximum length and updating display.
     * @param text New text string (UTF-8 encoded).
     * @note If text exceeds maxLength, it is truncated to the allowed character count.
     */
    void setText(const std::string& text);

    /**
     * @brief Sets the floating label text.
     * @param label Label string displayed above the input field when focused or non-empty.
     */
    void setLabel(const std::string& label);

    /**
     * @brief Sets the placeholder hint text.
     * @param hint Hint displayed when the field is empty and not focused.
     */
    void setHint(const std::string& hint);

    /**
     * @brief Sets an error message and transitions the field to Error state.
     * @param error Error string; if empty, clears error state and reverts to Normal/Focused.
     */
    void setError(const std::string& error);

    /**
     * @brief Configures the visual container style variant.
     * @param style One of Outlined, Filled, or Underlined.
     */
    void setStyle(TextFieldStyle style);

    /**
     * @brief Sets the text font size in density-independent pixels (dp).
     * @param sizeDp Font size in dp.
     */
    void setTextSize(float sizeDp);

    /**
     * @brief Sets the label font size in dp.
     * @param sizeDp Label font size.
     */
    void setLabelSize(float sizeDp);

    /**
     * @brief Sets the container corner radius in dp.
     * @param radius Corner radius value.
     */
    void setCornerRadius(float radius);

    /**
     * @brief Sets the maximum number of UTF-8 characters allowed (-1 means unlimited).
     * @param maxLen Maximum character count.
     */
    void setMaxLength(int maxLen);

    /**
     * @brief Enables or disables read-only mode.
     * @param readonly If true, text modifications are prevented.
     */
    void setReadOnly(bool readonly);

    /**
     * @brief Enables or disables password masking (displays '*' instead of actual characters).
     * @param password If true, display text is obscured.
     */
    void setPassword(bool password);

    /**
     * @brief Sets the leading (left) icon from an Icon descriptor.
     * @param icon Icon structure containing asset or built-in token.
     */
    void setLeadingIcon(const Icon& icon);

    /**
     * @brief Sets the leading icon from a string token or asset path.
     * @param str Icon identifier (e.g., "search" or "ic_search.png").
     */
    void setLeadingIcon(const std::string& str);

    /**
     * @brief Sets the trailing (right) icon from an Icon descriptor.
     * @param icon Icon structure.
     */
    void setTrailingIcon(const Icon& icon);

    /**
     * @brief Sets the trailing icon from a string token or asset path.
     * @param str Icon identifier.
     */
    void setTrailingIcon(const std::string& str);

    /**
     * @brief Sets the clear (X) icon from an Icon descriptor.
     * @param icon Icon structure for the clear button.
     */
    void setClearIcon(const Icon& icon);

    /**
     * @brief Sets the clear icon from a string token or asset path.
     * @param str Icon identifier.
     */
    void setClearIcon(const std::string& str);

    /**
     * @brief Sets the icon bounding box size in dp.
     * @param sizeDp Icon width/height in dp.
     */
    void setIconSize(float sizeDp);

    /**
     * @brief Registers a callback that fires whenever text content changes.
     * @param callback Function taking the new text string as argument.
     */
    void setOnTextChanged(std::function<void(const std::string&)> callback);

    /**
     * @brief Registers a callback that fires when the user presses the Enter/Submit key.
     * @param callback Function taking the current text string as argument.
     */
    void setOnSubmit(std::function<void(const std::string&)> callback);

    /**
     * @brief Registers a callback for focus state changes.
     * @param callback Function taking a boolean indicating focus state (true = gained).
     */
    void setOnFocusChange(std::function<void(bool)> callback);

    /**
     * @brief Registers a callback for trailing icon click events.
     * @param callback Function with no arguments.
     */
    void setOnTrailingIconClick(std::function<void()> callback);

    /**
     * @brief Registers a callback for clear icon click events.
     * @param callback Function with no arguments.
     */
    void setOnClearClick(std::function<void()> callback);

    /**
     * @brief Appends a single ASCII character at the current cursor position.
     * @param c Character to insert.
     * @note Does nothing if read-only or max length reached.
     */
    void appendChar(char c);

    /**
     * @brief Appends a UTF-8 string at the current cursor position.
     * @param str String to insert (newline characters are replaced with spaces).
     */
    void appendString(const std::string& str);

    /**
     * @brief Deletes the character immediately before the cursor, or removes active selection.
     */
    void backspace();

    /**
     * @brief Clears the entire text buffer and resets cursor.
     */
    void clear();

    /**
     * @brief Checks whether any text range is currently selected.
     * @return true if a non-empty selection exists.
     */
    bool hasSelection() const { return m_selectionStart >= 0 && m_selectionEnd >= 0 && m_selectionStart != m_selectionEnd; }

    /**
     * @brief Retrieves the highlighted text substring.
     * @return The selected text, or empty string if no selection.
     */
    std::string getSelectedText() const;

    /**
     * @brief Deletes the currently selected text range.
     */
    void deleteSelection();

    /**
     * @brief Selects all text content.
     */
    void selectAll();

    /**
     * @brief Clears the active text selection.
     */
    void clearSelection();

    // --- View overrides ---
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

    /**
     * @brief Resets hover states when the mouse leaves the view bounds.
     */
    void onMouseLeave() { 
        m_isHovering = false; 
        if (!m_hasFocus && m_error.empty()) m_state = TextFieldState::Normal; 
    }

    bool isFocusable() const override { return !m_readOnly; }

    /**
     * @brief Retrieves the current raw text string.
     * @return Const reference to internal text buffer.
     */
    const std::string& getText() const { return m_text; }

    /**
     * @brief Checks whether the component currently holds keyboard focus.
     * @return true if focused.
     */
    bool isFocused() const { return m_hasFocus; }

private:
    // --- Rendering helpers for each style ---
    void renderOutlined(MaterialShader& shader, MaterialTheme& theme);
    void renderFilled(MaterialShader& shader, MaterialTheme& theme);
    void renderUnderlined(MaterialShader& shader, MaterialTheme& theme);

    // --- Utility functions ---
    size_t getUtf8Length(const std::string& str) const;
    void updateDisplayText();
    void rebuildCharOffsets(MaterialShader& shader, float fontSize);
    void ensureCursorVisible(float maxTextWidth);

    M3Color getBorderColor(const MaterialTheme& theme) const;
    M3Color getLabelColor(const MaterialTheme& theme) const;

    int getCharIndexAtX(float localTextX) const;
    bool isInsideIconPos(float mx, float my, float iconCenterX, float iconCenterY, float hitRadius);

    // --- Data members ---
    std::string m_text;              /**< Raw UTF-8 text content. */
    std::string m_label;             /**< Floating label string. */
    std::string m_hint;              /**< Placeholder hint text. */
    std::string m_error;             /**< Error message (empty when no error). */
    std::string m_displayText;       /**< Text displayed (masked if password mode). */

    TextFieldStyle m_style = TextFieldStyle::Outlined;   /**< Current visual style. */
    TextFieldState m_state = TextFieldState::Normal;     /**< Current interaction/validation state. */

    float m_textSizeDp = 16.0f;      /**< Font size for main text. */
    float m_labelSizeDp = 14.0f;     /**< Font size for floating label. */
    float m_cornerRadius = 12.0f;    /**< Corner rounding radius (dp). */
    float m_iconSizeDp = 20.0f;      /**< Icon bounding square size (dp). */
    int m_maxLength = -1;            /**< Maximum UTF-8 character count (-1 = unlimited). */

    Icon m_leadingIcon;              /**< Left-side icon. */
    Icon m_trailingIcon;             /**< Right-side icon. */
    Icon m_clearIcon;                /**< Clear button icon. */

    bool m_hasLeadingIcon = false;   /**< Whether a leading icon is set. */
    bool m_hasTrailingIcon = false;  /**< Whether a trailing icon is set. */
    bool m_showClearIcon = false;    /**< Whether the clear icon should be visible (focused + non‑empty). */
    bool m_isClearHovered = false;   /**< Whether mouse hovers over clear icon. */
    bool m_isTrailingHovered = false;/**< Whether mouse hovers over trailing icon. */

    bool m_readOnly = false;         /**< Disables text editing. */
    bool m_password = false;         /**< Enables password masking. */
    bool m_hasFocus = false;         /**< Keyboard focus flag. */
    bool m_isHovering = false;       /**< Mouse hover flag. */

    float m_labelAnim = 0.0f;        /**< Floating label animation progress [0..1]. */
    float m_indicatorAnim = 0.0f;    /**< Bottom indicator underline animation progress. */
    float m_cursorBlink = 0.0f;      /**< Cursor blink phase timer (seconds). */
    int m_cursorPos = 0;             /**< Cursor byte index in m_text. */

    float m_scrollOffset = 0.0f;     /**< Horizontal scroll offset in pixels. */
    float m_targetCursorX = -1.0f;   /**< Target cursor X position (for smooth animation). */
    float m_smoothCursorX = -1.0f;   /**< Smoothed cursor X position. */

    float m_preferredHeight = 56.0f; /**< Cached preferred height in dp. */

    int m_selectionStart = -1;       /**< Selection start byte index, -1 if none. */
    int m_selectionEnd = -1;         /**< Selection end byte index. */
    bool m_isMouseSelecting = false; /**< Flag indicating drag selection in progress. */
    float m_cachedTextRenderX = 0.0f;/**< Cached X position where text drawing starts. */

    bool m_offsetsDirty = true;      /**< Flag indicating character offset cache needs rebuild. */
    std::vector<std::pair<int, float>> m_charByteOffsets; /**< (byteIndex, xOffsetFromTextStart) for each character. */

    // --- Callbacks ---
    std::function<void(const std::string&)> m_onTextChanged;
    std::function<void(const std::string&)> m_onSubmit;
    std::function<void(bool)> m_onFocusChange;
    std::function<void()> m_onTrailingIconClick;
    std::function<void()> m_onClearClick;
};

/**
 * @class TextFieldBuilder
 * @brief Fluent builder helper for constructing and configuring TextField instances.
 * 
 * Provides method-chaining syntax for setting all TextField properties before building.
 */
class TextFieldBuilder {
public:
    TextFieldBuilder();

    // --- Property setters ---
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

    // --- Icon setters ---
    TextFieldBuilder& leadingIcon(const Icon& icon);
    TextFieldBuilder& leadingIcon(const std::string& str);
    TextFieldBuilder& trailingIcon(const Icon& icon);
    TextFieldBuilder& trailingIcon(const std::string& str);
    TextFieldBuilder& clearIcon(const Icon& icon);
    TextFieldBuilder& clearIcon(const std::string& str);

    // --- Callback setters ---
    TextFieldBuilder& onTextChanged(std::function<void(const std::string&)> cb);
    TextFieldBuilder& onSubmit(std::function<void(const std::string&)> cb);
    TextFieldBuilder& onFocusChange(std::function<void(bool)> cb);
    TextFieldBuilder& onTrailingClick(std::function<void()> cb);
    TextFieldBuilder& onClearClick(std::function<void()> cb);

    /**
     * @brief Allocates and initializes the configured TextField instance.
     * @return Pointer to a new TextField object on the heap. Ownership is transferred to the caller.
     */
    TextField* build();

private:
    // --- Stored configuration values ---
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
