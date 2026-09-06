/**
 * @file MaterialSwitch.hpp
 * @brief Material Design 3 Switch toggle component with kinetic thumb motion, post-landing icon pop-in expansion, and halo effects.
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
#include <string>
#include <functional>

/**
 * @class MaterialSwitch
 * @brief Interactive toggle switch component strictly conforming to Material Design 3 motion specifications with post-landing icon pop-in.
 */
class MaterialSwitch : public View {
public:
    bool m_checked = false;        /**< Current binary toggle state (true for checked, false for unchecked). */
    bool m_hasIcon = false;        /**< Flag indicating if thumb icon rendering is enabled. */
    bool m_themeSelector = false;  /**< Special flag for theme toggle rendering dynamic sun/moon icons. */

    float m_anim = 0.0f;           /**< Normalized horizontal slide transition progress [0.0, 1.0]. */
    float m_animTime = 0.0f;       /**< Elapsed slide animation timer in seconds. */
    float m_animDuration = 0.22f;  /**< Horizontal slide animation duration (220ms). */
    bool m_isAnimating = false;    /**< Flag tracking active horizontal slide transition. */
    float m_animStartVal = 0.0f;   /**< Starting slide progress value for current transition. */
    float m_animTargetVal = 0.0f;  /**< Target slide progress value for current transition. */

    float m_iconAnim = 1.0f;          /**< Scale and alpha progression [0.0, 1.0] for the active icon pop-in. */
    float m_iconPopTime = 0.0f;       /**< Elapsed icon pop-in animation timer in seconds. */
    float m_iconPopDuration = 0.16f;  /**< Icon pop-in animation duration (160ms). */
    bool m_iconPopAnimating = false;  /**< Flag indicating if icon pop-in animation is running. */

    float m_iconSizeDp = -1.0f;    /**< Custom thumb icon dimension override in dp (-1.0f for M3 defaults). */

    Icon m_checkedIcon;            /**< Custom icon descriptor for the checked state. */
    Icon m_uncheckedIcon;          /**< Custom icon descriptor for the unchecked state. */
    std::function<void(bool)> m_onCheckedChangeListener = nullptr; /**< Callback dispatched on toggle state change. */

    /**
     * @brief Constructs a new MaterialSwitch instance with default 52x32 dp dimensions.
     */
    MaterialSwitch();

    /**
     * @brief Updates animation states, manages post-slide icon pop-in progression, and requests redraws.
     * @param dt Delta time in seconds since the previous frame.
     */
    void update(float dt) override;

    /**
     * @brief Renders switch track, animated thumb, ripple halos, and post-landing popped-in icons.
     * @param shader MaterialShader rendering pipeline instance.
     * @param theme Current active theme color tokens.
     */
    void render(MaterialShader& shader, MaterialTheme& theme) override;

    /**
     * @brief Handles mouse click interactions, triggers slide motion, and notifies listeners.
     */
    void onClick() override;

    /**
     * @brief Retrieves preferred layout width in density-independent pixels.
     * @return Preferred width (52 dp).
     */
    float getPreferredWidth() override;

    /**
     * @brief Retrieves preferred layout height in density-independent pixels.
     * @return Preferred height (32 dp).
     */
    float getPreferredHeight() override;

    /**
     * @brief Sets switch toggle state with optional animated motion.
     * @param checked Target toggle state.
     * @param animate true to animate the state change, false for instant transition.
     */
    void setChecked(bool checked, bool animate = true);

    /**
     * @brief Retrieves current toggle state.
     * @return true if checked, false otherwise.
     */
    bool isChecked() const { return m_checked; }

    /**
     * @brief Sets custom icon dimension in dp.
     * @param sizeDp Target icon size in dp.
     */
    void setIconSize(float sizeDp) { m_iconSizeDp = sizeDp; }

    /**
     * @brief Sets custom icon descriptor for the checked state.
     * @param icon Custom Icon structure.
     */
    void setCheckedIcon(const Icon& icon) { m_checkedIcon = icon; }

    /**
     * @brief Sets custom icon for the checked state via token string or file path.
     * @param str Icon token or image file path.
     */
    void setCheckedIcon(const std::string& str) { m_checkedIcon = iconFromString(str); }

    /**
     * @brief Sets custom icon descriptor for the unchecked state.
     * @param icon Custom Icon structure.
     */
    void setUncheckedIcon(const Icon& icon) { m_uncheckedIcon = icon; }

