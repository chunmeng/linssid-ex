#ifndef _LOGGER_H
#define _LOGGER_H

#include <iosfwd>
#include <ostream>
#include <string>
#include <memory>

enum class LogLevel {
    None,
    Error,
    Warn,
    Info,
    Debug,
    Verbose,
};

class Logger {
public:
    Logger(const std::string& name, LogLevel initialLevel = LogLevel::Error);
    ~Logger();
    Logger(const Logger& loglogger) = delete;
    Logger& operator=(const Logger& loglogger) = delete;
    bool isEnabled(LogLevel level) const;
    void setLevel(LogLevel level);
    LogLevel getLevel() const;
    const std::string& getName() const;
    static LogLevel ToLogLevel(const std::string& lvlStr);
private:
    LogLevel currentLevel_;
    const std::string name_;
};

#define DoLog(logger, level) logger.isEnabled(level) && LogOutputter(logger, level).log()
#define ErrorLog(logger) DoLog(logger, LogLevel::Error)
#define WarnLog(logger) DoLog(logger, LogLevel::Warn)
#define InfoLog(logger) DoLog(logger, LogLevel::Info)
#define DebugLog(logger) DoLog(logger, LogLevel::Debug)
#define VerboseLog(logger) DoLog(logger, LogLevel::Verbose)

// For using boost lexical_cast with LogLevel
std::ostream& operator<<(std::ostream& out, const LogLevel& level);

class LogOutputter {
public:
    LogOutputter(const Logger& log, LogLevel level);
    ~LogOutputter();
    LogOutputter(LogOutputter const& log) = delete;
    LogOutputter& operator=(LogOutputter const& log) = delete;
    std::ostream& log();
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

inline bool Logger::isEnabled(LogLevel level) const
{
    return static_cast<int>(level) <= static_cast<int>(currentLevel_);
}

inline LogLevel Logger::getLevel() const
{
    return currentLevel_;
}

inline const std::string& Logger::getName() const
{
    return name_;
}

#endif // _LOGGER_H
