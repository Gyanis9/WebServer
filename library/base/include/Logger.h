//
// Created by guo on 25-1-2.
//

#ifndef WEBSERVER_LOGGER_H
#define WEBSERVER_LOGGER_H

#include "Timestamp.h"
#include <filesystem>
#include <functional>

class Logger {
public:
    enum LogLevel {
        TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NUM_LOG_LEVELS,
    };

    class SourceFile {
    public:
        template<typename T>
        SourceFile(T &&path):m_path(std::forward<T>(path)), m_filename(m_path.filename().string()) {
        }

        const std::string &filename() const { return m_filename; }

        operator std::string() const { return m_filename; }

    private:
        std::filesystem::path m_path;
        std::string m_filename;
    };

    Logger(SourceFile file, int line);

    Logger(SourceFile file, int line, LogLevel level);

    Logger(SourceFile file, int line, LogLevel level, const char *func);

    Logger(SourceFile file, int line, bool toAbort);

    ~Logger();

    std::ostringstream &stream() { return m_impl.getString(); }

public:
    static LogLevel logLevel();

    static void setLogLevel(LogLevel level);

    using OutputFunc = std::function<void(const char *, int len)>;
    using FlushFunc = std::function<void()>;

    static void setOutput(OutputFunc);

    static void setFlush(FlushFunc);

private:
    class Impl {
    public:
        using LogLevel = Logger::LogLevel;

        Impl() = delete;

        Impl(LogLevel level, int old_errno, const SourceFile &file, int line);

        std::ostringstream &getString() { return m_stream; }

        void formatTime();

        void finish();

        LogLevel level() const { return m_level; }

    private:
        Timestamp m_time;
        std::ostringstream m_stream;
        LogLevel m_level;
        int m_line;
        SourceFile m_basename;
    };

    Impl m_impl;
};

extern Logger::LogLevel global_logLevel;

inline Logger::LogLevel Logger::logLevel() {
    return global_logLevel;
}

#define LOG_TRACE if (Logger::logLevel() <= Logger::TRACE) \
        Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
        Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
        Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()

const char *strerror_tl(int savedError);


#endif //WEBSERVER_LOGGER_H
