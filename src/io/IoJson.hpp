/**
 * @file IoJson.hpp
 * @brief Interoperability bridge between Io::File and JsonObject/JsonArray.
 * 
 * Provides convenient functions to read/write JSON from/to files,
 * convert File objects to JSON representations, and list directories as JSON.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#ifndef IO_JSON_HPP
#define IO_JSON_HPP

#include "Io.hpp"
#include "Json.hpp"

/**
 * @namespace IoJsonExt
 * @brief Extension functions bridging Io and JSON.
 */
namespace IoJsonExt {

    /**
     * @brief Converts a File object to a JSON object with metadata.
     * @param file The File to inspect.
     * @return JsonObject containing path, name, exists, isFile, isDirectory, size, lastModified, canRead, canWrite.
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
     * @brief Reads a JSON file and parses it into a JsonObject.
     * @param file The file to read.
     * @param outJson Output JsonObject.
     * @return true on success (file exists, content valid, and parsed object is not null unless literal "null").
     */
    inline bool readJson(const Io::File& file, JsonObject& outJson) {
        std::string content;
        if (!file.read(content, IoMode::Text)) {
            return false;
        }
        if (content.empty()) return false;

        outJson = JsonObject::parse(content);

        // If content is not the literal "null", but parsing yields null, treat as parse failure.
        bool isLiteralNull = false;
        for (char c : content) {
            if (!std::isspace(static_cast<unsigned char>(c))) {
                if (c == 'n') isLiteralNull = true;
                break;
            }
        }
        if (outJson.isNull() && !isLiteralNull) {
            return false;
        }
        return true;
    }

    /**
     * @brief Writes a JsonObject to a file.
     * @param file Destination file.
     * @param json JsonObject to write.
     * @param format JSON formatting style.
     * @return true on success.
     */
    inline bool writeJson(const Io::File& file, const JsonObject& json, JsonFormat format = JsonFormat::Pretty2) {
        std::string content = json.toJson(format);
        return file.write(content, IoMode::Text, false);
    }

    /**
     * @brief Lists directory entry names as a JSON array of strings.
     * @param dir Directory File.
     * @return JsonArray of entry names (excluding "." and "..").
     */
    inline JsonArray listJson(const Io::File& dir) {
        JsonArray arr;
        for (const auto& name : dir.list()) {
            arr.put(name);
        }
        return arr;
    }

    /**
     * @brief Lists directory entries as a JSON array of paths or detailed objects.
     * @param dir Directory File.
     * @param detailed If true, each entry is a JSON object with metadata; otherwise, just the path string.
     * @return JsonArray of entries.
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
 * @brief Convenience overload to put a File object into a JsonObject as a string path.
 * @param obj Target JsonObject.
 * @param key Key name.
 * @param file File to store.
 * @return Reference to the JsonObject for chaining.
 */
inline JsonObject& putFile(JsonObject& obj, const std::string& key, const Io::File& file) {
    return obj.put(key, file.getPath());
}

/**
 * @brief Convenience overload to put a File into a JsonArray (as path string).
 * @param arr Target JsonArray.
 * @param file File to store.
 * @return Reference to the JsonArray for chaining.
 */
inline JsonObject& putFile(JsonObject& arr, const Io::File& file) {
    return arr.put(file.getPath());
}

/**
 * @class IoJson
 * @brief Static utility class bridging Io files and JSON operations.
 */
class IoJson {
public:
    /**
     * @brief Reads a JSON file and parses it.
     * @param file File to read.
     * @param out Output JsonObject.
     * @return true on success.
     */
    static bool read(const Io::File& file, JsonObject& out) {
        return IoJsonExt::readJson(file, out);
    }

    /**
     * @brief Writes a JsonObject to a file.
     * @param file Destination file.
     * @param json JsonObject to write.
     * @param format JSON formatting style.
     * @return true on success.
     */
    static bool write(const Io::File& file, const JsonObject& json, JsonFormat format = JsonFormat::Pretty2) {
        return IoJsonExt::writeJson(file, json, format);
    }
    
    /**
     * @brief Returns a JSON object describing a file's metadata.
     * @param file File to inspect.
     * @return JsonObject with metadata.
     */
    static JsonObject inspect(const Io::File& file) {
        return IoJsonExt::toJsonObject(file);
    }

    /**
     * @brief Lists directory names as JSON array.
     * @param dir Directory.
     * @return JsonArray of names.
     */
    static JsonArray list(const Io::File& dir) {
        return IoJsonExt::listJson(dir);
    }

    /**
     * @brief Lists directory entries as JSON array of paths or detailed objects.
     * @param dir Directory.
     * @param detailed If true, include full metadata.
     * @return JsonArray.
     */
    static JsonArray listFiles(const Io::File& dir, bool detailed = false) {
        return IoJsonExt::listFilesJson(dir, detailed);
    }
};

#endif // IO_JSON_HPP
