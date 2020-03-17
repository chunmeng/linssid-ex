#ifndef _STATS_H
#define	_STATS_H

// Provide summary data of scan
struct Stats {
    int total5GBss = 0;
    int total2GBss = 0;
    int totalOpen = 0;  // Total BSS with open network

    void reset() {
        total5GBss = 0;
        total2GBss = 0;
        totalOpen = 0;
    }
};

#endif	/* _STATS_H */