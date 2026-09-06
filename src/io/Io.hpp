/**
 * @file Io.hpp
 * @brief High-performance cross-platform File and I/O utility library.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
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
    #define PLATFORM_GETCWD(buf, len) _getcwd(buf, len)
    #define PLATFORM_ACCESS(p, m) _access(p, (m) == 1 ? 0 : (m))
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
    #define PLATFORM_GETCWD(buf, len) getcwd(buf, len)
    #define PLATFORM_ACCESS(p, m) access(p, m)
#endif

enum class IoMode {
    Text,
    Binary,
    Hex
};

/**
 * @class Io
 * @brief Universal File and Byte Stream I/O Operations Facade.
 */
class Io {
public:
    /**
     * @class File
     * @brief Object-oriented file system entity representing files and directories.
     */
    class File {
    private:
        std::string m_path;

        static std::string normalize(std::string p) {
            if (p.empty()) return p;
            for (char& c : p) {
                if (c == '\\') c = '/';
            }
            while (p.size() > 1 && p.back() == '/') {
                p.pop_back();
            }
            return p;
        }

    public:
        static constexpr char separatorChar = '/';
        static constexpr const char* separator = "/";
        static constexpr char pathSeparatorChar = ':';
        static constexpr const char* pathSeparator = ":";

        File() = default;
        explicit File(const std::string& pathname) : m_path(normalize(pathname)) {}
        File(const std::string& parent, const std::string& child) {
            if (parent.empty()) m_path = normalize(child);
            else m_path = normalize(normalize(parent) + "/" + child);
        }
        File(const File& parent, const std::string& child) : File(parent.getPath(), child) {}

        const std::string& getPath() const { return m_path; }
        std::string getName() const {
            if (m_path.empty()) return "";
            size_t idx = m_path.find_last_of('/');
            if (idx == std::string::npos) return m_path;
            return m_path.substr(idx + 1);
        }

        std::string getExtension() const {
            std::string name = getName();
            size_t idx = name.find_last_of('.');
            if (idx == std::string::npos || idx == 0) return "";
            return name.substr(idx + 1);
        }

        std::string getParent() const {
            if (m_path.empty()) return "";
            size_t idx = m_path.find_last_of('/');
            if (idx == std::string::npos) return "";
            if (idx == 0) return "/";
            return m_path.substr(0, idx);
        }

        File getParentFile() const {
            std::string parent = getParent();
            return parent.empty() ? File("") : File(parent);
        }

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

        bool isAbsolute() const {
            if (m_path.empty()) return false;
            return (m_path[0] == '/' || (m_path.size() > 1 && m_path[1] == ':'));
        }

        bool exists() const {
            struct stat buf;
            return (stat(m_path.c_str(), &buf) == 0);
        }

        bool isFile() const {
            struct stat buf;
            if (stat(m_path.c_str(), &buf) == 0) {
                return (buf.st_mode & S_IFMT) == S_IFREG;
            }
            return false;
        }

        bool isDirectory() const {
            struct stat buf;
            if (stat(m_path.c_str(), &buf) == 0) {
                return (buf.st_mode & S_IFMT) == S_IFDIR;
            }
            return false;
        }

        bool isHidden() const {
            std::string name = getName();
            return !name.empty() && name[0] == '.';
        }

        int64_t length() const {
            struct stat buf;
            if (stat(m_path.c_str(), &buf) == 0) {
                return static_cast<int64_t>(buf.st_size);
            }
            return 0;
        }

        int64_t lastModified() const {
            struct stat buf;
            if (stat(m_path.c_str(), &buf) == 0) {
                return static_cast<int64_t>(buf.st_mtime) * 1000;
            }
            return 0;
        }
        
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

        bool canRead() const { return PLATFORM_ACCESS(m_path.c_str(), R_OK) == 0; }
        bool canWrite() const { return PLATFORM_ACCESS(m_path.c_str(), W_OK) == 0; }
        bool canExecute() const { return PLATFORM_ACCESS(m_path.c_str(), X_OK) == 0; }

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

        bool createNewFile() const {
            if (exists()) return false;
            FILE* fp = fopen(m_path.c_str(), "wb");
            if (!fp) return false;
            fclose(fp);
            return true;
        }

        bool deleteFile() const { return ::remove(m_path.c_str()) == 0; }
        bool mkdir() const { return PLATFORM_MKDIR(m_path.c_str()) == 0; }

        bool mkdirs() const {
            if (exists()) return isDirectory();
            std::string parent = getParent();
            if (!parent.empty() && parent != "/" && !File(parent).exists()) {
                if (!File(parent).mkdirs()) return false;
            }
            return mkdir();
        }

        bool renameTo(const File& dest) const {
            return ::rename(m_path.c_str(), dest.getPath().c_str()) == 0;
        }
        bool renameTo(const std::string& destPath) const {
            return renameTo(File(destPath));
        }

        bool deleteRecursively() const {
            if (!exists()) return false;
            if (isFile()) return deleteFile();
            if (isDirectory()) {
                for (const auto& f : listFiles()) {
                    f.deleteRecursively();
                }
                return deleteFile();
            }
            return false;
        }

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

        std::vector<File> listFiles() const {
            std::vector<File> results;
            for (const auto& name : list()) {
                results.emplace_back(m_path + "/" + name);
            }
            return results;
        }
        
        bool read(std::string& outData, IoMode mode = IoMode::Text) const;
        bool read(std::vector<uint8_t>& outBytes, IoMode mode = IoMode::Binary) const;
        bool write(std::string_view data, IoMode mode = IoMode::Text, bool append = false) const;
        bool write(const std::vector<uint8_t>& data, IoMode mode = IoMode::Binary, bool append = false) const;
    };

    static bool exists(const std::string& path) { return File(path).exists(); }
    static int64_t size(const std::string& path) { return File(path).length(); }
    static bool remove(const std::string& path) { return File(path).deleteFile(); }
    static bool mkdirs(const std::string& path) { return File(path).mkdirs(); }

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

    static bool read(const std::string& path, std::string& outData, IoMode mode = IoMode::Text) {
        FILE* fp = fopen(path.c_str(), "rb");
        if (!fp) return false;
        fseek(fp, 0, SEEK_END);
        long fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
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

    static bool read(const std::string& path, std::vector<uint8_t>& outBytes, IoMode mode = IoMode::Binary) {
        if (mode == IoMode::Hex) {
            std::string hexStr;
            if (!read(path, hexStr, IoMode::Text)) return false;
            outBytes = fromHex(hexStr);
            return true;
        }
        FILE* fp = fopen(path.c_str(), "rb");
        if (!fp) return false;
        fseek(fp, 0, SEEK_END);
        long fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (fileSize < 0) { fclose(fp); return false; }

        outBytes.resize(static_cast<size_t>(fileSize));
        if (fileSize > 0) {
            size_t bytesRead = fread(outBytes.data(), 1, static_cast<size_t>(fileSize), fp);
            outBytes.resize(bytesRead);
        }
        fclose(fp);
        return true;
    }
    
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

    static bool write(const std::string& path, std::string_view data, IoMode mode = IoMode::Text, bool append = false) {
        if (mode == IoMode::Hex) {
            std::vector<uint8_t> raw = fromHex(data);
            return write(path, raw.data(), raw.size(), IoMode::Binary, append);
        }
        return write(path, data.data(), data.size(), mode, append);
    }

    static bool write(const std::string& path, const std::vector<uint8_t>& data, IoMode mode = IoMode::Binary, bool append = false) {
        return write(path, data.data(), data.size(), mode, append);
    }
};

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