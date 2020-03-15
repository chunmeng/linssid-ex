/* 
 * File:   CustomEvents.h
 * Author: warren
 *
 * Created on November 4, 2012, 3:49 PM
 */

#ifndef CUSTOM_H
#define	CUSTOM_H

#define LINSSIDVERSION "3.6"
#define LINSSIDPREFSVER "3.3"
#define LINSSIDDATALOGVER "2.8"
#define DATAWANTED QEvent::User + 1
#define DATAREADY QEvent::User + 2
#define NUMBER_OF_COLORS 13
#define MAX_SAMPLES 100
#define TIME_PLOT_SCALE 100
#define MAX_TABLE_COLS 19 // as of version 2.3
#define VENDOR_FILE_NAME "/usr/share/linssid/oui.datc"
#define PREFS_FILE_NAME ".linssid.prefs"
#define LOG_DATA_FILE_NAME "LinSSID.datalog"


enum wlDriver {
    IOCTL, NL80211
};

// The colTitle enum must be in the same order as both the headerTitle array
// and the tableHeader variable of the Qwt table
enum colTitle {
    PLOT, SSID, MAC, CHANNEL, MODE, SECURITY, PRIVACY,
    CIPHER, FREQUENCY, QUALITY, SIGNAL, BW, MINSIGNAL, MAXSIGNAL, CENCHAN,
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
