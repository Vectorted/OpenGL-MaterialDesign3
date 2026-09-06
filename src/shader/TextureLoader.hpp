/**
 * @file TextureLoader.hpp
 * @brief Lightweight utility functions for loading and caching OpenGL 2D textures from image files.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#pragma once

#include <string>

/**
 * @brief Loads a local image file and generates a corresponding OpenGL texture ID.
 * 
 * This function utilizes stb_image internally to parse standard image formats (e.g., PNG, JPG). 
 * It employs an internal caching mechanism; subsequent calls using the exact same file path 
 * will instantly return the cached OpenGL texture ID to bypass duplicate disk I/O and GPU uploads.
 * 
 * @param path The absolute or relative file path to the target image resource.
 * @return unsigned int The allocated OpenGL texture ID. Returns 0 if the asset fails to load.
 */
unsigned int loadCustomImageTexture(const std::string& path);

/**
 * @brief Flushes the internal texture map and deletes all associated OpenGL texture descriptors.
 * 
 * Should be explicitly invoked during application teardown or when a graphical context 
 * switch occurs to avoid VRAM memory leaks.
 */
void clearTextureCache();