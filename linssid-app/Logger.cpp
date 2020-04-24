#include "Logger.h"

#include <boost/lexical_cast.hpp>
#include <istream>
#include <sstream>
#include <iostream>

using namespace std;

Logger::Logger(const string& name, LogLevel initialLevel)
    : currentLevel_(initialLevel)
    , name_(name)
{
}

Logger::~Logger() = default;

void Logger::setLevel(LogLevel level)
{
    currentLevel_ = level;
}

LogLevel Logger::ToLogLevel(const std::string& lvlStr)
{
    LogLevel level = LogLevel::Error;
    if (lvlStr == "None") level = LogLevel::None;
    else if (lvlStr == "Error") level = LogLevel::Error;
    else if (lvlStr == "Warn") level = LogLevel::Warn;
    else if (lvlStr == "Info") level = LogLevel::Info;
    else if (lvlStr == "Debug") level = LogLevel::Debug;
    else if (lvlStr == "Verbose") level = LogLevel::Verbose;
    return level;
}

ostream& operator<<(ostream& os, const LogLevel& level)
{
    switch (level) {
    case LogLevel::None: return os;
    case LogLevel::Error: return os << "Error";
    case LogLevel::Warn: return os << "Warn";
    case LogLevel::Info: return os << "Info";
    case LogLevel::Debug: return os << "Debug";
    case LogLevel::Verbose: return os << "Verbose";
    }
    return os;
}

class LogOutputter::Impl {
public:
    Impl(const Logger& log, LogLevel level);
    ~Impl();
    ostream& log();
private:
    const Logger& log_;
    const LogLevel level_;
    ostringstream logBuf_;
};

LogOutputter::Impl::Impl(const Logger& log, LogLevel level)
    : log_(log)
    , level_(level)
{
}

LogOutputter::Impl::~Impl()
{
    string str = logBuf_.str();
    size_t found = str.find_last_not_of('\n');
    if (found != string::npos)
        str.erase(found + 1);
    std::cout << log_.getName() << ": " << level_ << ": " << str << std::endl;
}

ostream& LogOutputter::Impl::log()
{
    return logBuf_;
}

LogOutputter::LogOutputter(const Logger& log, LogLevel level)
    : impl_(new Impl(log, level))
{
}

LogOutputter::~LogOutputter() = default;

ostream& LogOutputter::log()
{
    return impl_->log();
}
