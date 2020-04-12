#ifndef _DATASTRUCT_H
#define	_DATASTRUCT_H

#include "Custom.h"
#include <QColor>
#include <memory>
#include <vector>

// Struct to hold summary data of scan
struct Stats {
    int total5GBss = -1;
    int total2GBss = -1;
    int totalOpen = -1;  // Total BSS with open network
    int totalHidden = -1;    // Total hidden BSS

    void reset() {
        total5GBss = 0;
        total2GBss = 0;
        totalOpen = 0;
        totalHidden = 0;
    }

    std::string toString() {
        return "5G: " + ((total5GBss < 0) ? "-" : std::to_string(total5GBss))
            + "\t2.4G: " + ((total2GBss < 0) ? "-" : std::to_string(total2GBss))
            + "\tOpen: " + ((totalOpen < 0) ? "-" : std::to_string(totalOpen))
            + "\tHidden: " + ((totalHidden < 0) ? "-" : std::to_string(totalHidden));
    }
};

struct History {
    // This structure is created to record the history of each cell. It's a ring buffer.
    // Sample buffers are TWICE the size needed so that we can efficiently give the
    // plotting package contiguous arrays of size MAX_SAMPLES. Each sample is entered
    // twice: at index 0<=i<MAX_SAMPLES and again at MAX_SAMPLES <= i+MAX_SAMPLES  < (MAX_SAMPLES * 2)
    int totalSamples; // modulo max_samples gives index into sample array
    double sampleSec[MAX_SAMPLES * 2];
    double signal[MAX_SAMPLES * 2];
};

class QwtPlotCurve;
class QwtSymbol;
class QwtPlotMarker;
class QStandardItem;

// Struct to hold individual AP entry (cell) found
struct CellData {
    using Vector = std::vector<CellData>;
    std::string macAddr;
    std::string essid;
    std::string mode; // master, managed, etc.
    std::string security; // on or off
    std::string privacy; // group cipher
    std::string cipher; // pairwise cipher
    int channel;
    std::string frequency;
    std::string protocol; // b,g,n,ac
    int quality; // per microsoft definition, derived from signal
    int signal; // try to get dBm
    int load = -1; // bss load-channel util in % (-1 as not reported by BSS)
    int stationCount = -1; // bss station in % (-1 as not reported by BSS)
    int minSignal; // lowest seen
    int maxSignal; // highest seen
    int BW; // max BW in any protocol 20 or 40 or 80 or 160 MHz
    int cenChan; // center channel in 40/80/160 MHz bandwidths
    std::string vendor;
    bool firstPlot;
    long firstSeen;
    long lastSeen;
    std::string netType;
    QColor color;
    std::unique_ptr<QwtPlotCurve> pBandCurve;
    double xPlot[4];
    double yPlot[4];
    // see: http://www.qtcentre.org/threads/46316-Draw-a-single-point-into-a-qwt-plot
    std::unique_ptr<QwtPlotMarker> pCntlChanPlot; // to plot control channel marker
    std::unique_ptr<QwtPlotMarker> pSignalTimeMarker; // to plot marker on signal time graph
    std::unique_ptr<QwtPlotCurve> pTimeCurve;
    std::unique_ptr<QStandardItem> pTableItem[MAX_TABLE_COLS];
    std::unique_ptr<History> pHistory;
    int timesSeen; // believe it or not, some drivers report a MAC more than once per scan
};

#endif	/* _DATASTRUCT_H */