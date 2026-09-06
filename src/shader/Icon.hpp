/**
 * @file Icon.hpp
 * @brief Material Design 3 icon definitions and variant-based polymorphic icon container.
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
#include <variant>
#include <unordered_map>
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <iomanip>

/**
 * @enum M3Icon
 * @brief Enumerates standard Material Design 3 icon codepoints.
 */
enum class M3Icon : uint32_t {
    None = 0,               /**< Empty or invalid icon identifier. */

    // Base navigation and general icons
    Home = 0xE88A,          /**< Home navigation icon. */
    Search = 0xE8B6,        /**< Search magnifier icon. */
    Settings = 0xE8B8,      /**< Settings gear icon. */
    Menu = 0xE5D2,          /**< Drawer menu hamburger icon. */
    Close = 0xE5CD,         /**< Close/dismiss cross icon. */
    Cross = 0xE5CD,         /**< Cross symbol alias for Close. */
    Add = 0xE145,           /**< Add/plus symbol icon. */
    Check = 0xE5CA,         /**< Checkmark confirmation icon. */
    ArrowBack = 0xE5C4,     /**< Leftward navigation arrow. */
    ArrowForward = 0xE5C8,  /**< Rightward navigation arrow. */

    // Chevron and expansion icons
    ChevronUp = 0xE5CE,     /**< Upward pointing chevron. */
    ChevronDown = 0xE5CF,   /**< Downward pointing chevron. */
    ChevronLeft = 0xE5CB,   /**< Leftward pointing chevron. */
    ChevronRight = 0xE5CC,  /**< Rightward pointing chevron. */
    ExpandLess = 0xE5CE,    /**< Collapse panel indicator icon. */
    ExpandMore = 0xE5CF,    /**< Expand panel indicator icon. */

    // Warning and status icons
    Warning = 0xE002,       /**< Warning triangle icon. */
    Alert = 0xE002,         /**< Alert indicator icon. */
    PriorityHigh = 0xE645,  /**< High priority exclamation mark icon. */
    Exclamation = 0xE645,   /**< Exclamation mark icon. */
    Error = 0xE000,         /**< Filled error symbol icon. */
    ErrorOutline = 0xE001,  /**< Outlined error symbol icon. */

    // General action and entity icons
    Edit = 0xE254,          /**< Pencil edit icon. */
    Info = 0xE88E,          /**< Information circle icon. */
    Sun = 0xE518,           /**< Sun brightness/light mode icon. */
    Moon = 0xE3A4,          /**< Crescent moon/dark mode icon. */
    Diamond = 0xE3EA,       /**< Diamond accent icon. */
    Person = 0xE7FD,        /**< User profile icon. */
    Message = 0xE0C9,       /**< Chat message bubble icon. */
    Grid = 0xE8F0,          /**< Dashboard grid layout icon. */
    Favorite = 0xE87D,      /**< Favorite heart icon. */
    Star = 0xE838,          /**< Rating star icon. */
    Share = 0xE80D,         /**< Share branch icon. */
    Delete = 0xE872,        /**< Trash bin deletion icon. */
    Refresh = 0xE5D5,       /**< Circular refresh arrow icon. */
    MoreVert = 0xE5D4,      /**< Vertical three-dots overflow icon. */
    MoreHoriz = 0xE5D3,     /**< Horizontal three-dots overflow icon. */
    Copy = 0xE14D,          /**< Clipboard copy icon. */
    Paste = 0xE14F,         /**< Clipboard paste icon. */
    Cut = 0xE14B,           /**< Scissors cut icon. */
    Print = 0xE8AD,         /**< Printer device icon. */
    Save = 0xE161,          /**< Floppy disk save icon. */
    Undo = 0xE166,          /**< History step undo icon. */
    Redo = 0xE15A,          /**< History step redo icon. */
    ZoomIn = 0xE8FF,        /**< Magnifier zoom in icon. */
    ZoomOut = 0xE900,       /**< Magnifier zoom out icon. */
    SettingsBrightness = 0xE8B8, /**< Brightness settings icon. */

