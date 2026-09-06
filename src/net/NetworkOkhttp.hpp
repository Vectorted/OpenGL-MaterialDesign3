/**
 * @file NetworkOkhttp.hpp
 * @brief High-performance HTTP/HTTPS client facade conforming to OkHttp patterns powered by libcurl.
 * @author Vectorted (github.com/Vectorted)
 * @details Exposes unified Network::OKHTTP::HTTP and Network::OKHTTP::HTTPS entry
 *          points with native support for form, multipart, JSON, and CORS requests.
 */

#ifndef NETWORK_OKHTTP_HPP
#define NETWORK_OKHTTP_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <thread>
#include <sstream>
#include <iomanip>
#include <utility>
#include <chrono>
#include <algorithm>
#include <curl/curl.h>

#include "../io/Io.hpp"
#include "../io/Json.hpp"

namespace Network {
namespace OKHTTP {

    /**
     * @class HttpUtil
     * @brief Internal helper utilities for URL escaping, headers, and CORS specs.
     * @author Vectorted (github.com/Vectorted)
     */
    class HttpUtil {
    public:
        /**
         * @brief Encodes a string into standard application/x-www-form-urlencoded format.
         * @param value The raw string.
         * @return URL encoded string.
         */
        static std::string urlEncode(const std::string& value) {
            std::ostringstream escaped;
            escaped.fill('0');
            escaped << std::hex;
            for (char c : value) {
                if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
                    escaped << c;
                } else {
                    escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
                }
            }
            return escaped.str();
        }

        /**
         * @brief Generates a random MIME boundary for multipart transmissions.
         * @return Unique boundary string.
         */
        static std::string generateBoundary() {
            return "----WebKitFormBoundary" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        }

