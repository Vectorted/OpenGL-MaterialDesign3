/**
 * @file Json.hpp
 * @brief Lightweight JSON parser and serializer with support for comments, formatting, and full type support.
 * 
 * Provides JsonObject and JsonArray classes with an intuitive API for constructing,
 * parsing, and serializing JSON data. Supports all standard JSON types plus
 * hexadecimal string parsing and comment stripping.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#ifndef JSON_HPP
#define JSON_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cstdint>

/**
 * @enum JsonFormat
 * @brief Formatting options for JSON serialization.
 */
enum class JsonFormat { Compact, Pretty2, Pretty4, Tab };

/**
 * @enum JsonType
 * @brief Discriminator for JsonObject's internal type.
 */
enum class JsonType { Null, Boolean, Number, String, Array, Object };

class JsonObject;
class JsonArray;

/**
 * @class JsonObject
 * @brief Universal JSON node that can represent any JSON value (null, bool, number, string, array, object).
 * 
 * Provides a unified interface for building, accessing, and serializing JSON data.
 * Supports chaining for convenient construction.
 */
class JsonObject {
public:
    JsonType type = JsonType::Object;          /**< Discriminator for the stored type. */
    bool boolValue = false;                    /**< Boolean value (valid when type == Boolean). */
    double numberValue = 0.0;                  /**< Numeric value (valid when type == Number). */
    std::string stringValue;                   /**< String value (valid when type == String). */
    std::vector<JsonObject> arrayValue;        /**< Array elements (valid when type == Array). */
    std::map<std::string, JsonObject> objectValue; /**< Object members (valid when type == Object). */

    // --- Constructors (no ambiguity) ---

    /** @brief Default constructor creates a null object. */
    JsonObject() : type(JsonType::Object) {}

    /** @brief Constructs a null object. */
    JsonObject(std::nullptr_t) : type(JsonType::Null) {}

    /** @brief Constructs a boolean. */
    JsonObject(bool b) : type(JsonType::Boolean), boolValue(b) {}

    /** @brief Constructs a number from int. */
    JsonObject(int n) : type(JsonType::Number), numberValue(static_cast<double>(n)) {}

    /** @brief Constructs a number from int64_t. */
    JsonObject(int64_t n) : type(JsonType::Number), numberValue(static_cast<double>(n)) {}

    /** @brief Constructs a number from double. */
    JsonObject(double n) : type(JsonType::Number), numberValue(n) {}

    /** @brief Constructs a string from const char* (handles null pointer). */
    JsonObject(const char* s) : type(JsonType::String), stringValue(s ? s : "") {}

    /** @brief Constructs a string from std::string. */
    JsonObject(const std::string& s) : type(JsonType::String), stringValue(s) {}

    /** @brief Constructs an array from a vector of JsonObjects. */
    JsonObject(std::vector<JsonObject> arr) : type(JsonType::Array), arrayValue(std::move(arr)) {}

    /** @brief Constructs an object from a map of key-value pairs. */
    JsonObject(std::map<std::string, JsonObject> obj) : type(JsonType::Object), objectValue(std::move(obj)) {}

    /**
     * @brief Parses a JSON string into a JsonObject.
     * @param jsonString Input JSON string.
     * @return Parsed JsonObject (null if parsing fails).
     */
    static JsonObject parse(const std::string& jsonString);

    // --- Type checkers ---

    bool isNull()    const { return type == JsonType::Null; }
    bool isBool()    const { return type == JsonType::Boolean; }
    bool isNumber()  const { return type == JsonType::Number; }
    bool isString()  const { return type == JsonType::String; }
    bool isArray()   const { return type == JsonType::Array; }
    bool isObject()  const { return type == JsonType::Object; }

    // --- Get raw values (for scalar types) ---

    std::string asString() const { return stringValue; }
    int asInt()          const { return static_cast<int>(numberValue); }
    int64_t asInt64()    const { return static_cast<int64_t>(numberValue); }
    double asDouble()    const { return numberValue; }
    bool asBool()        const { return boolValue; }