    // Extended Material 3 built-in icons
    Lock = 0xE897,          /**< Security lock icon. */
    Unlock = 0xE898,        /**< Security unlock icon. */
    Visibility = 0xE8F4,    /**< Eye visibility enabled icon. */
    VisibilityOff = 0xE8F5, /**< Eye visibility disabled icon. */
    Download = 0xF090,      /**< Download arrow icon. */
    Upload = 0xF09B,        /**< Upload arrow icon. */
    Play = 0xE037,          /**< Media play icon. */
    Pause = 0xE034,         /**< Media pause icon. */
    VolumeUp = 0xE04D,      /**< Audio volume high icon. */
    VolumeOff = 0xE04F,     /**< Audio volume muted icon. */
    Folder = 0xE2C7,        /**< File system folder icon. */
    File = 0xE873,          /**< Generic document file icon. */
    Bell = 0xE7F4,          /**< Notification bell icon. */
    Mail = 0xE0BE,          /**< Email envelope icon. */
    Phone = 0xE0CD,         /**< Telephone handset icon. */
    Camera = 0xE412,        /**< Photo camera icon. */
    Image = 0xE3F4,         /**< Picture frame icon. */
    Location = 0xE55F,      /**< Map pin location icon. */
    ShoppingCart = 0xE881,  /**< Shopping cart icon. */
    Filter = 0xE152,        /**< Content filter funnel icon. */
    Sort = 0xE164,          /**< Order sorting lines icon. */
    Dashboard = 0xE871,     /**< Dashboard analytics icon. */
    Code = 0xE86F,          /**< Code bracket syntax icon. */
    Terminal = 0xE99A,      /**< Command terminal prompt icon. */
    ThumbUp = 0xE8DC,       /**< Like thumb up icon. */
    History = 0xE889,       /**< Clock history record icon. */
    Cloud = 0xE2BD,         /**< Cloud storage icon. */
    Mic = 0xE029            /**< Audio microphone input icon. */
};

/**
 * @brief Converts an M3Icon enum to its corresponding 32-bit Unicode codepoint.
 * 
 * @param icon The target M3Icon enum.
 * @return uint32_t The underlying UTF-32 codepoint value.
 */
inline uint32_t toUnicode(M3Icon icon) {
    return static_cast<uint32_t>(icon);
}

/**
 * @brief Parses a string identifier into its corresponding M3Icon enumerator.
 * 
 * Supports prefixes such as "icon:warning", "m3:home", hex codes, and named aliases.
 * 
 * @param str The string representation or token name.
 * @return M3Icon The parsed icon identifier, or M3Icon::None if unresolved.
 */
inline M3Icon parseM3Icon(const std::string& str) {
    static const std::unordered_map<std::string, M3Icon> map = {
        {"home", M3Icon::Home},
        {"search", M3Icon::Search},
        {"settings", M3Icon::Settings},
        {"menu", M3Icon::Menu},
        {"close", M3Icon::Close},
        {"cross", M3Icon::Cross},
        {"add", M3Icon::Add},
        {"check", M3Icon::Check},
        {"arrow_back", M3Icon::ArrowBack},
        {"arrow_forward", M3Icon::ArrowForward},

        // Chevron aliases
        {"chevron_up", M3Icon::ChevronUp},
        {"chevron_down", M3Icon::ChevronDown},
        {"chevron_left", M3Icon::ChevronLeft},
        {"chevron_right", M3Icon::ChevronRight},
        {"expand_less", M3Icon::ExpandLess},
        {"expand_more", M3Icon::ExpandMore},

        // Alert and warning aliases
        {"warning", M3Icon::Warning},
        {"alert", M3Icon::Alert},
        {"priority_high", M3Icon::PriorityHigh},
        {"exclamation", M3Icon::Exclamation},
        {"error", M3Icon::Error},
        {"error_outline", M3Icon::ErrorOutline},

        // Standard actions and assets
        {"edit", M3Icon::Edit},
        {"info", M3Icon::Info},
        {"sun", M3Icon::Sun},
        {"moon", M3Icon::Moon},
        {"diamond", M3Icon::Diamond},
        {"person", M3Icon::Person},
        {"message", M3Icon::Message},
        {"grid", M3Icon::Grid},
        {"favorite", M3Icon::Favorite},
        {"star", M3Icon::Star},
        {"share", M3Icon::Share},
        {"delete", M3Icon::Delete},
        {"refresh", M3Icon::Refresh},
        {"more_vert", M3Icon::MoreVert},
        {"more_horiz", M3Icon::MoreHoriz},
        {"copy", M3Icon::Copy},
        {"paste", M3Icon::Paste},
        {"cut", M3Icon::Cut},
        {"print", M3Icon::Print},
        {"save", M3Icon::Save},
        {"undo", M3Icon::Undo},
        {"redo", M3Icon::Redo},
        {"zoom_in", M3Icon::ZoomIn},
        {"zoom_out", M3Icon::ZoomOut},
        {"lock", M3Icon::Lock},
        {"unlock", M3Icon::Unlock},
        {"visibility", M3Icon::Visibility},
        {"visibility_off", M3Icon::VisibilityOff},
        {"download", M3Icon::Download},
        {"upload", M3Icon::Upload},
        {"play", M3Icon::Play},
        {"pause", M3Icon::Pause},
        {"volume_up", M3Icon::VolumeUp},
        {"volume_off", M3Icon::VolumeOff},
        {"folder", M3Icon::Folder},
        {"file", M3Icon::File},
        {"bell", M3Icon::Bell},
        {"mail", M3Icon::Mail},
        {"phone", M3Icon::Phone},
        {"camera", M3Icon::Camera},
        {"image", M3Icon::Image},
        {"location", M3Icon::Location},
        {"shopping_cart", M3Icon::ShoppingCart},
        {"filter", M3Icon::Filter},
        {"sort", M3Icon::Sort},
        {"dashboard", M3Icon::Dashboard},
        {"code", M3Icon::Code},
        {"terminal", M3Icon::Terminal},
        {"thumb_up", M3Icon::ThumbUp},
        {"history", M3Icon::History},
        {"cloud", M3Icon::Cloud},
        {"mic", M3Icon::Mic}
    };

    std::string key = str;
    if (key.rfind("icon:", 0) == 0) {
        key = key.substr(5);
    } else if (key.rfind("m3:", 0) == 0) {
        key = key.substr(3);
    }

    auto it = map.find(key);
    if (it != map.end()) {
        return it->second;
    }
    if (!key.empty()) {
        try {
            uint32_t code = std::stoul(key, nullptr, 16);
            return static_cast<M3Icon>(code);
        }
        catch (...) {}
    }
    return M3Icon::None;
}

