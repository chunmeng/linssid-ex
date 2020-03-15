/*
 * File:   mainForm.h
 * Author: warren
 *
 * Created on October 25, 2012, 11:43 AM
 */

#ifndef _MAINFORM_H
#define	_MAINFORM_H

#include <iostream>
#include <fstream>
#include <QEvent>
#include <QString>
#include <QFont>
#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_draw.h>
#include "ui_MainForm.h"
#include "prefsDialog.h"
#include "Custom.h"
using namespace std;

class Getter; // forward declare

class MainForm : public QMainWindow {
    Q_OBJECT
public:

    struct cellData;
    struct History;
    struct vendorStruct;
    struct sSort;
    struct sMaingeom;
    struct sMainsplit;
    struct sPlotprefs;
    struct sDefPref;

    MainForm();
    virtual ~MainForm();
    //    void run();
    void init();
    void loadVendorDb();
    string findVendor(string);
    void getUserID();
    void addInterfaces();
    void setInterface(int);
    int getNapTime();
    void writePrefsFile();
    void writePrefsBlock(sDefPref);
    void readPrefsFile();
    string getCurrentInterface();
    static const QEvent::Type DATA_READY_EVENT;
    class DataReadyEvent;
    int getReadyBlockNo();
    void postDataReadyEvent(const int);
    void drawTable();
    void setVisibleCols();
    void fillTable();
    class Chan24ScaleDraw;
    class Chan5ScaleDraw;
    void initColtoAction();
    void initPlotGrids();
    void drawChan24Plot();
    void drawChan5Plot();
    void drawTimePlot();
    void fillPlots();
    void initNewCell(string, int);
    void extractData(string, int &, int &);
    void doLogData();
    void writeLogDataHeader();
    int MaxIntStr(const string&);
    int MinIntStr(const string&);
    inline void waste(int);
    void resolveMesh(int);
    // void trimRight( string& );
    // void trimLeft( string& );

    static Getter* pGetter; // a pointer to the instance of the Getter that calls this MainForm
    static QThread* pGetterThread; // a pointer to the getter's thread
    static vector<cellData> cellDataRay;
    static vendorStruct* vendor;
    static fstream logDataStream;
    static int maxTableIndex;
    static int numVendors;
    static int maxVendorRecL;
    static long runStartTime;
    static long blockSampleTime;
    static long now;
    static int logDataState;
    static bool firstScan;
    static string wlPrivacies[3];
    static string wlCiphers[3];
    static string chan24Freq[15];
    static string chan50Freq[42][2];
    static QColor qColorArray[NUMBER_OF_COLORS];
    static QFont tblFnt;
    static QString fntSizes[];
    static int numFntSizes;
    static QAction* colToQAction[MAX_TABLE_COLS];
    static int columnWidth[MAX_TABLE_COLS];
    static QwtPlotGrid* chan24Grid;
    static QwtPlotGrid* chan5Grid;
    static QwtPlotGrid* timeGrid;
    static prefsDialog* prefsDlg1;
    static sDefPref defPref;

public slots:
    void doRun();
    void doPlotAll();
    void doPlotNone();
    void doTableChanged(int,int);
    void reDrawTable();
    void showAboutBox();
    void showPrefsDlg();
    void columnWidthSave(int, int, int);
    void updatePlotPrefs(QString, int, int, bool);
    void logPrefChanged(int);

protected:
    Ui::mainForm mainFormWidget;
    void customEvent(QEvent*); // This overrides QObject::customEvent()
    void closeEvent(QCloseEvent*); // Overides built-in closeEvent()
    void handleDataReadyEvent(const DataReadyEvent*);
};

#endif	/* _MAINFORM_H */
