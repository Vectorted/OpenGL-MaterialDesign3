#ifndef JSON_HPP
#define JSON_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cstdint>

enum class JsonFormat { Compact, Pretty2, Pretty4, Tab };
enum class JsonType { Null, Boolean, Number, String, Array, Object };

class JsonObject;
class JsonArray;

/**
 * @class JsonObject
 * @brief Universal JSON node..
 */
class JsonObject {
public:
    JsonType type = JsonType::Object;
    bool boolValue = false;
    double numberValue = 0.0;
    std::string stringValue;
    std::vector<JsonObject> arrayValue;
    std::map<std::string, JsonObject> objectValue;

    // --- Constructors (No Ambiguity) ---
    JsonObject() : type(JsonType::Object) {}
    JsonObject(std::nullptr_t) : type(JsonType::Null) {}
    JsonObject(bool b) : type(JsonType::Boolean), boolValue(b) {}
    JsonObject(int n) : type(JsonType::Number), numberValue(static_cast<double>(n)) {}
    JsonObject(int64_t n) : type(JsonType::Number), numberValue(static_cast<double>(n)) {}
    JsonObject(double n) : type(JsonType::Number), numberValue(n) {}
    JsonObject(const char* s) : type(JsonType::String), stringValue(s ? s : "") {}
    JsonObject(const std::string& s) : type(JsonType::String), stringValue(s) {}
    JsonObject(std::vector<JsonObject> arr) : type(JsonType::Array), arrayValue(std::move(arr)) {}
    JsonObject(std::map<std::string, JsonObject> obj) : type(JsonType::Object), objectValue(std::move(obj)) {}

    // Static Parse helper
    static JsonObject parse(const std::string& jsonString);

    // --- Type Checkers ---
    bool isNull() const { return type == JsonType::Null; }
    bool isBool() const { return type == JsonType::Boolean; }
    bool isNumber() const { return type == JsonType::Number; }
    bool isString() const { return type == JsonType::String; }
    bool isArray() const { return type == JsonType::Array; }
    bool isObject() const { return type == JsonType::Object; }

    // --- Get Raw Value ---
    std::string asString() const { return stringValue; }
    int asInt() const { return static_cast<int>(numberValue); }
    int64_t asInt64() const { return static_cast<int64_t>(numberValue); }
    double asDouble() const { return numberValue; }
    bool asBool() const { return boolValue; }
    // --- Object (Key-Value) Operations ---
    bool has(const std::string& key) const { return objectValue.find(key) != objectValue.end(); }
    bool isNull(const std::string& key) const {
        auto it = objectValue.find(key);
        return it == objectValue.end() || it->second.isNull();
    }

    JsonObject& put(const std::string& key, const JsonObject& val) {
        type = JsonType::Object;
        objectValue[key] = val;
        return *this;
    }
    JsonObject& put(const std::string& key, const std::string& val) { return put(key, JsonObject(val)); }
    JsonObject& put(const std::string& key, const char* val) { return put(key, JsonObject(val)); }
    JsonObject& put(const std::string& key, int val) { return put(key, JsonObject(val)); }
    JsonObject& put(const std::string& key, int64_t val) { return put(key, JsonObject(val)); }
    JsonObject& put(const std::string& key, double val) { return put(key, JsonObject(val)); }
    JsonObject& put(const std::string& key, bool val) { return put(key, JsonObject(val)); }

    JsonObject& get(const std::string& key) { return objectValue.at(key); }
    const JsonObject& get(const std::string& key) const { return objectValue.at(key); }
    std::string getString(const std::string& key) const { return objectValue.at(key).stringValue; }
    int getInt(const std::string& key) const { return static_cast<int>(objectValue.at(key).numberValue); }
    int64_t getInt64(const std::string& key) const { return static_cast<int64_t>(objectValue.at(key).numberValue); }
    double getDouble(const std::string& key) const { return objectValue.at(key).numberValue; }
    bool getBool(const std::string& key) const { return objectValue.at(key).boolValue; }
    JsonObject getJsonObject(const std::string& key) const { return objectValue.at(key); }

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

    // --- Array Operations ---
    size_t length() const { return arrayValue.size(); }
    size_t size() const { return isArray() ? arrayValue.size() : objectValue.size(); }
    bool isEmpty() const { return isArray() ? arrayValue.empty() : objectValue.empty(); }

    JsonObject& put(const JsonObject& v) {
        type = JsonType::Array;
        arrayValue.push_back(v);
        return *this;
    }
    JsonObject& put(const std::string& v) { return put(JsonObject(v)); }
    JsonObject& put(const char* v) { return put(JsonObject(v)); }
    JsonObject& put(int v) { return put(JsonObject(v)); }
    JsonObject& put(double v) { return put(JsonObject(v)); }
    JsonObject& put(bool v) { return put(JsonObject(v)); }

    JsonObject& get(size_t index) { return arrayValue.at(index); }
    const JsonObject& get(size_t index) const { return arrayValue.at(index); }
    std::string getString(size_t index) const { return arrayValue.at(index).stringValue; }
    int getInt(size_t index) const { return static_cast<int>(arrayValue.at(index).numberValue); }
    double getDouble(size_t index) const { return arrayValue.at(index).numberValue; }
    bool getBool(size_t index) const { return arrayValue.at(index).boolValue; }
    // --- Serialization ---
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
 * @brief Recursive descent parser with comment filtering.
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
        get();
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
        get();
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
        get();
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
 * @brief Dedicated Array helper class.
 */
class JsonArray : public JsonObject {
public:
    JsonArray() { type = JsonType::Array; }
    explicit JsonArray(JsonObject obj) : JsonObject(std::move(obj)) {
        if (!isArray()) type = JsonType::Array;
    }
    static JsonArray parse(const std::string& jsonString) {
        return JsonArray(JsonObject::parse(jsonString));
    }
};

// Aliases
using JSONObject = JsonObject;
using JSONArray  = JsonArray;
using JsonValue  = JsonObject;

/**
 * @class JSON
 * @brief Static utility helper class.
 */
class JSON {
public:
    static std::string toJson(const JsonObject& obj, JsonFormat format = JsonFormat::Compact) { return obj.toJson(format); }
    static std::string toJson(const JsonObject& obj, const std::string& customIndent) { return obj.toJson(customIndent); }
    static JsonObject parse(const std::string& jsonStr) { return JsonObject::parse(jsonStr); }
};

#endif // JSON_HPP