/**
 * @struct IconNone
 * @brief Represents an empty or uninitialized icon placeholder.
 */
struct IconNone {};

/**
 * @struct IconM3
 * @brief Wraps a standard Material Design 3 built-in icon or codepoint.
 */
struct IconM3 {
    M3Icon icon; /**< The active M3Icon enumerator. */

    /** @brief Constructs a default empty IconM3. */
    IconM3() : icon(M3Icon::None) {}

    /**
     * @brief Constructs an IconM3 with a specified icon enumerator.
     * @param i The M3Icon value.
     */
    IconM3(M3Icon i) : icon(i) {}

    /**
     * @brief Constructs an IconM3 by parsing an icon name or identifier.
     * @param str The icon string token or alias.
     */
    IconM3(const std::string& str) : icon(parseM3Icon(str)) {}

    /**
     * @brief Retrieves the UTF-32 codepoint of the icon.
     * @return uint32_t Unicode codepoint value.
     */
    uint32_t unicode() const { return toUnicode(icon); }

    /**
     * @brief Checks if the icon contains a valid enumerator.
     * @return true if valid, false otherwise.
     */
    bool isValid() const { return icon != M3Icon::None; }
};

/**
 * @struct IconFile
 * @brief Represents an icon loaded from an external file path.
 */
struct IconFile {
    std::string path; /**< Local file path or prefixed token string. */

    /** @brief Constructs a default empty IconFile. */
    IconFile() = default;

    /**
     * @brief Constructs an IconFile pointing to a resource path.
     * @param p The path to the image resource.
     */
    IconFile(const std::string& p) : path(p) {}

    /**
     * @brief Verifies whether the path string is non-empty.
     * @return true if the path is not empty, false otherwise.
     */
    bool isValid() const { return !path.empty(); }

    /**
     * @brief Checks whether the path string begins with an icon schema prefix.
     * @return true if prefixed with "icon:" or "m3:".
     */
    bool isIconPrefix() const {
        return path.rfind("icon:", 0) == 0 || path.rfind("m3:", 0) == 0;
    }

    /**
     * @brief Attempts to convert the file path token into an M3Icon enum if prefixed.
     * @return M3Icon Parsed icon enum or M3Icon::None.
     */
    M3Icon toM3() const {
        if (isIconPrefix()) return parseM3Icon(path);
        return M3Icon::None;
    }
};

/**
 * @struct IconTexture
 * @brief Represents an icon using a raw pre-allocated OpenGL texture descriptor.
 */
struct IconTexture {
    unsigned int textureId = 0; /**< Raw OpenGL texture descriptor handle. */

    /** @brief Constructs an uninitialized IconTexture. */
    IconTexture() = default;

    /**
     * @brief Constructs an IconTexture wrapping a GPU texture handle.
     * @param id The OpenGL texture handle.
     */
    IconTexture(unsigned int id) : textureId(id) {}

    /**
     * @brief Checks if the texture handle is valid.
     * @return true if textureId is non-zero, false otherwise.
     */
    bool isValid() const { return textureId != 0; }
};

/**
 * @struct IconVector
 * @brief Represents an icon rendered through internal vector SDF algorithms.
 */
struct IconVector {
    int vectorId = 0; /**< Internal procedural SDF vector geometry index. */

    /** @brief Constructs an uninitialized IconVector. */
    IconVector() = default;

    /**
     * @brief Constructs an IconVector with a specific internal vector geometry ID.
     * @param id The SDF vector geometry identifier.
     */
    IconVector(int id) : vectorId(id) {}

