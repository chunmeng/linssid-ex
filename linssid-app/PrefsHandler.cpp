#include "PrefsHandler.h"
#include "Utils.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

using namespace std;

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
    save(defPref);
}

void PrefsHandler::save(const sDefPref& prefData)
{
    // Writes a block of preferences of the structure sDefPref struct.
    // At entry, the file must either not exist or be closed.
    // At exit, the newly written file will be closed.
    std::cout << "Writing pref to " << logPath_ << endl;
    ofstream prefs;
    prefs.open(logPath_, ios::out);
    Utils::waste(chown(logPath_.c_str(), realUser->pw_uid, realUser->pw_gid));
    chmod(logPath_.c_str(), 00644);

    prefs << "version " << prefData.version << endl;
    prefs << "colwidth";
    for (int i = 0; i < MAX_TABLE_COLS; i++)
        prefs << " " << prefData.colwidth[i];
    prefs << endl;
    prefs << "colvis";
    for (int i = 0; i < MAX_TABLE_COLS; i++)
        prefs << " " << prefData.colvis[i];
    prefs << endl;
    prefs << "visorder";
    for (int i = 0; i < MAX_TABLE_COLS; i++)
        prefs << " " << prefData.visorder[i];
    prefs << endl;
    prefs << "sort " << prefData.sort.column
            << " " << prefData.sort.order << endl;
    prefs << "maingeom " << prefData.maingeom.x
            << " " << prefData.maingeom.y
            << " " << prefData.maingeom.width
            << " " << prefData.maingeom.height << endl;
    prefs << "mainsplit " << prefData.mainsplit.topheight
            << " " << prefData.mainsplit.bottomheight << endl;
    prefs << "plottab " << prefData.plottab << endl;
    prefs << "naptime " << prefData.naptime << endl;
    prefs << "plotprefs " << prefData.plotprefs.fntSize
            << " " << prefData.plotprefs.plotlb
            << " " << prefData.plotprefs.plotub
            << " " << prefData.plotprefs.showgrid
            << endl;
    prefs << "logdata " << prefData.logData << endl;
    prefs.close();
}