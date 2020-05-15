#include "DataLogger.h"
#include "Utils.h"
#include <ctime>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

using namespace std;

// @TODO: Should do something about these extern...
extern struct passwd *realUser;

DataLogger::DataLogger(const string& logPath)
    : logPath_(logPath)
{
}

DataLogger::~DataLogger()
{
    if (logDataStream_.is_open()) logDataStream_.close();
}

void DataLogger::log(const CellData::Vector& data)
{
    if (!logDataStream_.is_open()) { // if data log file is not open...
        // Append timestamp to filepath
        startTime_ = time(NULL);
        char startTimePostfix[64];
        std::strftime(startTimePostfix, 64, "%Y%m%d_%H%M%S", std::localtime(&startTime_));
        logFileName_ = logPath_ + "." + startTimePostfix;
        logDataStream_.open(logFileName_, ios::out | ios::app); // open for write append
        if (!logDataStream_.is_open()) { // Failed to open
            std::cout << "Failed to open " << logFileName_ << " for data logging" << endl;;
            return;
        }
        Utils::waste(chown(logFileName_.c_str(), realUser->pw_uid, realUser->pw_gid));
        chmod(logFileName_.c_str(), 00644);
        writeHeader();
    }

    // now write data
    char nowStr[64], firstStr[64], lastStr[64];
    long entryTime = time(NULL);
    std::strftime(nowStr, 64, "%Y/%m/%d.%H:%M:%S", std::localtime(&entryTime));
    for (unsigned tbi = 0; tbi < data.size(); tbi++) {
        std::strftime(firstStr, 64, "%Y/%m/%d.%H:%M:%S", std::localtime(&data[tbi].firstSeen));
        std::strftime(lastStr, 64, "%Y/%m/%d.%H:%M:%S", std::localtime(&data[tbi].lastSeen));
        logDataStream_ << nowStr << "\t"
                << "\"" << data[tbi].essid << "\"\t"
                << data[tbi].macAddr << "\t"
                << data[tbi].channel << "\t"
                << data[tbi].mode << "\t"
                << data[tbi].protocol << "\t"
                << data[tbi].security << "\t"
                << data[tbi].privacy << "\t"
                << data[tbi].cipher << "\t"
                << data[tbi].frequency << "\t"
                << data[tbi].quality << "\t"
                << data[tbi].signal << "\t"
                << data[tbi].load << "\t"
                << data[tbi].stationCount << "\t"
                << data[tbi].BW << "\t"
                << data[tbi].minSignal << "\t"
                << data[tbi].maxSignal << "\t"
                << data[tbi].cenChan << "\t"
                << firstStr << "\t"
                << lastStr << "\t"
                << "\"" << data[tbi].vendor << "\"" << endl;
    }
}

void DataLogger::writeHeader()
{
    // @TODO: Move this to CellData
    logDataStream_ << "LINSSIDDATALOGVER " << LINSSIDDATALOGVER << "\n";
    logDataStream_ << "Time\tSSID\tMAC\tChannel\tMode\tProtocol\tSecurity\tPrivacy\t\
Cipher\tFrequency\tQuality\tSignal\tLoad\tStationCount\tBW\tMin_Sig\tMax_Sig\tCen_Chan\tFirst_Seen\tLast_Seen\tVendor\n";
}
