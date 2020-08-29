/* 
 * File:   Custom.h
 * Author: warren
 *
 * Created on November 4, 2012, 3:49 PM
 */

#ifndef CUSTOM_H
#define	CUSTOM_H

#define LINSSIDVERSION "3.6-ext"
#define LINSSIDPREFSVER "3.3.1"
#define LINSSIDDATALOGVER "2.8"
#define MAX_SAMPLES 100
#define TIME_PLOT_SCALE 100
#define MAX_TABLE_COLS 21 // as of version 3.6-ext
#define VENDOR_FILE_NAME "/usr/share/linssid/oui.datc"
#define PREFS_FILE_NAME ".linssid.prefs"
#define LOG_DATA_FILE_NAME "LinSSID.datalog"
#define FILTER_PREFS_FILE_NAME  ".linssid.filters"

enum wlDriver {
    IOCTL, NL80211
};

// The colTitle enum must be in the same order as both the headerTitle array
// and the tableHeader variable of the Qwt table
enum colTitle {
    PLOT, SSID, MAC, CHANNEL, MODE, SECURITY, PRIVACY,
    CIPHER, FREQUENCY, QUALITY, SIGNAL, LOAD, STATION_COUNT, BW, MINSIGNAL, MAXSIGNAL, CENCHAN,
    FIRST_SEEN, LAST_SEEN, VENDOR, PROTOCOL, TYPE // TYPE not yet impl
};

enum pageBlockType {
    BT_NONE, BT_BSS, BT_RSN, BT_BSS_LOAD, BT_HT_CAPABILITIES, BT_HT_OPERATION, BT_EXTENDED_CAPABILITIES,
    BT_VHT_CAPABILITIES, BT_VHT_OPERATION, BT_WPA, BT_WPS, BT_WMM
};

enum runStates {
    STOPPED, STOPPING, RUNNING
};

#endif	/* CUSTOM_H */