    // --- Object (key-value) operations ---

    /**
     * @brief Checks if a key exists in the object.
     * @param key Key name.
     * @return true if key exists.
     */
    bool has(const std::string& key) const { return objectValue.find(key) != objectValue.end(); }

    /**
     * @brief Checks if a key is absent or its value is null.
     * @param key Key name.
     * @return true if key missing or value is null.
     */
    bool isNull(const std::string& key) const {
        auto it = objectValue.find(key);
        return it == objectValue.end() || it->second.isNull();
    }

    /**
     * @brief Inserts or updates a key with a JsonObject value.
     * @param key Key name.
     * @param val Value.
     * @return Reference to this object for chaining.
     */
    JsonObject& put(const std::string& key, const JsonObject& val) {
        type = JsonType::Object;
        objectValue[key] = val;
        return *this;
    }

    // Overloads for primitive types.
    JsonObject& put(const std::string& key, const std::string& val) { return put(key, JsonObject(val)); }
    JsonObject& put(const std::string& key, const char* val)       { return put(key, JsonObject(val)); }
    JsonObject& put(const std::string& key, int val)              { return put(key, JsonObject(val)); }
    JsonObject& put(const std::string& key, int64_t val)          { return put(key, JsonObject(val)); }
    JsonObject& put(const std::string& key, double val)           { return put(key, JsonObject(val)); }
    JsonObject& put(const std::string& key, bool val)             { return put(key, JsonObject(val)); }

    /**
     * @brief Retrieves a value by key (non-const).
     * @param key Key name.
     * @return Reference to the JsonObject.
     * @throws std::out_of_range if key not found.
     */
    JsonObject& get(const std::string& key) { return objectValue.at(key); }

    /**
     * @brief Retrieves a value by key (const).
     * @param key Key name.
     * @return Const reference.
     * @throws std::out_of_range if key not found.
     */
    const JsonObject& get(const std::string& key) const { return objectValue.at(key); }

    /** @brief Returns string value for key (assumes type is String). */
    std::string getString(const std::string& key) const { return objectValue.at(key).stringValue; }
    /** @brief Returns int value for key (assumes type is Number). */
    int getInt(const std::string& key) const { return static_cast<int>(objectValue.at(key).numberValue); }
    /** @brief Returns int64_t value for key. */
    int64_t getInt64(const std::string& key) const { return static_cast<int64_t>(objectValue.at(key).numberValue); }
    /** @brief Returns double value for key. */
    double getDouble(const std::string& key) const { return objectValue.at(key).numberValue; }
    /** @brief Returns bool value for key. */
    bool getBool(const std::string& key) const { return objectValue.at(key).boolValue; }
    /** @brief Returns a copy of the JsonObject for a key. */
    JsonObject getJsonObject(const std::string& key) const { return objectValue.at(key); }

    // --- Safe accessors with fallback ---

    std::string optString(const std::string& key, const std::string& fallback = "") const {
        auto it = objectValue.find(key);
        return (it != objectValue.end() && it->second.isString()) ? it->second.stringValue : fallback;
    }
    int optInt(const std::string& key, int fallback = 0) const {
        auto it = objectValue.find(key);
        return (it != objectValue.end() && it->second.isNumber()) ? static_cast<int>(it->second.numberValue) : fallback;
    }
    double optDouble(const std::string& key, double fallback = 0.0) const {
        auto it = objectValue.find(key);
        return (it != objectValue.end() && it->second.isNumber()) ? it->second.numberValue : fallback;
    }
    bool optBool(const std::string& key, bool fallback = false) const {
        auto it = objectValue.find(key);
        return (it != objectValue.end() && it->second.isBool()) ? it->second.boolValue : fallback;
    }

    // --- Array operations (when type is Array) ---

    /** @brief Returns number of elements in array (0 if not array). */
    size_t length() const { return arrayValue.size(); }

