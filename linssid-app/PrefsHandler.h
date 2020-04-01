#ifndef _PREFSHANDLER_H
#define	_PREFSHANDLER_H

#include "Custom.h"
#include <string>

class PrefsHandler {
public:
    struct sSort {
        int column;
        int order;
    };

    struct sMaingeom {
        int x;
        int y;
        int width;
        int height;
    };

    struct sMainsplit {
        int topheight;
        int bottomheight;
    };

    struct sPlotprefs {
        int fntSize;
        int plotlb;
        int plotub;
        bool showgrid;
        bool showLabel;
    };

    struct sDefPref {
        std::string version;
        int colwidth[MAX_TABLE_COLS];
        bool colvis[MAX_TABLE_COLS];    // column visibility
        int visorder[MAX_TABLE_COLS];
        sSort sort;
        sMaingeom maingeom;
        sMainsplit mainsplit;
        int plottab;
        int naptime;
        sPlotprefs plotprefs;
        int logData;
    };

public:
    PrefsHandler(const std::string& filePath);
    virtual ~PrefsHandler();

    void save(const sDefPref& prefData);
    sDefPref load();

private:
    void writeDefault();

private:
    std::string filePath_;
};

#endif	/* _PREFSHANDLER_H */