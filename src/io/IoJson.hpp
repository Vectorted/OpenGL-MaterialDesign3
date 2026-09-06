/**
 * @file IoJson.hpp
 * @brief Interoperability bridge between Io::File and JsonObject/JsonArray.
 * @author Vectorted (github.com/Vectorted)
 * @details Provides seamless conversions, directory-to-JSON utilities,
 *          and direct file-based JSON reading and writing.
 */

#ifndef IO_JSON_HPP
#define IO_JSON_HPP

#include "Io.hpp"
#include "Json.hpp"

namespace IoJsonExt {

    /**
     * @brief Converts file metadata and attributes into a JsonObject.
     * @param file The target Io::File instance to inspect.
     * @return A JsonObject containing path, name, exists, isFile, isDirectory,
     *         size, lastModified, canRead, and canWrite properties.
     */
    inline JsonObject toJsonObject(const Io::File& file) {
        JsonObject obj;
        obj.put("path", file.getPath());
        obj.put("name", file.getName());
        obj.put("exists", file.exists());
        if (file.exists()) {
            obj.put("isFile", file.isFile());
            obj.put("isDirectory", file.isDirectory());
            obj.put("size", file.length());
            obj.put("lastModified", file.lastModified());
            obj.put("canRead", file.canRead());
            obj.put("canWrite", file.canWrite());
        }
        return obj;
    }
    
    /**
     * @brief Reads a text file and parses its content directly into a JsonObject.
     * @param file The target file to read from.
     * @param outJson The output JsonObject to receive the parsed structure.
     * @return true if the file read succeeds; false otherwise.
     */
    inline bool readJson(const Io::File& file, JsonObject& outJson) {
        std::string content;
        if (!file.read(content, IoMode::Text)) {
            return false;
        }
        outJson = JsonObject::parse(content);
        return true;
    }

    /**
     * @brief Serializes a JsonObject and writes it directly to the target file.
     * @param file The target file to write to.
     * @param json The JsonObject to serialize.
     * @param format Formatting style (e.g. Compact, Pretty2, Pretty4).
     * @return true if write operation succeeds; false otherwise.
     */
    inline bool writeJson(const Io::File& file, const JsonObject& json, JsonFormat format = JsonFormat::Pretty2) {
        std::string content = json.toJson(format);
        return file.write(content, IoMode::Text, false);
    }

    /**
     * @brief Lists all entry names in a directory as a JsonArray of strings.
     * @param dir The directory to traverse.
     * @return A JsonArray containing file and folder names (e.g., ["a.txt", "sub"]).
     */
    inline JsonArray listJson(const Io::File& dir) {
        JsonArray arr;
        for (const auto& name : dir.list()) {
            arr.put(name);
        }
        return arr;
    }

    /**
     * @brief Lists all entries in a directory as paths or detailed metadata objects.
     * @param dir The directory to traverse.
     * @param detailed If true, returns an array of metadata objects; if false, returns an array of path strings.
     * @return A JsonArray containing paths or detailed object representations.
     */
    inline JsonArray listFilesJson(const Io::File& dir, bool detailed = false) {
        JsonArray arr;
        for (const auto& f : dir.listFiles()) {
            if (detailed) {
                arr.put(toJsonObject(f));
            } else {
                arr.put(f.getPath());
            }
        }
        return arr;
    }
} // namespace IoJsonExt

/**
 * @brief Appends a File path value into an object by key.
 * @param obj The JsonObject to modify.
 * @param key The property key.
 * @param file The File whose path will be stored.
 * @return Reference to the updated JsonObject.
 */
inline JsonObject& putFile(JsonObject& obj, const std::string& key, const Io::File& file) {
    return obj.put(key, file.getPath());
}

/**
 * @brief Appends a File path value into an array node.
 * @param arr The JsonArray (JsonObject) to append to.
 * @param file The File whose path will be appended.
 * @return Reference to the updated array node.
 */
inline JsonObject& putFile(JsonObject& arr, const Io::File& file) {
    return arr.put(file.getPath());
}

/**
 * @class IoJson
 * @brief Static utility facade for high-level Io and Json interoperability.
 * @author Vectorted (github.com/Vectorted)
 */
class IoJson {
public:
    /**
     * @brief Directly parses a file into a JsonObject.
     * @param file The target file.
     * @param out The destination JsonObject.
     * @return true on success, false on read failure.
     */
    static bool read(const Io::File& file, JsonObject& out) {
        return IoJsonExt::readJson(file, out);
    }

    /**
     * @brief Writes a JsonObject directly to a file in specified format.
     * @param file The target file.
     * @param json The JsonObject instance.
     * @param format Serialization formatting option.
     * @return true on success, false on write failure.
     */
    static bool write(const Io::File& file, const JsonObject& json, JsonFormat format = JsonFormat::Pretty2) {
        return IoJsonExt::writeJson(file, json, format);
    }
    
    /**
     * @brief Gathers full file attributes as a JsonObject.
     * @param file The target file.
     * @return JsonObject populated with detailed file attributes.
     */
    static JsonObject inspect(const Io::File& file) {
        return IoJsonExt::toJsonObject(file);
    }

    /**
     * @brief Traverses a directory and returns an array of entry names.
     * @param dir The target directory.
     * @return JsonArray populated with string names.
     */
    static JsonArray list(const Io::File& dir) {
        return IoJsonExt::listJson(dir);
    }

    /**
     * @brief Traverses a directory and returns an array of paths or metadata objects.
     * @param dir The target directory.
     * @param detailed If true, entries are objects with full stats; otherwise string paths.
     * @return JsonArray containing either paths or JSON objects.
     */
    static JsonArray listFiles(const Io::File& dir, bool detailed = false) {
        return IoJsonExt::listFilesJson(dir, detailed);
    }
};

#endif // IO_JSON_HPP