    /**
     * @brief Returns size: for array, element count; for object, member count.
     * @return size_t.
     */
    size_t size() const { return isArray() ? arrayValue.size() : objectValue.size(); }

    /** @brief Returns true if container (array or object) is empty. */
    bool isEmpty() const { return isArray() ? arrayValue.empty() : objectValue.empty(); }

    /**
     * @brief Appends a value to the array.
     * @param v Value to append.
     * @return Reference to this object (for chaining).
     */
    JsonObject& put(const JsonObject& v) {
        type = JsonType::Array;
        arrayValue.push_back(v);
        return *this;
    }

    // Primitive overloads for array append.
    JsonObject& put(const std::string& v) { return put(JsonObject(v)); }
    JsonObject& put(const char* v)    { return put(JsonObject(v)); }
    JsonObject& put(int v)           { return put(JsonObject(v)); }
    JsonObject& put(double v)        { return put(JsonObject(v)); }
    JsonObject& put(bool v)          { return put(JsonObject(v)); }

    /**
     * @brief Accesses an array element (non-const).
     * @param index Zero-based index.
     * @return Reference to the element.
     * @throws std::out_of_range.
     */
    JsonObject& get(size_t index) { return arrayValue.at(index); }

    /**
     * @brief Accesses an array element (const).
     * @param index Zero-based index.
     * @return Const reference.
     */
    const JsonObject& get(size_t index) const { return arrayValue.at(index); }

    /** @brief Returns string at array index. */
    std::string getString(size_t index) const { return arrayValue.at(index).stringValue; }
    /** @brief Returns int at array index. */
    int getInt(size_t index) const { return static_cast<int>(arrayValue.at(index).numberValue); }
    /** @brief Returns double at array index. */
    double getDouble(size_t index) const { return arrayValue.at(index).numberValue; }
    /** @brief Returns bool at array index. */
    bool getBool(size_t index) const { return arrayValue.at(index).boolValue; }

    // --- Serialization ---

    /**
     * @brief Serializes the JSON node to a string.
     * @param indentStr Indentation string (empty for compact). Used for pretty printing.
     * @param currentDepth Current nesting depth (for indentation).
     * @return JSON string.
     */
    std::string toJson(const std::string& indentStr = "", int currentDepth = 0) const {
        std::ostringstream ss;
        bool pretty = !indentStr.empty();

        switch (type) {
            case JsonType::Null:    return "null";
            case JsonType::Boolean: return boolValue ? "true" : "false";
            case JsonType::Number: {
                if (numberValue == static_cast<int64_t>(numberValue)) {
                    ss << static_cast<int64_t>(numberValue);
                } else {
                    ss << numberValue;
                }
                return ss.str();
            }
            case JsonType::String: return escapeString(stringValue);
            case JsonType::Array: {
                if (arrayValue.empty()) return "[]";
                ss << "[";
                if (pretty) ss << "\n";
                for (size_t i = 0; i < arrayValue.size(); ++i) {
                    if (pretty) ss << repeat(indentStr, currentDepth + 1);
                    ss << arrayValue[i].toJson(indentStr, currentDepth + 1);
                    if (i + 1 < arrayValue.size()) ss << ",";
                    if (pretty) ss << "\n";
                }
                if (pretty) ss << repeat(indentStr, currentDepth);
                ss << "]";
                return ss.str();
            }
            case JsonType::Object: {
                if (objectValue.empty()) return "{}";
                ss << "{";
                if (pretty) ss << "\n";
                size_t i = 0;
                for (auto const& [k, v] : objectValue) {
                    if (pretty) ss << repeat(indentStr, currentDepth + 1);
                    ss << escapeString(k) << (pretty ? ": " : ":") << v.toJson(indentStr, currentDepth + 1);
                    if (++i < objectValue.size()) ss << ",";
                    if (pretty) ss << "\n";
                }
                if (pretty) ss << repeat(indentStr, currentDepth);
                ss << "}";
                return ss.str();
            }
        }
        return "null";
    }

