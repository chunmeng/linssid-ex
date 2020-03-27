#include "PrefsHandler.h"
#include "Custom.h"
#include "Utils.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

using namespace std;

struct PrefsHandler::sSort {
    int column;
    int order;
};

struct PrefsHandler::sMaingeom {
    int x;
    int y;
    int width;
    int height;
};

struct PrefsHandler::sMainsplit {
    int topheight;
    int bottomheight;
};

struct PrefsHandler::sPlotprefs {
    int fntSize;
    int plotlb;
    int plotub;
    bool showgrid;
};

struct PrefsHandler::sDefPref {
    string version;
    int colwidth[MAX_TABLE_COLS];
    bool colvis[MAX_TABLE_COLS];
    int visorder[MAX_TABLE_COLS];
    sSort sort;
    sMaingeom maingeom;
    sMainsplit mainsplit;
    int plottab;
    int naptime;
    sPlotprefs plotprefs;
    int logData;
};

// @TODO: Should do something about these extern...
extern struct passwd *realUser;

namespace {
static PrefsHandler::sDefPref defPref = {// default prefs defined here
    /* version  */ LINSSIDPREFSVER,
    /* colwidth */
    {100, 100, 100, 100, 100, 100, 100, 100, 100,
            100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
    /* colvis   */
    {1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    /* visorder */
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18},
    /* sort     */
    { 18, 1},
    /* maingeom */
    {389, 83, 721, 542},
    ///* mainsplit*/
    {154, 223},
    /* plottab  */ 0,
    /* naptime  */ 2,
    /* plotprefs*/
    {11, -100, -20, 1}, // added font size
    /* logdata */ 0
};

}

PrefsHandler::PrefsHandler(const std::string& logPath)
    : logPath_(logPath)
{
}

PrefsHandler::~PrefsHandler() = default;

void PrefsHandler::writeDefault()
{
    // Writes a block of preferences of the structure sDefPref struct.
    // At entry, the file must either not exist or be closed.
    // At exit, the newly written file will be closed.
    std::cout << "Writing default pref to " << logPath_ << endl;
    ofstream prefs;
    prefs.open(logPath_, ios::out);
    Utils::waste(chown(logPath_.c_str(), realUser->pw_uid, realUser->pw_gid));
    chmod(logPath_.c_str(), 00644);
    std::cout << "Done chown pref " << logPath_;

    prefs << "version " << LINSSIDPREFSVER << endl;
    prefs << "colwidth";
    for (int i = 0; i < MAX_TABLE_COLS; i++)
        prefs << " " << defPref.colwidth[i];
    prefs << endl;
    prefs << "colvis";
    for (int i = 0; i < MAX_TABLE_COLS; i++)
        prefs << " " << defPref.colvis[i];
    prefs << endl;
    prefs << "visorder";
    for (int i = 0; i < MAX_TABLE_COLS; i++)
        prefs << " " << defPref.visorder[i];
    prefs << endl;
    prefs << "sort " << defPref.sort.column
            << " " << defPref.sort.order << endl;
    prefs << "maingeom " << defPref.maingeom.x
            << " " << defPref.maingeom.y
            << " " << defPref.maingeom.width
            << " " << defPref.maingeom.height << endl;
    prefs << "mainsplit " << defPref.mainsplit.topheight
            << " " << defPref.mainsplit.bottomheight << endl;
    prefs << "plottab " << defPref.plottab << endl;
    prefs << "naptime " << defPref.naptime << endl;
    prefs << "plotprefs " << defPref.plotprefs.fntSize
            << " " << defPref.plotprefs.plotlb
            << " " << defPref.plotprefs.plotub
            << " " << defPref.plotprefs.showgrid
            << endl;
    prefs << "logdata " << defPref.logData << endl;
    prefs.close();
}