    /**
     * @brief Checks if the vector geometry ID is valid.
     * @return true if vectorId is non-zero, false otherwise.
     */
    bool isValid() const { return vectorId != 0; }
};

/**
 * @typedef Icon
 * @brief Polymorphic container representing any supported icon source variant.
 */
using Icon = std::variant<IconNone, IconM3, IconFile, IconTexture, IconVector>;

/**
 * @brief Creates an Icon variant containing an M3Icon enum.
 * @param icon The M3Icon enumerator.
 * @return Icon variant holding IconM3.
 */
inline Icon M3IconValue(M3Icon icon) { return IconM3(icon); }

/**
 * @brief Creates an Icon variant by parsing an icon name.
 * @param name The icon string name or alias.
 * @return Icon variant holding IconM3.
 */
inline Icon M3IconValue(const std::string& name) { return IconM3(name); }

/**
 * @brief Creates an Icon variant pointing to a file asset.
 * @param path The file path to the icon image.
 * @return Icon variant holding IconFile.
 */
inline Icon FileIcon(const std::string& path) { return IconFile(path); }

/**
 * @brief Creates an Icon variant holding a GPU texture handle.
 * @param tex The OpenGL texture descriptor ID.
 * @return Icon variant holding IconTexture.
 */
inline Icon TextureIcon(unsigned int tex) { return IconTexture(tex); }

/**
 * @brief Creates an Icon variant holding an internal vector geometry ID.
 * @param id The vector procedural SDF ID.
 * @return Icon variant holding IconVector.
 */
inline Icon VectorIcon(int id) { return IconVector(id); }

/**
 * @brief Checks if the icon variant is empty (holds IconNone).
 * @param icon The target Icon variant.
 * @return true if empty, false otherwise.
 */
inline bool isIconEmpty(const Icon& icon) { return std::holds_alternative<IconNone>(icon); }

/**
 * @brief Checks if the icon variant contains an M3 built-in icon.
 * @param icon The target Icon variant.
 * @return true if holding IconM3, false otherwise.
 */
inline bool isIconM3(const Icon& icon) { return std::holds_alternative<IconM3>(icon); }

/**
 * @brief Checks if the icon variant contains an external file path.
 * @param icon The target Icon variant.
 * @return true if holding IconFile, false otherwise.
 */
inline bool isIconFile(const Icon& icon) { return std::holds_alternative<IconFile>(icon); }

/**
 * @brief Checks if the icon variant contains an OpenGL texture handle.
 * @param icon The target Icon variant.
 * @return true if holding IconTexture, false otherwise.
 */
inline bool isIconTexture(const Icon& icon) { return std::holds_alternative<IconTexture>(icon); }

/**
 * @brief Checks if the icon variant contains an internal vector geometry ID.
 * @param icon The target Icon variant.
 * @return true if holding IconVector, false otherwise.
 */
inline bool isIconVector(const Icon& icon) { return std::holds_alternative<IconVector>(icon); }

/**
 * @brief Factory function converting an arbitrary string into an appropriate Icon variant.
 * 
 * Resolves URI prefixes, system file paths, and known token names.
 * 
 * @param str The raw string descriptor.
 * @return Icon The resolved polymorphic Icon variant.
 */
inline Icon iconFromString(const std::string& str) {
    if (str.empty()) return IconNone();
    if (str.rfind("icon:", 0) == 0 || str.rfind("m3:", 0) == 0) {
        return IconM3(str);
    }
    if (str.find('.') != std::string::npos ||
        str.find('/') != std::string::npos ||
        str.find('\\') != std::string::npos) {
        return IconFile(str);
    }
    M3Icon m3 = parseM3Icon(str);
    if (m3 != M3Icon::None) {
        return IconM3(m3);
    }
    return IconFile(str);
}

/**
 * @brief Serializes an Icon variant into a human-readable debug string.
 * 
 * @param icon The target Icon variant.
 * @return std::string Formatted descriptive string.
 */
inline std::string iconToString(const Icon& icon) {
    if (std::holds_alternative<IconNone>(icon)) return "None";
    if (std::holds_alternative<IconM3>(icon)) {
        auto& m3 = std::get<IconM3>(icon);
        std::stringstream ss;
        ss << "Icon:0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << static_cast<uint32_t>(m3.icon);
        return ss.str();
    }
    if (std::holds_alternative<IconFile>(icon)) return "File:" + std::get<IconFile>(icon).path;
    if (std::holds_alternative<IconTexture>(icon)) return "Texture:" + std::to_string(std::get<IconTexture>(icon).textureId);
    if (std::holds_alternative<IconVector>(icon)) return "Vector:" + std::to_string(std::get<IconVector>(icon).vectorId);
    return "Unknown";
}