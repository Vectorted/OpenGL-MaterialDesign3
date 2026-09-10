/**
 * @file Io.hpp
 * @brief High-performance cross-platform File and I/O utility library.
 * 
 * Provides a unified API for file system operations, path manipulation,
 * file read/write with text, binary, and hex modes, and directory handling.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#ifndef IO_HPP
#define IO_HPP

#include <string>
#include <vector>
#include <string_view>
#include <cstdio>
#include <cstdint>
#include <sys/stat.h>
#include <sys/types.h>
#include <climits>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
    #include <sys/utime.h>
    #include <direct.h>
    #include <io.h>
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #define PLATFORM_MKDIR(p) _mkdir(p)
    #define PLATFORM_RMDIR(p) _rmdir(p)
    #define PLATFORM_UNLINK(p) _unlink(p)
    #define PLATFORM_GETCWD(buf, len) _getcwd(buf, len)
    #define PLATFORM_ACCESS(p, m) _access(p, (m) == 1 ? 0 : (m))
    #define PLATFORM_FSEEK64 _fseeki64
    #define PLATFORM_FTELL64 _ftelli64
    #ifndef PATH_MAX
        #define PATH_MAX MAX_PATH
    #endif
    #ifndef R_OK
        #define R_OK 4
        #define W_OK 2
        #define X_OK 1
    #endif
#else
    #include <utime.h>
    #include <unistd.h>
    #include <dirent.h>
    #define PLATFORM_MKDIR(p) ::mkdir(p, 0755)
    #define PLATFORM_RMDIR(p) ::rmdir(p)
    #define PLATFORM_UNLINK(p) ::unlink(p)
    #define PLATFORM_GETCWD(buf, len) getcwd(buf, len)
    #define PLATFORM_ACCESS(p, m) access(p, m)
    #define PLATFORM_FSEEK64 fseeko
    #define PLATFORM_FTELL64 ftello
#endif

/**
 * @enum IoMode
 * @brief Specifies the interpretation mode for file read/write operations.
 */
enum class IoMode {
    Text,   /**< Read/write as plain text (no transformation). */
    Binary, /**< Read/write raw binary data. */
    Hex     /**< Read as hex string, write converts data to hex. */
};

/**
 * @class Io
 * @brief Namespace-like container for file I/O utilities.
 * 
 * Contains the nested @ref File class and static convenience functions.
 */
class Io {
public:
    /**
     * @class File
     * @brief Encapsulates a file or directory path with comprehensive filesystem operations.
     * 
     * Supports path manipulation, existence checks, permissions, creation, deletion,
     * reading, writing, and directory listing with cross-platform portability.
     */
    class File {
    private:
        std::string m_path; /**< Normalized path string (uses forward slashes). */

        /**
         * @brief Normalizes a path: converts backslashes to forward slashes and removes trailing slashes,
         *        except for root directories (e.g., "C:/" or "/").
         * @param p Input path string.
         * @return Normalized path.
         */
        static std::string normalize(std::string p) {
            if (p.empty()) return p;
            for (char& c : p) {
                if (c == '\\') c = '/';
            }
            while (p.size() > 1 && p.back() == '/') {
                if (p.size() == 3 && p[1] == ':') break; // Protect "C:/"
                if (p.size() == 1 && p[0] == '/') break; // Protect "/"
                p.pop_back();
            }
            return p;
        }

    public:
        static constexpr char separatorChar = '/';   /**< Path separator character. */
        static constexpr const char* separator = "/";/**< Path separator string. */
        static constexpr char pathSeparatorChar = ':';/**< Path list separator character. */
        static constexpr const char* pathSeparator = ":";/**< Path list separator string. */

        /** @brief Default constructor creates an empty File object. */
        File() = default;

        /**
         * @brief Constructs a File from a path string.
         * @param pathname The path to represent.
         */
        explicit File(const std::string& pathname) : m_path(normalize(pathname)) {}

        /**
         * @brief Constructs a File by joining a parent path and a child name.
         * @param parent Parent directory path.
         * @param child Child file/directory name.
         */
        File(const std::string& parent, const std::string& child) {
            if (parent.empty()) m_path = normalize(child);
            else m_path = normalize(normalize(parent) + "/" + child);
        }

        /**
         * @brief Constructs a File from a parent File and a child name.
         * @param parent Parent File object.
         * @param child Child name.
         */
        File(const File& parent, const std::string& child) : File(parent.getPath(), child) {}

