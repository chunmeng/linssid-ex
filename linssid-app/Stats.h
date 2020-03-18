#ifndef _STATS_H
#define	_STATS_H

// Provide summary data of scan
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

#endif	/* _STATS_H */