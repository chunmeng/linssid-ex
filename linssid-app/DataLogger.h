#ifndef _DATALOGGER_H
#define	_DATALOGGER_H

class CellData;

class DataLogger {
public:
    void log();

private:
    void writeHeader();
};

#endif	/* _DATALOGGER_H */