#ifndef _PREFSHANDLER_H
#define	_PREFSHANDLER_H

#include <string>

class PrefsHandler {
public:
    struct sSort;
    struct sMaingeom;
    struct sMainsplit;
    struct sPlotprefs;
    struct sDefPref;

public:
    PrefsHandler(const std::string& logPath);
    virtual ~PrefsHandler();

    void writeDefault();

private:
    std::string logPath_;
};

#endif	/* _PREFSHANDLER_H */