        /**
         * @brief Case-insensitive header key comparison / trimming.
         */
        static std::string trim(const std::string& str) {
            size_t first = str.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) return "";
            size_t last = str.find_last_not_of(" \t\r\n");
            return str.substr(first, (last - first + 1));
        }
    };
    
    /**
     * @class Response
     * @brief Encapsulates HTTP execution results and payload readers.
     * @author Vectorted (github.com/Vectorted)
     */
    class Response {
    private:
        int statusCode_ = 0;
        std::string statusMessage_;
        std::map<std::string, std::string> headers_;
        std::string rawBody_;

    public:
        Response(int code, std::string message, std::map<std::string, std::string> headers, std::string body)
            : statusCode_(code), statusMessage_(std::move(message)),
              headers_(std::move(headers)), rawBody_(std::move(body)) {}

        /** @brief Returns HTTP status code. */
        [[nodiscard]] int code() const { return statusCode_; }

        /** @brief Returns true if code is in range [200, 299]. */
        [[nodiscard]] bool isSuccessful() const { return statusCode_ >= 200 && statusCode_ < 300; }

        /** @brief Returns the status message text. */
        [[nodiscard]] const std::string& message() const { return statusMessage_; }

        /** @brief Returns all response headers. */
        [[nodiscard]] const std::map<std::string, std::string>& headers() const { return headers_; }

        /** @brief Returns the raw body as std::string. */
        [[nodiscard]] const std::string& string() const { return rawBody_; }

        /** @brief Direct deserialization to a JsonObject. */
        [[nodiscard]] JsonObject toJson() const { return JsonObject::parse(rawBody_); }

        /** @brief Returns the raw binary payload buffer. */
        [[nodiscard]] std::vector<uint8_t> bytes() const {
            return std::vector<uint8_t>(rawBody_.begin(), rawBody_.end());
        }

        /**
         * @brief Directly persists payload into an Io::File.
         * @param destination The destination file.
         * @return true on success, false on failure.
         */
        bool saveToFile(const Io::File& destination) const {
            return destination.write(rawBody_, IoMode::Binary, false);
        }

        /** @brief Gets a response header value by name (case-insensitive search). */
        [[nodiscard]] std::string header(const std::string& name, const std::string& fallback = "") const {
            std::string lowerName = name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            for (const auto& [k, v] : headers_) {
                std::string currentKey = k;
                std::transform(currentKey.begin(), currentKey.end(), currentKey.begin(), ::tolower);
                if (currentKey == lowerName) return v;
            }
            return fallback;
        }
    };
    
    /**
     * @class RequestBuilder
     * @brief Fluent builder formulating standard and special HTTP/HTTPS requests with curl backend.
     * @author Vectorted (github.com/Vectorted)
     */
    class RequestBuilder {
    protected:
        std::string method_ = "GET";
        std::string url_;
        std::map<std::string, std::string> headers_;
        std::string contentType_;
        std::string rawBody_;

        // Form & Multipart intermediate containers
        std::map<std::string, std::string> urlEncodedForm_;
        struct MultipartFile {
            std::string fieldName;
            std::string fileName;
            std::string fileData;
            std::string mimeType;
        };
        std::vector<std::pair<std::string, std::string>> multipartFields_;
        std::vector<MultipartFile> multipartFiles_;

        int timeoutMs_ = 10000;
        bool isHttps_ = false;
        bool verifySsl_ = true;

    public:
        explicit RequestBuilder(std::string url, std::string method = "GET", bool isHttps = false)
            : method_(std::move(method)), url_(std::move(url)), isHttps_(isHttps) {}

        /** @brief Sets HTTP method explicitly. */
        RequestBuilder& method(const std::string& method) {
            method_ = method;
            return *this;
        }

        /** @brief Sets custom HTTP header. */
        RequestBuilder& header(const std::string& name, const std::string& value) {
            headers_[name] = value;
            return *this;
        }

        /**
         * @brief Injects Cross-Origin Resource Sharing (CORS) headers.
         * @param origin Allowed origin (e.g. "https://client.example.com" or "*").
         * @param allowCredentials Whether to send credentials header.
         */
        RequestBuilder& cors(const std::string& origin = "*", bool allowCredentials = true) {
            headers_["Origin"] = origin;
            headers_["Access-Control-Request-Method"] = method_;
            headers_["Access-Control-Request-Headers"] = "Content-Type, Authorization, X-Requested-With";
            if (allowCredentials) {
                headers_["Access-Control-Allow-Credentials"] = "true";
            }
            return *this;
        }

        /** @brief Sets socket / connection timeout in milliseconds. */
        RequestBuilder& timeout(int milliseconds) {
            timeoutMs_ = milliseconds;
            return *this;
        }
        
        /** @brief Directly sends a JsonObject payload (application/json). */
        RequestBuilder& json(const JsonObject& jsonObject) {
            contentType_ = "application/json; charset=utf-8";
            rawBody_ = jsonObject.toJson(JsonFormat::Compact);
            return *this;
        }

        /** @brief Appends a key-value pair to x-www-form-urlencoded form. */
        RequestBuilder& form(const std::string& key, const std::string& value) {
            urlEncodedForm_[key] = value;
            return *this;
        }

        /** @brief Appends a key-value text part to multipart/form-data. */
        RequestBuilder& part(const std::string& fieldName, const std::string& value) {
            multipartFields_.emplace_back(fieldName, value);
            return *this;
        }

        /**
         * @brief Appends an Io::File to multipart/form-data for upload.
         * @param fieldName The form field key.
         * @param file The file on disk to transmit.
         * @param mimeType Optional file MIME type.
         */
        RequestBuilder& partFile(const std::string& fieldName, const Io::File& file,
                                 const std::string& mimeType = "application/octet-stream") {
            std::string buffer;
            file.read(buffer, IoMode::Binary);
            multipartFiles_.push_back({fieldName, file.getName(), std::move(buffer), mimeType});
            return *this;
        }

        /** @brief Direct raw binary or text stream payload. */
        RequestBuilder& raw(std::string data, std::string contentType = "application/octet-stream") {
            rawBody_ = std::move(data);
            contentType_ = std::move(contentType);
            return *this;
        }

        /** @brief Configures SSL verification (useful for HTTPS internal testing). */
        RequestBuilder& insecure(bool skipVerification = true) {
            verifySsl_ = !skipVerification;
            return *this;
        }
        
    private:
        /** @brief Finalizes and packs the body based on configured form / multipart data. */
        void assemblePayload() {
            if (!urlEncodedForm_.empty()) {
                contentType_ = "application/x-www-form-urlencoded";
                std::string encoded;
                for (auto it = urlEncodedForm_.begin(); it != urlEncodedForm_.end(); ++it) {
                    if (it != urlEncodedForm_.begin()) encoded += "&";
                    encoded += HttpUtil::urlEncode(it->first) + "=" + HttpUtil::urlEncode(it->second);
                }
                rawBody_ = std::move(encoded);
            } else if (!multipartFields_.empty() || !multipartFiles_.empty()) {
                std::string boundary = HttpUtil::generateBoundary();
                contentType_ = "multipart/form-data; boundary=" + boundary;
                std::string body;
                for (const auto& field : multipartFields_) {
                    body += "--" + boundary + "\r\n";
                    body += "Content-Disposition: form-data; name=\"" + field.first + "\"\r\n\r\n";
                    body += field.second + "\r\n";
                }
                for (const auto& file : multipartFiles_) {
                    body += "--" + boundary + "\r\n";
                    body += "Content-Disposition: form-data; name=\"" + file.fieldName + "\"; filename=\"" + file.fileName + "\"\r\n";
                    body += "Content-Type: " + file.mimeType + "\r\n\r\n";
                    body += file.fileData + "\r\n";
                }
                body += "--" + boundary + "--\r\n";
                rawBody_ = std::move(body);
            }
            if (!contentType_.empty()) {
                headers_["Content-Type"] = contentType_;
            }
        }

        // libcurl write callbacks
        static size_t writeBodyCallback(void* contents, size_t size, size_t nmemb, void* userp) {
            size_t totalSize = size * nmemb;
            auto* mem = static_cast<std::string*>(userp);
            mem->append(static_cast<char*>(contents), totalSize);
            return totalSize;
        }

        static size_t writeHeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
            size_t totalSize = size * nitems;
            auto* headers = static_cast<std::map<std::string, std::string>*>(userdata);
            std::string line(buffer, totalSize);
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = HttpUtil::trim(line.substr(0, colon));
                std::string val = HttpUtil::trim(line.substr(colon + 1));
                if (!key.empty()) {
                    (*headers)[key] = val;
                }
            }
            return totalSize;
        }

    public:
        /**
         * @brief Synchronously dispatches the HTTP/HTTPS request using libcurl.
         * @return The received Response.
         */
        Response execute() {
            assemblePayload();

            CURL* curl = curl_easy_init();
            if (!curl) {
                return Response(0, "Failed to initialize curl", {}, "");
            }

            std::string responseBody;
            std::map<std::string, std::string> responseHeaders;
            struct curl_slist* headerList = nullptr;

            // Target URL
            curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());

            // Method configuration
            if (method_ == "POST") {
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, rawBody_.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)rawBody_.size());
            } else if (method_ == "PUT") {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, rawBody_.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)rawBody_.size());
            } else if (method_ == "DELETE") {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
                if (!rawBody_.empty()) {
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, rawBody_.c_str());
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)rawBody_.size());
                }
            } else if (method_ == "OPTIONS") {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");
            } else if (method_ == "HEAD") {
                curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            } else {
                curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
            }

            // Headers
            for (const auto& [k, v] : headers_) {
                std::string h = k + ": " + v;
                headerList = curl_slist_append(headerList, h.c_str());
            }
            if (headerList) {
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
            }

            // SSL verification
            if (!verifySsl_) {
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            } else {
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
            }

            // Follow redirects & Timeouts
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (long)timeoutMs_);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, (long)(timeoutMs_ > 5000 ? 5000 : timeoutMs_));

            // Write callbacks
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBodyCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeaderCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeaders);

            // Execute
            CURLcode res = curl_easy_perform(curl);
            int statusCode = 0;
            std::string statusMsg = "OK";

            if (res == CURLE_OK) {
                long httpCode = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
                statusCode = static_cast<int>(httpCode);
            } else {
                statusCode = 0;
                statusMsg = curl_easy_strerror(res);
            }

            if (headerList) {
                curl_slist_free_all(headerList);
            }
            curl_easy_cleanup(curl);

            return Response(statusCode, statusMsg, responseHeaders, responseBody);
        }

        /**
         * @brief Asynchronously dispatches the request without blocking the UI/GL thread.
         * @param onSuccess Callback invoked on completion.
         * @param onFailure Callback invoked on error.
         */
        void enqueue(std::function<void(Response)> onSuccess,
                     std::function<void(std::string)> onFailure = nullptr) {
            RequestBuilder reqCopy = *this;
            std::thread([req = std::move(reqCopy), onSuccess, onFailure]() {
                try {
                    RequestBuilder workerReq = req;
                    Response res = workerReq.execute();
                    if (onSuccess) onSuccess(res);
                } catch (const std::exception& e) {
                    if (onFailure) onFailure(e.what());
                }
            }).detach();
        }
    };
    
    /**
     * @class HTTP
     * @brief Normalized entry facade for non-secure HTTP operations.
     * @author Vectorted (github.com/Vectorted)
     */
    class HTTP {
    public:
        static RequestBuilder get(const std::string& url) {
            return RequestBuilder(url, "GET", false);
        }

        static RequestBuilder post(const std::string& url) {
            RequestBuilder req(url, "POST", false);
            req.header("X-Requested-With", "OKHTTP-Client");
            return req;
        }

        static RequestBuilder put(const std::string& url) {
            return RequestBuilder(url, "PUT", false);
        }

        static RequestBuilder deleteResource(const std::string& url) {
            return RequestBuilder(url, "DELETE", false);
        }

        /** @brief Preflight OPTIONS request specialized for checking CORS boundaries. */
        static RequestBuilder options(const std::string& url) {
            return RequestBuilder(url, "OPTIONS", false);
        }
    };

    /**
     * @class HTTPS
     * @brief Normalized entry facade for secure SSL/TLS HTTPS operations.
     * @author Vectorted (github.com/Vectorted)
     */
    class HTTPS {
    public:
        static RequestBuilder get(const std::string& url) {
            return RequestBuilder(url, "GET", true);
        }

        static RequestBuilder post(const std::string& url) {
            RequestBuilder req(url, "POST", true);
            req.header("X-Requested-With", "OKHTTP-Client");
            return req;
        }

        static RequestBuilder put(const std::string& url) {
            return RequestBuilder(url, "PUT", true);
        }

        static RequestBuilder deleteResource(const std::string& url) {
            return RequestBuilder(url, "DELETE", true);
        }

        static RequestBuilder options(const std::string& url) {
            return RequestBuilder(url, "OPTIONS", true);
        }
    };

} // namespace OKHTTP
} // namespace Network

#endif // NETWORK_OKHTTP_HPP