/**
 * @file MaterialComponents.hpp
 * @brief Unified aggregation header providing declarative builder APIs for all Material 3 UI components.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

// 1. Layout Containers
#include "../layout/LinearLayout.hpp"
#include "../layout/ScrollView.hpp"

// 2. Interactive Input Controls
#include "TextView.hpp"
#include "TextField.hpp"
#include "Button.hpp"
#include "MaterialSwitch.hpp"
#include "SkeletonText.hpp"
#include "CardView.hpp"
#include "FloatingActionButton.hpp"
#include "MaterialSegmentedButton.hpp"
#include "Checkbox.hpp"
#include "RadioButton.hpp"
#include "Chip.hpp"

// 3. Overlays and Navigation
#include "NavigationBar.hpp"
#include "NavigationDrawer.hpp"
#include "Snackbar.hpp"
#include "MaterialDialog.hpp"
#include "NotificationOverlay.hpp"

// 4. Data Presentation and Charts
#include "ListView.hpp"
#include "MediaSlider.hpp"
#include "MaterialSearch.hpp"
#include "MaterialPieChart.hpp"
#include "DataTable.hpp"
#include "MaterialChart.hpp"

// 5. Temporal and Detachable Media Components
#include "TimePickerDial.hpp"
#include "DatePicker.hpp"
#include "FloatingImageView.hpp"

// 6. Shader and Texture Utilities
#include "../shader/stb_image.h"

/**
 * @class MaterialDialogBuilder
 * @brief Declarative fluent builder for configuring and instantiating modal dialogs.
 */
class MaterialDialogBuilder {
private:
    MaterialDialog* m_dialog;
public:
    /** @brief Constructs a MaterialDialogBuilder with empty default fields. */
    MaterialDialogBuilder() {
        m_dialog = new MaterialDialog("", "");
    }

    /** @brief Sets dialog title text. */
    MaterialDialogBuilder& title(const std::string& t) {
        m_dialog->setTitle(t); 
        return *this;
    }

    /** @brief Sets dialog message body content text. */
    MaterialDialogBuilder& content(const std::string& c) {
        m_dialog->setContent(c); 
        return *this;
    }

    /**
     * @brief Configures cancel and confirm action button labels and callbacks.
     * 
     * @param cancelText Text label for cancel button.
     * @param onCancel Callback invoked on cancel.
     * @param confirmText Text label for confirm button.
     * @param onConfirm Callback invoked on confirm.
     */
    MaterialDialogBuilder& actions(const std::string& cancelText, std::function<void()> onCancel,
        const std::string& confirmText, std::function<void()> onConfirm) {
        m_dialog->setActions(cancelText, onCancel, confirmText, onConfirm);
        return *this;
    }

    /**
     * @brief Returns the constructed MaterialDialog instance.
     * 
     * @return MaterialDialog* Pointer to allocated dialog.
     */
    MaterialDialog* build() { return m_dialog; }
};

/**
 * @class Components
 * @brief Factory class providing shorthand fluent builders for Material 3 UI elements.
 */
class Components {
public:
    /** @brief Creates a search bar builder. */
    static MaterialSearchBuilder searchBar() { return MaterialSearchBuilder(); }

    /** @brief Creates a button builder. */
    static ButtonBuilder button() { return ButtonBuilder(); }

    /** @brief Creates a floating action button builder. */
    static FloatingActionButtonBuilder fabButton() { return FloatingActionButtonBuilder(); }

    /** @brief Creates a switch toggle builder. */
    static MaterialSwitchBuilder switchButton() { return MaterialSwitchBuilder(); }

    /** @brief Creates a skeleton placeholder text builder. */
    static SkeletonTextBuilder skeleton() { return SkeletonTextBuilder(); }

    /** @brief Creates a card container builder. */
    static CardViewBuilder card() { return CardViewBuilder(); }

    /** @brief Allocates a new ScrollView container instance. */
    static ScrollView* scrollView() { return new ScrollView(); }

    /** @brief Creates a text view builder. */
    static TextViewBuilder textView() { return TextViewBuilder(); }

    /** @brief Creates a text input field builder. */
    static TextFieldBuilder textField() { return TextFieldBuilder(); }

    /** @brief Creates a modal dialog builder. */
    static MaterialDialogBuilder dialog() { return MaterialDialogBuilder(); }
};