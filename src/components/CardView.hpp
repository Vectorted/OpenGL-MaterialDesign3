/**
 * @file CardView.hpp
 * @brief Material Design 3 container resolving structural elevation bounds and interaction states.
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

/**
 * @enum CardType
 * @brief Material Design 3 card style variants.
 */
enum class CardType { 
    Elevated, /**< Projects a drop shadow to elevate content above background layers. */
    Filled,   /**< Uses a distinct filled surface variant color with no border. */
    Outlined  /**< Uses standard surface background surrounded by an outline border. */
};

/**
 * @class CardView
 * @brief Container component providing corner rounding, elevation shadows, and optional interaction states.
 */
class CardView : public ViewGroup {
public:
    float corner_radius = 12.0f;             /**< Corner curvature radius in dp. */
    CardType card_type = CardType::Elevated; /**< Visual card variant. */
    bool interactive = false;                /**< Indicates if card responds to hover and press interactions. */

    /** @brief Constructs a CardView with default padding and corner radius. */
    CardView();

    float getPreferredWidth() override;
    float getPreferredHeight() override;
    void render(MaterialShader& renderer, MaterialTheme& theme) override;

    /**
     * @brief Appends a child view node to this card container.
     * 
     * @param child Pointer to child View.
     */
    void addView(View* child);

    /** @brief Sets corner radius in dp. */
    void setCornerRadius(float radius);

    /** @brief Sets card visual variant. */
    void setCardType(CardType type) { card_type = type; }

    /** @brief Enables or disables interactive hover/press states. */
    void setInteractive(bool enabled) { interactive = enabled; }
};

/**
 * @class CardViewBuilder
 * @brief Fluent builder helper for configuring and creating CardView instances.
 */
class CardViewBuilder {
private:
    float m_width = MATCH_PARENT;
    float m_height = MATCH_PARENT;
    float m_marginLeft = 0.0f;
    float m_marginRight = 0.0f;
    float m_marginTop = 0.0f;
    float m_marginBottom = 0.0f;
    float m_paddingLeft = 8.0f;
    float m_paddingRight = 8.0f;
    float m_paddingTop = 8.0f;
    float m_paddingBottom = 8.0f;
    float m_cornerRadius = 12.0f;
    CardType m_type = CardType::Elevated;
    bool m_interactive = false; 

public:
    CardViewBuilder() = default;

    CardViewBuilder& layoutWidth(float w);
    CardViewBuilder& layoutHeight(float h);
    CardViewBuilder& margins(float left, float top, float right, float bottom);
    CardViewBuilder& padding(float left, float top, float right, float bottom);
    CardViewBuilder& cornerRadius(float radius);
    CardViewBuilder& type(CardType t);
    CardViewBuilder& interactive(bool val); 

    /**
     * @brief Allocates and initializes the CardView based on current builder state.
     * 
     * @return CardView* Pointer to heap-allocated CardView instance.
     */
    CardView* build();
};