        /** @brief Returns the normalized path string. */
        const std::string& getPath() const { return m_path; }

        /**
         * @brief Returns the last component of the path (file or directory name).
         * @return Name string, or empty if path is empty.
         */
        std::string getName() const {
            if (m_path.empty()) return "";
            size_t idx = m_path.find_last_of('/');
            if (idx == std::string::npos) return m_path;
            return m_path.substr(idx + 1);
        }

        /**
         * @brief Returns the file extension (substring after the last dot).
         * @return Extension string, or empty if none.
         */
        std::string getExtension() const {
            std::string name = getName();
            size_t idx = name.find_last_of('.');
            if (idx == std::string::npos || idx == 0) return "";
            return name.substr(idx + 1);
        }

        /**
         * @brief Returns the parent directory path.
         * @return Parent path, or empty string if none, or "/" for root.
         */
        std::string getParent() const {
            if (m_path.empty()) return "";
            size_t idx = m_path.find_last_of('/');
            if (idx == std::string::npos) return "";
            if (idx == 0) return "/";
            return m_path.substr(0, idx);
        }

        /**
         * @brief Returns a File object representing the parent directory.
         * @return File of the parent, or empty File if no parent.
         */
        File getParentFile() const {
            std::string parent = getParent();
            return parent.empty() ? File("") : File(parent);
        }

        /**
         * @brief Returns the absolute path of this file.
         * @return Absolute path string, or current path if unable to resolve.
         */
        std::string getAbsolutePath() const {
            if (m_path.empty()) return "";
            if (m_path[0] == '/' || (m_path.size() > 1 && m_path[1] == ':')) {
                return m_path;
            }
            char buf[PATH_MAX];
            if (PLATFORM_GETCWD(buf, sizeof(buf))) {
                return normalize(std::string(buf) + "/" + m_path);
            }
            return m_path;
        }

        /**
         * @brief Checks if the path is absolute (starts with '/' or drive letter).
         * @return true if absolute.
         */
        bool isAbsolute() const {
            if (m_path.empty()) return false;
            return (m_path[0] == '/' || (m_path.size() > 1 && m_path[1] == ':'));
        }

        /**
         * @brief Checks if the file or directory exists.
         * @return true if exists.
         */
        bool exists() const {
            struct stat buf;
            return (stat(m_path.c_str(), &buf) == 0);
        }

        /**
         * @brief Checks if the path refers to a regular file.
         * @return true if file.
         */
        bool isFile() const {
            struct stat buf;
            if (stat(m_path.c_str(), &buf) == 0) {
                return (buf.st_mode & S_IFMT) == S_IFREG;
            }
            return false;
        }

        /**
         * @brief Checks if the path refers to a directory.
         * @return true if directory.
         */
        bool isDirectory() const {
            struct stat buf;
            if (stat(m_path.c_str(), &buf) == 0) {
                return (buf.st_mode & S_IFMT) == S_IFDIR;
            }
            return false;
        }

        /**
         * @brief Checks if the file is hidden (name starts with '.' on Unix).
         * @return true if hidden.
         */
        bool isHidden() const {
            std::string name = getName();
            return !name.empty() && name[0] == '.';
        }

        /**
         * @brief Returns the file size in bytes.
         * @return Size, or 0 on error.
         */
        int64_t length() const {
            struct stat buf;
            if (stat(m_path.c_str(), &buf) == 0) {
                return static_cast<int64_t>(buf.st_size);
            }
            return 0;
        }

        /**
         * @brief Returns the last modification time in milliseconds since epoch.
         * @return Timestamp in ms, or 0 on error.
         */
        int64_t lastModified() const {
            struct stat buf;
            if (stat(m_path.c_str(), &buf) == 0) {
                return static_cast<int64_t>(buf.st_mtime) * 1000;
            }
            return 0;
        }
        
        /**
         * @brief Sets the last modification time of the file.
         * @param timeMs Timestamp in milliseconds since epoch.
         * @return true on success.
         */
        bool setLastModified(int64_t timeMs) const {
#if defined(_WIN32) || defined(_MSC_VER)
            struct _utimbuf new_times;
            new_times.actime = static_cast<time_t>(timeMs / 1000);
            new_times.modtime = static_cast<time_t>(timeMs / 1000);
            return (_utime(m_path.c_str(), &new_times) == 0);
#else
            struct utimbuf new_times;
            new_times.actime = static_cast<time_t>(timeMs / 1000);
            new_times.modtime = static_cast<time_t>(timeMs / 1000);
            return (utime(m_path.c_str(), &new_times) == 0);
#endif
        }

