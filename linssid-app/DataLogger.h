#ifndef _DATALOGGER_H
#define	_DATALOGGER_H

#include "DataStruct.h"
#include <fstream>

class DataLogger {
public:
    DataLogger(const std::string& logPath);
    ~DataLogger();
    void log(const CellData::Vector& data);

private:
    void writeHeader();

private:
    std::string logPath_;
    std::fstream logDataStream_;
    long startTime_ = 0; //!< Used to postfix the file name
    std::string logFileName_;
};

#endif	/* _DATALOGGER_H */