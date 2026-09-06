/**
 * @file TextureLoader.cpp
 * @brief Implementation of the external image texture loading and caching mechanisms.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "TextureLoader.hpp"
#include <glad/glad.h>
#include <map>

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief Static global cache mapping image file paths to their allocated OpenGL texture handles.
 */
static std::map<std::string, unsigned int> g_textureCache;

/**
 * @brief Loads a local image file into an OpenGL 2D texture or retrieves it from cache.
 * 
 * @param path File system path to the image file.
 * @return unsigned int OpenGL texture handle, or 0 if loading failed.
 */
unsigned int loadCustomImageTexture(const std::string& path) {
    if (path.empty()) {
        return 0;
    }

    auto it = g_textureCache.find(path);
    if (it != g_textureCache.end()) {
        return it->second;
    }

    int width = 0;
    int height = 0;
    int channels = 0;
    
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    
    if (!data) {
        return 0;
    }

    unsigned int tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
    g_textureCache[path] = tex;
    
    return tex;
}

/**
 * @brief Deallocates all cached GPU textures and clears the lookup cache.
 */
void clearTextureCache() {
    for (auto& pair : g_textureCache) {
        if (pair.second != 0) {
            glDeleteTextures(1, &pair.second);
        }
    }
    g_textureCache.clear();

#ifdef _WIN32
    // Trim process working set memory back to system on Windows platforms
    SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);
#endif
}