        /**
         * @brief Checks if the file is readable.
         * @return true if readable.
         */
        bool canRead() const { return PLATFORM_ACCESS(m_path.c_str(), R_OK) == 0; }

        /**
         * @brief Checks if the file is writable.
         * @return true if writable.
         */
        bool canWrite() const { return PLATFORM_ACCESS(m_path.c_str(), W_OK) == 0; }

        /**
         * @brief Checks if the file is executable.
         * @return true if executable.
         */
        bool canExecute() const { return PLATFORM_ACCESS(m_path.c_str(), X_OK) == 0; }

        /**
         * @brief Makes the file read-only (removes write permissions).
         * @return true on success.
         */
        bool setReadOnly() const {
            struct stat buf;
            if (stat(m_path.c_str(), &buf) != 0) return false;
            auto mode = buf.st_mode & ~0222;
#if defined(_WIN32) || defined(_MSC_VER)
            return _chmod(m_path.c_str(), mode) == 0;
#else
            return chmod(m_path.c_str(), mode) == 0;
#endif
        }

        /**
         * @brief Sets the file writable or read-only.
         * @param writable If true, grants write permission; if false, removes it.
         * @return true on success.
         */
        bool setWritable(bool writable) const {
            struct stat buf;
            if (stat(m_path.c_str(), &buf) != 0) return false;
            auto mode = buf.st_mode;
            if (writable) mode |= 0200;
            else mode &= ~0222;
#if defined(_WIN32) || defined(_MSC_VER)
            return _chmod(m_path.c_str(), mode) == 0;
#else
            return chmod(m_path.c_str(), mode) == 0;
#endif
        }

        /**
         * @brief Creates a new empty file if it does not already exist.
         * @return true if file was created, false if it already exists or creation failed.
         */
        bool createNewFile() const {
            if (exists()) return false;
            FILE* fp = fopen(m_path.c_str(), "wb");
            if (!fp) return false;
            fclose(fp);
            return true;
        }

        /**
         * @brief Deletes the file or empty directory.
         * @return true on success.
         */
        bool deleteFile() const {
            if (isDirectory()) {
                return PLATFORM_RMDIR(m_path.c_str()) == 0;
            }
            return PLATFORM_UNLINK(m_path.c_str()) == 0;
        }

        /**
         * @brief Creates a single directory (fails if parent doesn't exist).
         * @return true on success.
         */
        bool mkdir() const { return PLATFORM_MKDIR(m_path.c_str()) == 0; }

        /**
         * @brief Creates the directory and all necessary parent directories.
         * @return true on success.
         */
        bool mkdirs() const {
            if (exists()) return isDirectory();
            std::string parent = getParent();
            if (!parent.empty() && parent != "/" && !File(parent).exists()) {
                if (!File(parent).mkdirs()) return false;
            }
            return mkdir();
        }

        /**
         * @brief Renames/moves the file to a destination.
         * @param dest Destination File.
         * @return true on success.
         */
        bool renameTo(const File& dest) const {
            return ::rename(m_path.c_str(), dest.getPath().c_str()) == 0;
        }

        /**
         * @brief Renames/moves the file to a destination path.
         * @param destPath Destination path string.
         * @return true on success.
         */
        bool renameTo(const std::string& destPath) const {
            return renameTo(File(destPath));
        }

        /**
         * @brief Recursively deletes the file or directory (and all contents).
         * @return true on success.
         */
        bool deleteRecursively() const {
            if (!exists()) return false;
            if (isDirectory()) {
                for (const auto& f : listFiles()) {
                    f.deleteRecursively();
                }
                return PLATFORM_RMDIR(m_path.c_str()) == 0;
            }
            return PLATFORM_UNLINK(m_path.c_str()) == 0;
        }

