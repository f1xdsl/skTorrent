#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <string_view>
#include <ostream>
#include <sstream>
#include <concepts>
#include <type_traits>
#include <ctime>
#include <mutex>
#include <cstdio>
#include <filesystem>
#include <cstdint>

enum class LogLevel
{
    Info,
    Debug,
    Trace,
    Warning,
    Error,
    Critical
};

class Logger
{
public:
    static Logger& instance()
    {
        static Logger inst;
        return inst;
    }

    template <typename... Args>
    static void logInfo(std::string_view module, std::string_view title, Args&&... args)
    {
        logImpl(LogLevel::Info, module, title, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void logDebug(std::string_view module, std::string_view title, Args&&... args)
    {
        logImpl(LogLevel::Debug, module, title, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void logTrace(std::string_view module, std::string_view title, Args&&... args)
    {
        logImpl(LogLevel::Trace, module, title, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void logWarning(std::string_view module, std::string_view title, Args&&... args)
    {
        logImpl(LogLevel::Warning, module, title, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void logError(std::string_view module, std::string_view title, Args&&... args)
    {
        logImpl(LogLevel::Error, module, title, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void logCritical(std::string_view module, std::string_view title, Args&&... args)
    {
        logImpl(LogLevel::Critical, module, title, std::forward<Args>(args)...);
    }

private:
    Logger()
    {
        m_path = defaultLogPath();
        openFile(m_path);
    }

    ~Logger()
    {
        if (m_file)
        {
            std::fflush(m_file);
            std::fclose(m_file);
            m_file = nullptr;
        }
    }

    static std::string timestampForFile()
    {
        std::time_t t = std::time(nullptr);
        std::tm tmv{};
#if defined(_POSIX_VERSION)
        localtime_r(&t, &tmv);
#else
        tmv = *std::localtime(&t);
#endif
        char buf[32]{0};
        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d_%02d-%02d-%02d", tmv.tm_year + 1'900, tmv.tm_mon + 1, tmv.tm_mday,
            tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
        return std::string(buf);
    }

    static std::string defaultLogPath()
    {
#ifdef LOG_DEFAULT_DIR
        std::filesystem::create_directories(LOG_DEFAULT_DIR);
        return std::string(LOG_DEFAULT_DIR) + "/" + timestampForFile() + "_log.log";
#else
        std::filesystem::create_directories("logs");
        return std::string("logs/") + timestampForFile() + "_log.log";
#endif
    }

    void openFile(const std::string& path)
    {
        std::scoped_lock lk(m_fileMutex);
        if (m_file)
        {
            std::fflush(m_file);
            std::fclose(m_file);
            m_file = nullptr;
        }
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
        m_file = std::fopen(path.c_str(), "ab");
    }

    static std::string nowString()
    {
        std::time_t t = std::time(nullptr);
        std::tm tmv{};
#if defined(_POSIX_VERSION)
        localtime_r(&t, &tmv);
#else
        tmv = *std::localtime(&t);
#endif
        char buf[20]{0};
        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", tmv.tm_year + 1'900, tmv.tm_mon + 1, tmv.tm_mday,
            tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
        return std::string(buf);
    }

    static std::string levelToString(LogLevel lvl)
    {
        switch (lvl)
        {
            case LogLevel::Info:     return "I";
            case LogLevel::Debug:    return "D";
            case LogLevel::Trace:    return "T";
            case LogLevel::Warning:  return "W";
            case LogLevel::Error:    return "E";
            case LogLevel::Critical: return "C";
            default:                 return "U";
        }
    }

    static std::string levelColor(LogLevel lvl)
    {
        switch (lvl)
        {
            case LogLevel::Info:     return "\033[32m";
            case LogLevel::Debug:    return "\033[36m";
            case LogLevel::Trace:    return "\033[37m";
            case LogLevel::Warning:  return "\033[33m";
            case LogLevel::Error:    return "\033[31m";
            case LogLevel::Critical: return "\033[41m";
            default:                 return "\033[37m";
        }
    }

    template <typename... Args>
    static void appendMeta(std::ostringstream& line, const std::string& meta, Args&&... args)
    {
        line << meta;
        if constexpr (sizeof...(args) > 0)
        {
            line << " ";
            appendMeta(line, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    static void logImpl(LogLevel lvl, std::string_view module, std::string_view title, Args&&... args)
    {
        std::ostringstream consoleLine;
        consoleLine << nowString();
        consoleLine << " [";
        consoleLine << levelColor(lvl);
        consoleLine << levelToString(lvl);
        consoleLine << "\033[0m";
        consoleLine << "] ";
        consoleLine << '[' << module << "] ";
        consoleLine << title;
        if constexpr (sizeof...(args) > 0)
        {
            consoleLine << ' ';
            appendMeta(consoleLine, std::forward<Args>(args)...);
        }
        consoleLine << '\n';

        std::ostringstream fileLine;
        fileLine << nowString();
        fileLine << " [" << levelToString(lvl) << "] ";
        fileLine << '[' << module << "] ";
        fileLine << title;
        if constexpr (sizeof...(args) > 0)
        {
            fileLine << ' ';
            appendMeta(fileLine, std::forward<Args>(args)...);
        }
        fileLine << '\n';

        auto& inst = Logger::instance();

        {
            std::scoped_lock lk(inst.m_consoleMutex);
            const auto& s = consoleLine.str();
            std::fwrite(s.data(), 1, s.size(), stdout);
            std::fflush(stdout);
        }
        {
            std::scoped_lock lk(inst.m_fileMutex);
            if (inst.m_file)
            {
                const auto& s = fileLine.str();
                std::fwrite(s.data(), 1, s.size(), inst.m_file);
                std::fflush(inst.m_file);
            }
        }
    }

private:
    std::mutex m_consoleMutex;
    std::mutex m_fileMutex;
    std::string m_path;
    std::FILE* m_file = nullptr;
};

template <typename T>
concept Ostreamable = requires(std::ostream& os, const T& v) {
    { os << v } -> std::same_as<std::ostream&>;
};

template <std::integral T>
requires(!std::same_as<T, bool>)
std::string toString(T v)
{
    std::ostringstream oss;
    if constexpr (std::is_signed_v<T>)
    {
        oss << static_cast<long long>(v);
    }
    else
    {
        oss << static_cast<unsigned long long>(v);
    }
    return oss.str();
}

template <typename T>
requires(Ostreamable<T> && (!std::integral<T> || std::same_as<T, bool>))
std::string toString(const T& v)
{
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

template <typename T>
requires std::is_enum_v<T>
std::string toString(T v)
{
    using U = std::underlying_type_t<T>;
    return toString(static_cast<U>(v));
}

static std::string writePair(std::string_view k, std::string_view v)
{
    std::ostringstream oss;
    oss << k << "[" << v << "]";
    return oss.str();
}

#define LOG_MD(key, value) writePair(#key, toString(value))
#define LOG_INFO(module, title, ...) Logger::instance().logInfo(#module, title __VA_OPT__(, ) __VA_ARGS__)
#define LOG_DEBUG(module, title, ...) Logger::instance().logDebug(#module, title __VA_OPT__(, ) __VA_ARGS__)
#define LOG_TRACE(module, title, ...) Logger::instance().logTrace(#module, title __VA_OPT__(, ) __VA_ARGS__)
#define LOG_WARNING(module, title, ...) Logger::instance().logWarning(#module, title __VA_OPT__(, ) __VA_ARGS__)
#define LOG_ERROR(module, title, ...) Logger::instance().logError(#module, title __VA_OPT__(, ) __VA_ARGS__)
#define LOG_CRITICAL(module, title, ...) Logger::instance().logCritical(#module, title __VA_OPT__(, ) __VA_ARGS__)

#endif