    /**
     * @brief Sets custom icon for the unchecked state via token string or file path.
     * @param str Icon token or image file path.
     */
    void setUncheckedIcon(const std::string& str) { m_uncheckedIcon = iconFromString(str); }

    /**
     * @brief Sets the state change callback listener.
     * @param listener Callback taking the updated boolean checked state.
     */
    void setOnCheckedChangeListener(std::function<void(bool)> listener) {
        m_onCheckedChangeListener = std::move(listener);
    }

    /**
     * @brief Toggles light/dark theme selector mode (rendering dynamic Sun/Moon icons).
     * @param val true to enable theme selector mode.
     */
    void setThemeSelector(bool val) { m_themeSelector = val; }

    /**
     * @brief Enables or disables thumb icon display.
     * @param val true to display thumb icons.
     */
    void setHasIcon(bool val) { m_hasIcon = val; }
};

/**
 * @class MaterialSwitchBuilder
 * @brief Fluent builder helper for configuring and instantiating MaterialSwitch instances.
 */
class MaterialSwitchBuilder {
private:
    bool m_checked = false;                                        /**< Initial toggle state. */
    bool m_hasIcon = false;                                        /**< Flag enabling icon rendering. */
    bool m_themeSelector = false;                                  /**< Flag enabling theme selector mode. */
    float m_iconSizeDp = -1.0f;                                    /**< Configured custom icon size in dp. */
    Icon m_checkedIcon;                                            /**< Configured checked icon. */
    Icon m_uncheckedIcon;                                          /**< Configured unchecked icon. */
    std::function<void(bool)> m_onCheckedChangeListener = nullptr; /**< State change callback listener. */
    float m_marginLeft = 0.0f;                                     /**< Left margin spacing in dp. */
    float m_marginTop = 0.0f;                                      /**< Top margin spacing in dp. */
    float m_marginRight = 0.0f;                                    /**< Right margin spacing in dp. */
    float m_marginBottom = 0.0f;                                   /**< Bottom margin spacing in dp. */

public:
    /**
     * @brief Default builder constructor.
     */
    MaterialSwitchBuilder() = default;

    /**
     * @brief Sets initial checked state.
     * @param val Toggle state.
     * @return Builder reference.
     */
    MaterialSwitchBuilder& checked(bool val);

    /**
     * @brief Sets whether thumb icon is displayed.
     * @param val Flag value.
     * @return Builder reference.
     */
    MaterialSwitchBuilder& hasIcon(bool val);

    /**
     * @brief Sets theme selector mode.
     * @param val Flag value.
     * @return Builder reference.
     */
    MaterialSwitchBuilder& themeSelector(bool val);

    /**
     * @brief Sets custom icon dimension in dp.
     * @param sizeDp Icon size in dp.
     * @return Builder reference.
     */
    MaterialSwitchBuilder& iconSize(float sizeDp);

    /**
     * @brief Sets state change listener callback.
     * @param cb Callback function.
     * @return Builder reference.
     */
    MaterialSwitchBuilder& setOnCheckedChangeListener(std::function<void(bool)> cb);

    /**
     * @brief Sets checked state icon.
     * @param icon Icon structure.
     * @return Builder reference.
     */
    MaterialSwitchBuilder& checkedIcon(const Icon& icon);

    /**
     * @brief Sets checked state icon by name or path.
     * @param str Icon name string.
     * @return Builder reference.
     */
    MaterialSwitchBuilder& checkedIcon(const std::string& str);

    /**
     * @brief Sets unchecked state icon.
     * @param icon Icon structure.
     * @return Builder reference.
     */
    MaterialSwitchBuilder& uncheckedIcon(const Icon& icon);

    /**
     * @brief Sets unchecked state icon by name or path.
     * @param str Icon name string.
     * @return Builder reference.
     */
    MaterialSwitchBuilder& uncheckedIcon(const std::string& str);

    /**
     * @brief Sets four-directional outer layout margins in dp.
     * @param left Left margin in dp.
     * @param top Top margin in dp.
     * @param right Right margin in dp.
     * @param bottom Bottom margin in dp.
     * @return Builder reference.
     */
    MaterialSwitchBuilder& margins(float left, float top, float right, float bottom);
    
    /**
     * @brief Allocates and initializes the configured MaterialSwitch instance.
     * @return Pointer to heap-allocated MaterialSwitch instance.
     */
    MaterialSwitch* build();
};