    /**
     * @brief Serializes with a predefined format.
     * @param format JsonFormat enum.
     * @return JSON string.
     */
    std::string toJson(JsonFormat format) const {
        switch (format) {
            case JsonFormat::Pretty2: return toJson("  ");
            case JsonFormat::Pretty4: return toJson("    ");
            case JsonFormat::Tab:     return toJson("\t");
            case JsonFormat::Compact:
            default:                  return toJson("");
        }
    }

private:
    static std::string repeat(const std::string& s, int n) {
        std::string res;
        for (int i = 0; i < n; ++i) res += s;
        return res;
    }

    static std::string escapeString(const std::string& str) {
        std::ostringstream ss;
        ss << '"';
        for (char c : str) {
            switch (c) {
                case '"':  ss << "\\\""; break;
                case '\\': ss << "\\\\"; break;
                case '\b': ss << "\\b";  break;
                case '\f': ss << "\\f";  break;
                case '\n': ss << "\\n";  break;
                case '\r': ss << "\\r";  break;
                case '\t': ss << "\\t";  break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    } else {
                        ss << c;
                    }
                    break;
            }
        }
        ss << '"';
        return ss.str();
    }
};

/**
 * @class JsonParser
 * @brief Recursive-descent parser for JSON with support for C++-style line and block comments.
 * 
 * Parses a JSON string and returns a JsonObject. Skips whitespace and comments.
 * Implements the full JSON grammar.
 */
class JsonParser {
private:
    std::string m_src;
    size_t m_pos = 0;

    void skipWhitespaceAndComments() {
        while (m_pos < m_src.size()) {
            char c = m_src[m_pos];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                m_pos++;
                continue;
            }
            if (c == '/' && m_pos + 1 < m_src.size() && m_src[m_pos + 1] == '/') {
                m_pos += 2;
                while (m_pos < m_src.size() && m_src[m_pos] != '\n') m_pos++;
                continue;
            }
            if (c == '/' && m_pos + 1 < m_src.size() && m_src[m_pos + 1] == '*') {
                m_pos += 2;
                while (m_pos + 1 < m_src.size() && !(m_src[m_pos] == '*' && m_src[m_pos + 1] == '/')) m_pos++;
                if (m_pos + 1 < m_src.size()) m_pos += 2;
                else m_pos = m_src.size();
                continue;
            }
            break;
        }
    }

    char peek() { skipWhitespaceAndComments(); return m_pos >= m_src.size() ? '\0' : m_src[m_pos]; }
    char get() { skipWhitespaceAndComments(); return m_pos >= m_src.size() ? '\0' : m_src[m_pos++]; }

    std::string parseString() {
        get(); // consume opening quote
        std::string res;
        while (m_pos < m_src.size()) {
            char c = m_src[m_pos++];
            if (c == '"') return res;
            if (c == '\\' && m_pos < m_src.size()) {
                char esc = m_src[m_pos++];
                switch (esc) {
                    case '"':  res += '"'; break;
                    case '\\': res += '\\'; break;
                    case '/':  res += '/'; break;
                    case 'b':  res += '\b'; break;
                    case 'f':  res += '\f'; break;
                    case 'n':  res += '\n'; break;
                    case 'r':  res += '\r'; break;
                    case 't':  res += '\t'; break;
                    default:   res += esc; break;
                }
            } else { res += c; }
        }
        return res;
    }

    JsonObject parseNumber() {
        size_t start = m_pos;
        if (m_src[m_pos] == '-') m_pos++;
        while (m_pos < m_src.size() && (std::isdigit(m_src[m_pos]) || m_src[m_pos] == '.' || m_src[m_pos] == 'e' || m_src[m_pos] == 'E' || m_src[m_pos] == '+' || m_src[m_pos] == '-')) {
            m_pos++;
        }
        return JsonObject(std::stod(m_src.substr(start, m_pos - start)));
    }

    JsonObject parseArray() {
        get(); // consume '['
        std::vector<JsonObject> arr;
        if (peek() == ']') { get(); return JsonObject(arr); }
        while (true) {
            arr.push_back(parseValue());
            char next = peek();
            if (next == ']') { get(); break; }
            if (next == ',') { get(); } else { break; }
        }
        return JsonObject(arr);
    }

    JsonObject parseObject() {
        get(); // consume '{'
        std::map<std::string, JsonObject> obj;
        if (peek() == '}') { get(); return JsonObject(obj); }
        while (true) {
            if (peek() != '"') break;
            std::string key = parseString();
            if (peek() == ':') get();
            obj[key] = parseValue();
            char next = peek();
            if (next == '}') { get(); break; }
            if (next == ',') { get(); } else { break; }
        }
        return JsonObject(obj);
    }

