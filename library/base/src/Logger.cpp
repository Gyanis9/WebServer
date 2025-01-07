//
// Created by guo on 25-1-2.
//

#include "../include/Logger.h"

#include <cstring>

thread_local char t_errnobuf[512];
thread_local char t_time[64];
thread_local time_t t_lastSecond;

const char *strerror_tl(int saveError) {
    return strerror_r(saveError, t_errnobuf, sizeof t_errnobuf);
}


Logger::LogLevel initLogLevel() {
    if (::getenv("Gyanis_LOG_TACRED"))
        return Logger::TRACE;
    else if (::getenv("Gyanis_LOG_DEBUG"))
        return Logger::DEBUG;
    else
        return Logger::INFO;
}

Logger::LogLevel global_logLevel = initLogLevel();

const char *LogLevelName[Logger::NUM_LOG_LEVELS] = {"TACRED", "DEBUG", "INFO", "WARN", "ERROR", "FATAL",};

void defaultOutPut(const char *msg, int len) {
    size_t n = fwrite(msg, 1, len, stdout);
}

void defaultFlush() {
    fflush(stdout);
}

Logger::OutputFunc global_output = defaultOutPut;
Logger::FlushFunc global_flush = defaultFlush;

Logger::Impl::Impl(Logger::LogLevel level, int savedErrno, const Logger::SourceFile &file, int line) :
        m_time(Timestamp::now()),
        m_stream(), m_level(level),
        m_line(line),
        m_basename(file) {
    formatTime();
    m_stream << LogLevelName[level] << " ";
    if (savedErrno != 0) {
        m_stream << strerror_tl(savedErrno) << " (error=" << savedErrno << ")";
    }

}

void Logger::Impl::formatTime() {
    m_stream << m_time.toFormattedString() << " ";
}

void Logger::Impl::finish() {
    m_stream << " - " << m_basename.filename() << ':' << m_line << std::endl;
}

Logger::Logger(Logger::SourceFile file, int line) : m_impl(INFO, 0, file, line) {}

Logger::Logger(Logger::SourceFile file, int line, Logger::LogLevel level) : m_impl(level, 0, file, line) {}

Logger::Logger(Logger::SourceFile file, int line, Logger::LogLevel level, const char *func) : m_impl(level, 0, file,
                                                                                                     line) {
    m_impl.getString() << " " << func << " ";
}

Logger::Logger(Logger::SourceFile file, int line, bool toAbort) : m_impl(toAbort ? FATAL : ERROR, errno, file, line) {}

Logger::~Logger() {
    m_impl.finish();
    global_output(m_impl.getString().str().c_str(), m_impl.getString().str().size());
    if (m_impl.level() == FATAL) {
        global_flush();
        abort();
    }
}

void Logger::setLogLevel(Logger::LogLevel level) {
    global_logLevel = level;
}

void Logger::setOutput(Logger::OutputFunc out) {
    global_output = out;
}

void Logger::setFlush(Logger::FlushFunc flush) {
    global_flush = flush;
}
