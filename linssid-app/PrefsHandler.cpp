#include "PrefsHandler.h"
#include "Logger.h"
#include "Utils.h"
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

using namespace std;

extern Logger AppLogger;

// @TODO: Should do something about these extern...
extern struct passwd *realUser;

namespace {
// 3.3.1 - Added plotprefs showLabel
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
    {11, -100, -20, 1, 1},
    /* logdata */ 0
};

}

PrefsHandler::PrefsHandler(const std::string& filePath)
    : filePath_(filePath)
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
    InfoLog(AppLogger) << "Writing preference to " << filePath_ << endl;
    ofstream prefs;
    prefs.open(filePath_, ios::out);
    Utils::waste(chown(filePath_.c_str(), realUser->pw_uid, realUser->pw_gid));
    chmod(filePath_.c_str(), 00644);

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
            << " " << prefData.plotprefs.showLabel
            << endl;
    prefs << "logdata " << prefData.logData << endl;
    prefs.close();
}

PrefsHandler::sDefPref PrefsHandler::load()
{
    sDefPref prefData;
    fstream prefs;
    prefs.open(filePath_, ios::in);
    if (!prefs.is_open()) { // no prefs file, so return default
        InfoLog(AppLogger) << "Load default preference" << endl;
        prefData = defPref;
        return prefData;
    }
    // make sure right version
    string line, tag, vers;
    istringstream lineParse;
    while (getline(prefs, line)) {
        lineParse.str(line);
        lineParse.clear();
        lineParse >> tag;
        if (tag == "version") {
            lineParse >> prefData.version;
            break;
        }
    }
    if (prefData.version != LINSSIDPREFSVER) { // old version so trash and replace with defaults
        prefs.close();
        ErrorLog(AppLogger) << "Could not parse ver: " << prefData.version << ". Load default preference" << endl;
        prefData = defPref;
        return prefData;
    }
    // Proceed to parse prefs
    InfoLog(AppLogger) << "Load preference from " << filePath_ << endl;
    prefs.seekg(0);
    prefs.clear();
    while (getline(prefs, line)) {
        lineParse.str(line);
        lineParse.clear();
        lineParse >> tag;
        if (tag == "colwidth") {
            for (int col = 0; col < MAX_TABLE_COLS; col++) {
                lineParse >> prefData.colwidth[col];
            }
        } else if (tag == "colvis") {
            for (int col = 0; col < MAX_TABLE_COLS; col++) {
                lineParse >> prefData.colvis[col];
            }
        } else if (tag == "sort") {
            lineParse >> prefData.sort.column >> prefData.sort.order;
        } else if (tag == "maingeom") {
            lineParse >> prefData.maingeom.x >> prefData.maingeom.y >> prefData.maingeom.width >> prefData.maingeom.height;
        } else if (tag == "visorder") {
            for (int i = 0; i < MAX_TABLE_COLS; i++)
                lineParse >> prefData.visorder[i];
        } else if (tag == "plottab") {
            lineParse >> prefData.plottab;
        } else if (tag == "mainsplit") {
            lineParse >> prefData.mainsplit.topheight >> prefData.mainsplit.bottomheight;
        } else if (tag == "naptime") {
            lineParse >> prefData.naptime;
        } else if (tag == "plotprefs") {
            int fntSize;
            int plotLb, plotUb;
            lineParse >> fntSize >> plotLb >> plotUb >> prefData.plotprefs.showgrid >> prefData.plotprefs.showLabel;
            // validate or the mess gets big
            if ((plotLb < -100) || (plotUb > 0) || (plotUb < (plotLb + 10))) { // prefs were hosed
                plotLb = -100;
                plotUb = -20; // so reset them to nominal values
            }
            prefData.plotprefs.plotlb = plotLb;
            prefData.plotprefs.plotub = plotUb;
            if ((fntSize < 10) || (fntSize > 16)) fntSize = 11; // reset to nominal
            prefData.plotprefs.fntSize = fntSize;
        } else if (tag == "logdata") {
            lineParse >> prefData.logData;
        }
    }
    prefs.close();
    return prefData;
}