        /**
         * @brief Lists the names of entries in the directory (excluding "." and "..").
         * @return Vector of entry names.
         */
        std::vector<std::string> list() const {
            std::vector<std::string> results;
            if (!isDirectory()) return results;
#if defined(_WIN32) || defined(_WIN64)
            WIN32_FIND_DATAA fd;
            HANDLE hFind = FindFirstFileA((m_path + "/*").c_str(), &fd);
            if (hFind == INVALID_HANDLE_VALUE) return results;
            do {
                std::string name = fd.cFileName;
                if (name != "." && name != "..") results.push_back(name);
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
#else
            DIR* dir = opendir(m_path.c_str());
            if (!dir) return results;
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if (name != "." && name != "..") results.push_back(name);
            }
            closedir(dir);
#endif
            return results;
        }

        /**
         * @brief Returns a list of File objects for each entry in the directory.
         * @return Vector of File objects.
         */
        std::vector<File> listFiles() const {
            std::vector<File> results;
            for (const auto& name : list()) {
                results.emplace_back(m_path + "/" + name);
            }
            return results;
        }
        
        /**
         * @brief Reads the entire file content into a string.
         * @param outData Output string.
         * @param mode Read mode (Text, Binary, or Hex). For Hex, outData gets hex string.
         * @return true on success.
         */
        bool read(std::string& outData, IoMode mode = IoMode::Text) const;

        /**
         * @brief Reads the entire file content into a byte vector.
         * @param outBytes Output vector of bytes.
         * @param mode Read mode. If Hex, interprets file as hex and decodes to bytes.
         * @return true on success.
         */
        bool read(std::vector<uint8_t>& outBytes, IoMode mode = IoMode::Binary) const;

        /**
         * @brief Writes a string view to the file.
         * @param data Data to write.
         * @param mode Write mode (Text, Binary, or Hex). If Hex, encodes data as hex string.
         * @param append If true, append to file instead of overwriting.
         * @return true on success.
         */
        bool write(std::string_view data, IoMode mode = IoMode::Text, bool append = false) const;

        /**
         * @brief Writes a byte vector to the file.
         * @param data Byte vector.
         * @param mode Write mode. If Hex, encodes bytes as hex string.
         * @param append If true, append to file.
         * @return true on success.
         */
        bool write(const std::vector<uint8_t>& data, IoMode mode = IoMode::Binary, bool append = false) const;
    };

    // --- Static convenience functions ---

    /** @brief Checks if a path exists. */
    static bool exists(const std::string& path) { return File(path).exists(); }

    /** @brief Returns the size of a file in bytes. */
    static int64_t size(const std::string& path) { return File(path).length(); }

    /** @brief Deletes a file or empty directory. */
    static bool remove(const std::string& path) { return File(path).deleteFile(); }

    /** @brief Creates a directory and all parent directories. */
    static bool mkdirs(const std::string& path) { return File(path).mkdirs(); }

    /**
     * @brief Converts binary data to a hex string.
     * @param data Pointer to input bytes.
     * @param len Number of bytes.
     * @param uppercase If true, use uppercase letters A-F.
     * @return Hex string.
     */
    static std::string toHex(const uint8_t* data, size_t len, bool uppercase = false) {
        static const char hexUpper[] = "0123456789ABCDEF";
        static const char hexLower[] = "0123456789abcdef";
        const char* table = uppercase ? hexUpper : hexLower;
        std::string hex;
        hex.resize(len * 2);
        for (size_t i = 0; i < len; ++i) {
            hex[i * 2] = table[(data[i] >> 4) & 0x0F];
            hex[i * 2 + 1] = table[data[i] & 0x0F];
        }
        return hex;
    }

    /**
     * @brief Parses a hex string into a byte vector (ignores non-hex characters).
     * @param hex Input hex string view.
     * @return Vector of decoded bytes.
     */
    static std::vector<uint8_t> fromHex(std::string_view hex) {
        std::vector<uint8_t> bytes;
        bytes.reserve(hex.size() / 2);
        auto hexVal = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        int high = -1;
        for (char c : hex) {
            int val = hexVal(c);
            if (val == -1) continue;
            if (high == -1) {
                high = val;
            } else {
                bytes.push_back(static_cast<uint8_t>((high << 4) | val));
                high = -1;
            }
        }
        return bytes;
    }

