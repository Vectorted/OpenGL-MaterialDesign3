#
# @file CMakeLists.txt
# @brief Primary CMake build configuration for the Material Design 3 UI Framework.
#        Configures cross-platform GLFW backend compilation, FreeType rasterizer,
#        GLAD OpenGL loader, headless in-tree libcurl engine, and binary asset staging.
#
# Part of the Material 3 OpenGL ES Component Library.
#
# @author Vectorted
# @repository https://github.com/Vectorted
# @license Open-source
# @copyright Copyright (c) 2026 Vectorted. All rights reserved.
#

cmake_minimum_required(VERSION 3.18)

# Enforce modern Windows API target (Windows 10+) before project() and subprojects
if (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00")
    set(CURL_TARGET_WINDOWS_VERSION "0x0A00" CACHE STRING "" FORCE)
endif()

project(Material_Design3_Edition VERSION 1.0.0 LANGUAGES C CXX)

# Enforce C++17 language standard compliance
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable UTF-8 execution character set on MSVC
if (MSVC)
    add_compile_options(/utf-8 /EHsc)
    add_compile_options(/utf-8)
endif()

# FreeType internal build requirement macro definition
add_compile_definitions(FT2_BUILD_LIBRARY)

#
# Section 1: Headless libcurl Subproject Configuration
# Configures core static HTTP/HTTPS client and eliminates all tools, docs, and package lookups
#
set(BUILD_CURL_EXE OFF CACHE INTERNAL "")
set(BUILD_TESTING OFF CACHE INTERNAL "")
set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(CURL_BUILD_TESTING OFF CACHE INTERNAL "")

# Suppress manual and documentation generation to eliminate Perl warnings
set(BUILD_LIBCURL_DOCS OFF CACHE INTERNAL "")
set(BUILD_MISC_DOCS OFF CACHE INTERNAL "")
set(ENABLE_CURL_MANUAL OFF CACHE INTERNAL "")
set(CURL_LINT OFF CACHE INTERNAL "")
set(CURL_DISABLE_INSTALL ON CACHE INTERNAL "")

# Enforce static compilation
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")
set(CURL_STATICLIB ON CACHE INTERNAL "")

# Disable external third-party dependency lookups
set(CURL_USE_PKGCONFIG OFF CACHE INTERNAL "")
set(CURL_USE_CMAKECONFIG OFF CACHE INTERNAL "")
set(CURL_USE_LIBPSL OFF CACHE INTERNAL "")
set(CURL_USE_LIBSSH2 OFF CACHE INTERNAL "")
set(CURL_USE_LIBSSH OFF CACHE INTERNAL "")
set(CURL_USE_GSASL OFF CACHE INTERNAL "")
set(CURL_USE_GSSAPI OFF CACHE INTERNAL "")
set(CURL_ZLIB OFF CACHE INTERNAL "")
set(CURL_BROTLI OFF CACHE INTERNAL "")
set(CURL_ZSTD OFF CACHE INTERNAL "")
set(USE_NGHTTP2 OFF CACHE INTERNAL "")
set(USE_LIBIDN2 OFF CACHE INTERNAL "")
set(ENABLE_ARES OFF CACHE INTERNAL "")

# Enable native Windows Schannel SSL provider
if (WIN32)
    set(CURL_USE_SCHANNEL ON CACHE INTERNAL "")
    set(CURL_WINDOWS_SSPI ON CACHE INTERNAL "")
    set(USE_SCHANNEL ON CACHE INTERNAL "")
    set(USE_WINDOWS_SSPI ON CACHE INTERNAL "")
endif()

# Retain only HTTP and HTTPS protocols for minimal footprint
set(HTTP_ONLY ON CACHE INTERNAL "")
set(CURL_DISABLE_LDAP ON CACHE INTERNAL "")
set(CURL_DISABLE_LDAPS ON CACHE INTERNAL "")
set(CURL_DISABLE_TELNET ON CACHE INTERNAL "")
set(CURL_DISABLE_DICT ON CACHE INTERNAL "")
set(CURL_DISABLE_TFTP ON CACHE INTERNAL "")
set(CURL_DISABLE_RTSP ON CACHE INTERNAL "")
set(CURL_DISABLE_POP3 ON CACHE INTERNAL "")
set(CURL_DISABLE_IMAP ON CACHE INTERNAL "")
set(CURL_DISABLE_SMTP ON CACHE INTERNAL "")
set(CURL_DISABLE_GOPHER ON CACHE INTERNAL "")
set(CURL_DISABLE_MQTT ON CACHE INTERNAL "")

# Register core curl subproject
add_subdirectory(vendor/curl EXCLUDE_FROM_ALL)

#
# Section 2: Core Application Sources
#
file(GLOB_RECURSE MY_SOURCES "src/*.cpp" "src/*.hpp" "src/*.h")

#
# Section 3: GLFW Backend Base Sources
#
set(GLFW_BASE_SOURCES
    vendor/glfw/src/context.c
    vendor/glfw/src/init.c
    vendor/glfw/src/input.c
    vendor/glfw/src/monitor.c
    vendor/glfw/src/vulkan.c
    vendor/glfw/src/window.c
    vendor/glfw/src/osmesa_context.c
    vendor/glfw/src/egl_context.c
    vendor/glfw/src/platform.c
    vendor/glfw/src/null_init.c
    vendor/glfw/src/null_monitor.c
    vendor/glfw/src/null_window.c
    vendor/glfw/src/null_joystick.c
)

#
# Section 4: FreeType Typography Rasterizer Sources
#
set(FREETYPE_SOURCES
    vendor/freetype/src/autofit/autofit.c
    vendor/freetype/src/base/ftbase.c
    vendor/freetype/src/base/ftbbox.c
    vendor/freetype/src/base/ftbdf.c
    vendor/freetype/src/base/ftbitmap.c
    vendor/freetype/src/base/ftcid.c
    vendor/freetype/src/base/ftdebug.c
    vendor/freetype/src/base/ftfstype.c
    vendor/freetype/src/base/ftgasp.c
    vendor/freetype/src/base/ftglyph.c
    vendor/freetype/src/base/ftgxval.c
    vendor/freetype/src/base/ftinit.c
    vendor/freetype/src/base/ftmm.c
    vendor/freetype/src/base/ftotval.c
    vendor/freetype/src/base/ftpatent.c
    vendor/freetype/src/base/ftpfr.c
    vendor/freetype/src/base/ftstroke.c
    vendor/freetype/src/base/ftsynth.c
    vendor/freetype/src/base/ftsystem.c
    vendor/freetype/src/base/fttype1.c
    vendor/freetype/src/base/ftwinfnt.c
    vendor/freetype/src/bdf/bdf.c
    vendor/freetype/src/cache/ftcache.c
    vendor/freetype/src/cff/cff.c
    vendor/freetype/src/cid/type1cid.c
    vendor/freetype/src/gzip/ftgzip.c
    vendor/freetype/src/lzw/ftlzw.c
    vendor/freetype/src/pcf/pcf.c
    vendor/freetype/src/pfr/pfr.c
    vendor/freetype/src/psaux/psaux.c
    vendor/freetype/src/pshinter/pshinter.c
    vendor/freetype/src/psnames/psnames.c
    vendor/freetype/src/raster/raster.c
    vendor/freetype/src/sdf/sdf.c
    vendor/freetype/src/sfnt/sfnt.c
    vendor/freetype/src/smooth/smooth.c
    vendor/freetype/src/svg/svg.c
    vendor/freetype/src/truetype/truetype.c
    vendor/freetype/src/type1/type1.c
    vendor/freetype/src/type42/type42.c
    vendor/freetype/src/winfonts/winfnt.c
)

#
# Section 5: Platform-Specific Backend Sources and Compilation Macros
#
if (WIN32)
    set(GLFW_PLATFORM_SOURCES
        vendor/glfw/src/win32_time.c
        vendor/glfw/src/win32_module.c
        vendor/glfw/src/win32_thread.c
        vendor/glfw/src/win32_init.c
        vendor/glfw/src/win32_joystick.c
        vendor/glfw/src/win32_monitor.c
        vendor/glfw/src/win32_window.c
        vendor/glfw/src/wgl_context.c
    )

    add_compile_definitions(
        _GLFW_WIN32
        _CRT_SECURE_NO_WARNINGS
        CURL_STATICLIB
    )
    list(APPEND MY_SOURCES "platforms/windows/resource.rc")

elseif (UNIX AND NOT APPLE)
    set(GLFW_PLATFORM_SOURCES
        vendor/glfw/src/posix_time.c
        vendor/glfw/src/posix_module.c
        vendor/glfw/src/posix_thread.c
        vendor/glfw/src/x11_init.c
        vendor/glfw/src/x11_monitor.c
        vendor/glfw/src/x11_window.c
        vendor/glfw/src/xkb_unicode.c
        vendor/glfw/src/linux_joystick.c
        vendor/glfw/src/glx_context.c
        vendor/glfw/src/posix_poll.c
    )

    add_compile_definitions(
        _GLFW_X11
        CURL_STATICLIB
    )
endif()

#
# Section 6: Target Executable Definition
#
add_executable(${PROJECT_NAME}
    ${MY_SOURCES}
    ${GLFW_BASE_SOURCES}
    ${GLFW_PLATFORM_SOURCES}
    ${FREETYPE_SOURCES}
    vendor/glad/glad.c
)

# Pure GUI subsystem mode: suppresses console window launch on Windows
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

# Link options for pure GUI subsystem based on toolchain
if (WIN32)
    if (MSVC)
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
    elseif (MINGW)
        target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
    endif()
endif()

#
# Section 7: Target Include Directories
#
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/vendor/curl/include
    ${CMAKE_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_SOURCE_DIR}/vendor/glad
    ${CMAKE_SOURCE_DIR}/vendor/freetype/include
    ${CMAKE_SOURCE_DIR}/vendor/glm
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/components
    ${CMAKE_SOURCE_DIR}/src/event
    ${CMAKE_SOURCE_DIR}/src/layout
    ${CMAKE_SOURCE_DIR}/src/shader
    ${CMAKE_SOURCE_DIR}/src/theme
)

#
# Section 8: Target Link Libraries and Post-Build Asset Staging
#
if (WIN32)
    # Link libcurl static target and native Windows providers
    target_link_libraries(${PROJECT_NAME} PRIVATE
        libcurl_static
        opengl32 gdi32 user32 shell32
        ws2_32 wldap32 crypt32 normaliz bcrypt secur32
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/platforms/windows/icon.ico" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/icon.ico"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/platforms/windows/libEGL.dll" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libEGL.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/platforms/windows/libGLESv2.dll" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libGLESv2.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
        COMMENT "Windows Pipeline: Staging ANGLE runtime libraries and resources."
    )

elseif (UNIX AND NOT APPLE)
    # Link Linux system dependencies and cryptographic providers
    target_link_libraries(${PROJECT_NAME} PRIVATE
        libcurl_static
        m pthread dl rt X11 ssl crypto z
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
        COMMENT "Linux Pipeline: Staging graphical resource bundles."
    )
endif()