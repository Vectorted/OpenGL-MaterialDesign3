/**
 * @file SkeletonText.hpp
 * @brief Placeholder skeleton loading text lines component and builder.
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
 * @class SkeletonText
 * @brief Placeholder element rendering soft rounded preview bars for async loading states.
 */
class SkeletonText : public View {
public:
    /**
     * @brief Constructs a SkeletonText element with defined dimensions.
     * 
     * @param w Bounding width in dp.
     * @param h Bounding height in dp.
     */
    SkeletonText(float w, float h);

    /** @brief Virtual destructor. */
    virtual ~SkeletonText() override = default;

    void render(MaterialShader& renderer, MaterialTheme& theme) override;
};

/**
 * @class SkeletonTextBuilder
 * @brief Fluent builder helper for constructing configured SkeletonText instances.
 */
class SkeletonTextBuilder {
private:
    float m_w = 0.0f;
    float m_h = 0.0f;

public:
    SkeletonTextBuilder() = default;

    /** @brief Sets skeleton placeholder bounding dimensions. */
    SkeletonTextBuilder& size(float w, float h);

    /**
     * @brief Allocates and initializes the configured SkeletonText instance.
     * 
     * @return SkeletonText* Pointer to heap-allocated instance.
     */
    SkeletonText* build();
};