    /**
     * @brief Reads a file into a string (static version).
     * @param path File path.
     * @param outData Output string.
     * @param mode Read mode.
     * @return true on success.
     */
    static bool read(const std::string& path, std::string& outData, IoMode mode = IoMode::Text) {
        FILE* fp = fopen(path.c_str(), "rb");
        if (!fp) return false;
        PLATFORM_FSEEK64(fp, 0, SEEK_END);
        int64_t fileSize = PLATFORM_FTELL64(fp);
        PLATFORM_FSEEK64(fp, 0, SEEK_SET);
        if (fileSize < 0) { fclose(fp); return false; }

        std::vector<uint8_t> rawBuffer(static_cast<size_t>(fileSize));
        if (fileSize > 0) {
            size_t bytesRead = fread(rawBuffer.data(), 1, static_cast<size_t>(fileSize), fp);
            rawBuffer.resize(bytesRead);
        }
        fclose(fp);

        if (mode == IoMode::Hex) {
            outData = toHex(rawBuffer.data(), rawBuffer.size());
        } else {
            outData.assign(reinterpret_cast<const char*>(rawBuffer.data()), rawBuffer.size());
        }
        return true;
    }

    /**
     * @brief Reads a file into a byte vector (static version).
     * @param path File path.
     * @param outBytes Output byte vector.
     * @param mode Read mode. Hex mode decodes hex content.
     * @return true on success.
     */
    static bool read(const std::string& path, std::vector<uint8_t>& outBytes, IoMode mode = IoMode::Binary) {
        if (mode == IoMode::Hex) {
            std::string hexStr;
            if (!read(path, hexStr, IoMode::Text)) return false;
            outBytes = fromHex(hexStr);
            return true;
        }
        FILE* fp = fopen(path.c_str(), "rb");
        if (!fp) return false;
        PLATFORM_FSEEK64(fp, 0, SEEK_END);
        int64_t fileSize = PLATFORM_FTELL64(fp);
        PLATFORM_FSEEK64(fp, 0, SEEK_SET);
        if (fileSize < 0) { fclose(fp); return false; }

        outBytes.resize(static_cast<size_t>(fileSize));
        if (fileSize > 0) {
            size_t bytesRead = fread(outBytes.data(), 1, static_cast<size_t>(fileSize), fp);
            outBytes.resize(bytesRead);
        }
        fclose(fp);
        return true;
    }
    
    /**
     * @brief Writes arbitrary data to a file (static version).
     * @param path File path.
     * @param data Pointer to data.
     * @param len Length in bytes.
     * @param mode Write mode. Hex mode writes hex string representation.
     * @param append If true, append to file.
     * @return true on success.
     */
    static bool write(const std::string& path, const void* data, size_t len, IoMode mode = IoMode::Binary, bool append = false) {
        FILE* fp = fopen(path.c_str(), append ? "ab" : "wb");
        if (!fp) return false;
        if (!data || len == 0) { fclose(fp); return true; }

        if (mode == IoMode::Hex) {
            std::string hex = toHex(reinterpret_cast<const uint8_t*>(data), len);
            fwrite(hex.data(), 1, hex.size(), fp);
        } else {
            fwrite(data, 1, len, fp);
        }
        fclose(fp);
        return true;
    }

    /**
     * @brief Writes a string view to a file (static version).
     * @param path File path.
     * @param data String view.
     * @param mode Write mode.
     * @param append If true, append.
     * @return true on success.
     */
    static bool write(const std::string& path, std::string_view data, IoMode mode = IoMode::Text, bool append = false) {
        if (mode == IoMode::Hex) {
            std::vector<uint8_t> raw = fromHex(data);
            return write(path, raw.data(), raw.size(), IoMode::Binary, append);
        }
        return write(path, data.data(), data.size(), mode, append);
    }

    /**
     * @brief Writes a byte vector to a file (static version).
     * @param path File path.
     * @param data Byte vector.
     * @param mode Write mode.
     * @param append If true, append.
     * @return true on success.
     */
    static bool write(const std::string& path, const std::vector<uint8_t>& data, IoMode mode = IoMode::Binary, bool append = false) {
        return write(path, data.data(), data.size(), mode, append);
    }
};

// --- Inline method implementations for Io::File ---

inline bool Io::File::read(std::string& outData, IoMode mode) const {
    return Io::read(m_path, outData, mode);
}
inline bool Io::File::read(std::vector<uint8_t>& outBytes, IoMode mode) const {
    return Io::read(m_path, outBytes, mode);
}
inline bool Io::File::write(std::string_view data, IoMode mode, bool append) const {
    return Io::write(m_path, data, mode, append);
}
inline bool Io::File::write(const std::vector<uint8_t>& data, IoMode mode, bool append) const {
    return Io::write(m_path, data, mode, append);
}

using File = Io::File;

#endif // IO_HPP