public:
    explicit JsonParser(std::string src) : m_src(std::move(src)) {}

    /**
     * @brief Parses the next JSON value starting from current position.
     * @return JsonObject representing the value.
     */
    JsonObject parseValue() {
        skipWhitespaceAndComments();
        char c = peek();
        if (c == '"') return JsonObject(parseString());
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == 't' && m_src.substr(m_pos, 4) == "true") { m_pos += 4; return JsonObject(true); }
        if (c == 'f' && m_src.substr(m_pos, 5) == "false") { m_pos += 5; return JsonObject(false); }
        if (c == 'n' && m_src.substr(m_pos, 4) == "null") { m_pos += 4; return JsonObject(nullptr); }
        if (c == '-' || std::isdigit(c)) return parseNumber();
        return JsonObject(nullptr);
    }
};

inline JsonObject JsonObject::parse(const std::string& jsonString) {
    JsonParser parser(jsonString);
    return parser.parseValue();
}

/**
 * @class JsonArray
 * @brief Dedicated array helper, derived from JsonObject with an array type preset.
 * 
 * Convenience class for building JSON arrays. Inherits all array-related methods.
 */
class JsonArray : public JsonObject {
public:
    /** @brief Constructs an empty array. */
    JsonArray() { type = JsonType::Array; }

    /**
     * @brief Constructs from a JsonObject, ensuring type is array.
     * @param obj Source object (should be array type; if not, type is forced to array).
     */
    explicit JsonArray(JsonObject obj) : JsonObject(std::move(obj)) {
        if (!isArray()) type = JsonType::Array;
    }

    /**
     * @brief Parses a JSON string and constructs a JsonArray.
     * @param jsonString Input string.
     * @return JsonArray.
     */
    static JsonArray parse(const std::string& jsonString) {
        return JsonArray(JsonObject::parse(jsonString));
    }
};

// Type aliases for convenience
using JSONObject = JsonObject;
using JSONArray  = JsonArray;
using JsonValue  = JsonObject;

/**
 * @class JSON
 * @brief Static utility class providing convenient static methods for JSON operations.
 */
class JSON {
public:
    /**
     * @brief Serializes a JsonObject to string with given format.
     * @param obj JsonObject.
     * @param format Output formatting.
     * @return JSON string.
     */
    static std::string toJson(const JsonObject& obj, JsonFormat format = JsonFormat::Compact) { return obj.toJson(format); }

    /**
     * @brief Serializes a JsonObject with custom indentation.
     * @param obj JsonObject.
     * @param customIndent Indentation string.
     * @return JSON string.
     */
    static std::string toJson(const JsonObject& obj, const std::string& customIndent) { return obj.toJson(customIndent); }

    /**
     * @brief Parses a JSON string into a JsonObject.
     * @param jsonStr Input JSON string.
     * @return Parsed JsonObject.
     */
    static JsonObject parse(const std::string& jsonStr) { return JsonObject::parse(jsonStr); }
};

#endif // JSON_HPP
