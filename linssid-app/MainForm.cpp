/*
 * File:   mainForm.cpp
 * Author: warren
 *
 * Created on October 25, 2012, 11:43 AM
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <random>
#include <climits>
#include <memory>
#include <QtCore>
#include <QString>
#include <QThread>
#include <QEventLoop>
#include <QtWidgets>
#include <QDateTime>
#include <QHeaderView>
#include <QAbstractItemModel>
#include <QMainWindow>
#include <QMessageBox>
#include <QFont>
#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_textlabel.h>
#include <QPointF>
#include <qwt_scale_draw.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <boost/regex.hpp>
#include "Custom.h"
#include "MainForm.h"
#include "Getter.h"
#include "AboutBox.h"
#include "prefsDialog.h"
#include "ui_MainForm.h"
#include "Utils.h"
#include "DataLogger.h"
#include "PrefsHandler.h"
#include "VendorDb.h"
#include "DataProxyModel.h"
#include "ViewFilterDialog.h"

extern int lastBlockRequested;
extern int lastBlockReceived;
extern qint64 startTime;
extern string endBlockString;
extern string pipeName;
// extern ofstream linssidLog;
extern runStates runstate;
extern int realUID;
extern struct passwd *realUser;
extern string fullPrefsName;

extern string genPipeName(int);

using namespace std;

// define a few things

// declare some variables
Getter* MainForm::pGetter; // a pointer to our data getter
QThread* MainForm::pGetterThread; // a pointer to its thread
CellData::Vector MainForm::cellDataRay;
int MainForm::maxTableIndex; // holds the highest index pointer into cellData
long MainForm::runStartTime;
long MainForm::now; // absolute time of the block
pageBlockType pageBlock; // which section of page is data coming from
int MainForm::logDataState;
long MainForm::blockSampleTime; // time of the block relative to runStartTime
bool MainForm::firstScan; // do we need to get sudo privileges?

// Local unnamed namespace
namespace {

QColor qColorArray[]
{
    Qt::red, Qt::green, Qt::blue, Qt::darkRed, Qt::darkGreen, Qt::darkBlue,
    Qt::cyan, Qt::magenta, Qt::gray, Qt::darkCyan, Qt::darkMagenta,
    Qt::darkYellow, Qt::darkGray,
    QColor(255, 0, 0, 127) /* semi-transparent red */,
    QColor(0, 255, 0, 127) /* semi-transparent green */,
    QColor(0, 0, 255, 127) /* semi-transparent blue */,
    QColor(204, 102, 0, 255) /* Orange-range */,
    QColor(255, 128, 0, 255) /* Orange-range */,
    QColor(255, 153, 51, 255) /* Orange-range */,
    QColor(255, 178, 102, 255) /* Orange-range */,
    QColor(255, 155, 153, 255) /* Orange-range */,
    QColor(102, 0, 204, 255) /* Purple-range */,
    QColor(127, 0, 255, 255) /* Purple-range */,
    QColor(153, 57, 255, 255) /* Purple-range */,
    QColor(178, 102, 255, 255) /* Purple-range */,
    QColor(204, 0, 204, 255),
    QColor(255, 0, 255, 255),
    QColor(255, 51, 255, 255),
    QColor(255, 102, 255, 255),
    QColor(0, 102, 204, 255),
    QColor(0, 128, 255, 255),
    QColor(51, 153, 255, 255),
    QColor(102, 178, 255, 255),
};

constexpr auto NUMBER_OF_COLORS = sizeof(qColorArray)/sizeof(QColor);

};

QFont MainForm::tblFnt( (const QString) "Arial", 11);
QString MainForm::fntSizes[] = {"10", "11", "12", "14", "16"};
int MainForm::numFntSizes = 5;
QAction* MainForm::colToQAction[MAX_TABLE_COLS];
int MainForm::columnWidth[MAX_TABLE_COLS]; // since Qt doesn't see fit to remember for us...

MainForm::MainForm() {

    MainForm::mainFormWidget.setupUi(this);
    connect(MainForm::mainFormWidget.runBtn, SIGNAL(clicked()), this, SLOT(doRun()));
    connect(MainForm::mainFormWidget.allBtn, SIGNAL(clicked()), this, SLOT(doPlotAll()));
    connect(MainForm::mainFormWidget.noneBtn, SIGNAL(clicked()), this, SLOT(doPlotNone()));
    connect(MainForm::mainFormWidget.actionSSID, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionMAC, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionChannel, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionMode, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionProtocol, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionSecurity, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionPrivacy, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionCipher, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionFrequency, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionQuality, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionSignal, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionLoad, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionStationCount, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionBW, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionMin_Signal, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionMax_Signal, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionCenChan, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionFirst_Seen, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionLast_Seen, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionVendor, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionType, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(MainForm::mainFormWidget.actionAbout, SIGNAL(triggered()), this, SLOT(showAboutBox()));
    connect(MainForm::mainFormWidget.actionPrefs, SIGNAL(triggered()), this, SLOT(showPrefsDlg()));
    connect(MainForm::mainFormWidget.actionViewFilter, SIGNAL(triggered()), this, SLOT(showViewFilterDlg()));

    model_ = make_unique<QStandardItemModel>();
    model_->setColumnCount(MAX_TABLE_COLS);
    // Make sure the column labels below are same order as the enum <colTitle>
    model_->setHorizontalHeaderLabels(
            QString("Plot|SSID|MAC|Channel|Mode|Security|Privacy|Cipher|Frequency\
|Quality|Signal|Load|Station Count|BW MHz|Min Sig|Max Sig|Cen Chan|First Seen|Last Seen|Vendor|Protocol|Type").split("|"));
    proxyModel_ = make_unique<DataProxyModel>();
    proxyModel_->setSourceModel(model_.get());
    MainForm::mainFormWidget.mainTableView->setModel(proxyModel_.get());

    // https://stackoverflow.com/questions/19442050/qtableview-how-can-i-get-the-data-when-user-click-on-a-particular-cell-using-mo
    connect(MainForm::mainFormWidget.mainTableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(doTableClicked(const QModelIndex &)));
    connect(MainForm::mainFormWidget.mainTableView->horizontalHeader(),
            SIGNAL(sectionResized(int, int, int)), this, SLOT(columnWidthSave(int, int, int)));

    MainForm::cellDataRay.reserve(50); // expect about 50 cells to be found. More or less is OK.
    lastBlockRequested = 0; // these global variables should be made protected
    lastBlockReceived = 0;
    startTime = QDateTime::currentMSecsSinceEpoch();
    MainForm::firstScan = true;
}

MainForm::~MainForm() {
    MainForm::pGetter->quit();
}

void MainForm::init() {
    srand(time(NULL)); //initialize the random number generator seed
    MainForm::addInterfaces(); // find the wifi interface names and add them to the comboBox
    MainForm::setInterface(0); // set the interface select arbitrarily to 0
    MainForm::maxTableIndex = -1;
    MainForm::drawTable();
    MainForm::initPlotGrids(); // must do before reading prefs, since prefs will modify
    MainForm::initColtoAction(); // init pointers to view menu items
    MainForm::initStatusBar();
    MainForm::loadPrefs();
    MainForm::drawTable(); // do it again after application of prefs
    MainForm::drawChan24Plot();
    MainForm::drawChan5Plot();
    MainForm::drawTimePlot();
    extern string fullLogName;
    MainForm::dataLogger = make_unique<DataLogger>(fullLogName);
    MainForm::vendorDb = make_unique<VendorDb>();
}
// saving as comments in case ever need it again...
// reimplemented from QApplication so we can throw exceptions in slots

//virtual bool notify(QObject * receiver, QEvent * event) {
//    try {
//        return QApplication::notify(receiver, event);
//    } catch (std::exception& e) {
//        qCritical() << "Exception thrown:" << e.what();
//        linssidLog << "Exception caught by notify: " << e.what() << endl;
//    }
//    return false;
//}

void MainForm::initColtoAction() {
    // brutal -- set the array of pointers from columns to their menu items
    // these should have the same elements as the enum in 'custom.h'
    // PLOT, SSID, MAC, CHANNEL, MODE, SECURITY, PRIVACY,
    // CIPHER, FREQUENCY, QUALITY, SIGNAL, BW, MINSIGNAL, MAXSIGNAL, CENCHAN,
    // FIRST_SEEN, LAST_SEEN, VENDOR, PROTOCOL
    MainForm::colToQAction[PLOT] = MainForm::mainFormWidget.actionPlot;
    MainForm::colToQAction[SSID] = MainForm::mainFormWidget.actionSSID;
    MainForm::colToQAction[MAC] = MainForm::mainFormWidget.actionMAC;
    MainForm::colToQAction[CHANNEL] = MainForm::mainFormWidget.actionChannel;
    MainForm::colToQAction[MODE] = MainForm::mainFormWidget.actionMode;
    MainForm::colToQAction[PROTOCOL] = MainForm::mainFormWidget.actionProtocol;
    MainForm::colToQAction[SECURITY] = MainForm::mainFormWidget.actionSecurity;
    MainForm::colToQAction[PRIVACY] = MainForm::mainFormWidget.actionPrivacy;
    MainForm::colToQAction[CIPHER] = MainForm::mainFormWidget.actionCipher;
    MainForm::colToQAction[FREQUENCY] = MainForm::mainFormWidget.actionFrequency;
    MainForm::colToQAction[QUALITY] = MainForm::mainFormWidget.actionQuality;
    MainForm::colToQAction[SIGNAL] = MainForm::mainFormWidget.actionSignal;
    MainForm::colToQAction[LOAD] = MainForm::mainFormWidget.actionLoad;
    MainForm::colToQAction[STATION_COUNT] = MainForm::mainFormWidget.actionStationCount;
    MainForm::colToQAction[BW] = MainForm::mainFormWidget.actionBW;
    MainForm::colToQAction[MINSIGNAL] = MainForm::mainFormWidget.actionMin_Signal;
    MainForm::colToQAction[MAXSIGNAL] = MainForm::mainFormWidget.actionMax_Signal;
    MainForm::colToQAction[CENCHAN] = MainForm::mainFormWidget.actionCenChan;
    MainForm::colToQAction[FIRST_SEEN] = MainForm::mainFormWidget.actionFirst_Seen;
    MainForm::colToQAction[LAST_SEEN] = MainForm::mainFormWidget.actionLast_Seen;
    MainForm::colToQAction[VENDOR] = MainForm::mainFormWidget.actionVendor;
}

void MainForm::initPlotGrids() {
    // add some grids to our plots
    MainForm::chan24Grid = make_unique<QwtPlotGrid>();
    MainForm::chan24Grid->enableX(false);
    MainForm::chan24Grid->attach(MainForm::mainFormWidget.chan24Plot);
    MainForm::chan5Grid = make_unique<QwtPlotGrid>();
    MainForm::chan5Grid->enableX(false);
    MainForm::chan5Grid->attach(MainForm::mainFormWidget.chan5Plot);
    MainForm::timeGrid = make_unique<QwtPlotGrid>();
    MainForm::timeGrid->enableX(false);
    MainForm::timeGrid->attach(MainForm::mainFormWidget.timePlot);
}

void MainForm::initStatusBar() {
    MainForm::statusCounts = make_unique<QLabel>();
    MainForm::statusCounts->setText(QString::fromStdString((MainForm::stats.toString())));
    MainForm::mainFormWidget.statusbar->addPermanentWidget(MainForm::statusCounts.get());
}

void MainForm::fillStatus() {
    MainForm::statusCounts->setText(QString::fromStdString((MainForm::stats.toString())));
}

void MainForm::addInterfaces() {
    string somePipeName = genPipeName(10);
    string eof = "###EOF###";
    mkfifo(somePipeName.c_str(), 0666);
    static fstream ifs(somePipeName);
    string commandLine = "iw dev >> " + somePipeName;
    if (system(commandLine.c_str()) == 0) {
        commandLine = "echo \'" + eof + "\' >> " + somePipeName;
        Utils::waste(system(commandLine.c_str()));
        string interfaceLine;
        mainFormWidget.interfaceCbx->clear();
        boost::smatch sm;
        boost::regex rx("^[ \\t]+Interface +([^ ]+) *");
        while (getline(ifs, interfaceLine) && interfaceLine != eof) {
            if (boost::regex_match(interfaceLine, sm, rx))
                mainFormWidget.interfaceCbx->addItem(QString::fromStdString(sm[1]));
        }
    } else {
        QMessageBox::critical(0, "Bad Stuff", QString("Unable to continue.\nCannot find interface pipe"));
        ifs.close();
        remove(somePipeName.c_str());
        exit(1);
    }
    //
    // Now repeat the whole mess because linux wifi drivers can't decide where to announce themselves
    //
    commandLine = "cat /proc/net/wireless >> " + somePipeName;
    if (system(commandLine.c_str()) == 0) {
        commandLine = "echo \'" + eof + "\' >> " + somePipeName;
        Utils::waste(system(commandLine.c_str()));
        string interfaceLine;
        QString interface;
        boost::smatch sm2;
        boost::regex rx2("^ *([0-9A-Za-z_-]+) *:.+");
        while (getline(ifs, interfaceLine) && interfaceLine != eof) {
            if (boost::regex_match(interfaceLine, sm2, rx2)) {
                interface = QString::fromStdString(sm2[1]);
                bool found = false;
                for (int i = 0; i < mainFormWidget.interfaceCbx->count(); i++) {
                    if (interface == mainFormWidget.interfaceCbx->itemText(i)) {
                        found = true;
                        break;
                    }
                }
                if (!found) mainFormWidget.interfaceCbx->addItem(interface);
            }
        }
        if (mainFormWidget.interfaceCbx->count() <= 0) {
            QMessageBox::critical(0, "Bad Stuff",
                    QString("Unable to continue.\nNo wireless interfaces found"));
            exit(1);
        }
    } else {
        QMessageBox::critical(0, "Bad Stuff", QString("Unable to continue.\nCannot find interface pipe"));
        ifs.close();
        remove(somePipeName.c_str());
        exit(1);
    }
    ifs.close();
    remove(somePipeName.c_str());
}

void MainForm::setInterface(int ifIndx) {
    MainForm::mainFormWidget.interfaceCbx->setCurrentIndex(ifIndx);
}

string MainForm::getCurrentInterface() {
    return ((mainFormWidget.interfaceCbx->currentText()).toStdString());
}

// Define the custom event identifier
const QEvent::Type MainForm::DATA_READY_EVENT = static_cast<QEvent::Type> (DATAREADY);
// Define the custom event subclass

class MainForm::DataReadyEvent : public QEvent {
public:

    DataReadyEvent(const int customData1) :
    QEvent(MainForm::DATA_READY_EVENT), readyBlockNo(customData1) {
    }

    int getReadyBlockNo() const {
        return readyBlockNo;
    }
private:
    int readyBlockNo;
};

int MainForm::getNapTime() {
    return MainForm::mainFormWidget.napTimeSlider->value();
}

void MainForm::applyPlotPrefs(int fntSize, int plotMin, int plotMax, bool showGrid) {
    MainForm::tblFnt.setPointSize(fntSize);
    MainForm::mainFormWidget.timePlot->setAxisScale(QwtPlot::yLeft, plotMin, plotMax, 20);
    MainForm::mainFormWidget.chan24Plot->setAxisScale(QwtPlot::yLeft, plotMin, plotMax, 20);
    MainForm::mainFormWidget.chan5Plot->setAxisScale(QwtPlot::yLeft, plotMin, plotMax, 20);
    MainForm::timeGrid->enableY(showGrid);
    MainForm::chan24Grid->enableY(showGrid);
    MainForm::chan5Grid->enableY(showGrid);
}

void MainForm::updatePlotPrefs(QString tblFntSize, int plotMin, int plotMax, bool showGrid, bool showLabel) {
    // a slot called from the prefs dialog to dynamically update the plots
    applyPlotPrefs(tblFntSize.toInt(), plotMin, plotMax, showGrid);
    this->plotShowLabel = showLabel;
    MainForm::reDrawTable();
    MainForm::mainFormWidget.timePlot->replot();
    MainForm::mainFormWidget.chan24Plot->replot();
    MainForm::mainFormWidget.chan5Plot->replot();
}

void MainForm::logPrefChanged(int state) {
    MainForm::logDataState = state;
}

void MainForm::savePrefs() {
    PrefsHandler::sDefPref appPref;
    appPref.version = LINSSIDPREFSVER;
    // col number must match the enum in "custom.h"
    for (int col = 0; col < MAX_TABLE_COLS; col++) {
        int colWidth = mainFormWidget.mainTableView->columnWidth(col);
        appPref.colwidth[col] = (colWidth == 0) ? columnWidth[col] : colWidth;
    }
    for (int col = 0; col < MAX_TABLE_COLS; col++)
        appPref.colvis[col] = !mainFormWidget.mainTableView->isColumnHidden(col);
    for (int visCol = 0; visCol < MAX_TABLE_COLS; visCol++)
        appPref.visorder[visCol] = mainFormWidget.mainTableView->horizontalHeader()->logicalIndex(visCol);
    appPref.sort = {.column = MainForm::mainFormWidget.mainTableView->horizontalHeader()->sortIndicatorSection(),
                    .order = MainForm::mainFormWidget.mainTableView->horizontalHeader()->sortIndicatorOrder()};
    appPref.maingeom = {.x = this->x(), .y = this->y(),
                        .width = this->width(), .height = this->height()};
    appPref.mainsplit = {.topheight = MainForm::mainFormWidget.splitter->sizes().value(0),
                         .bottomheight = MainForm::mainFormWidget.splitter->sizes().value(1)};
    appPref.plottab = MainForm::mainFormWidget.mainTabWgt->currentIndex();
    appPref.naptime = MainForm::mainFormWidget.napTimeSlider->value();
    appPref.plotprefs = {.fntSize = MainForm::tblFnt.pointSize(),
                         .plotlb = static_cast<int>(MainForm::mainFormWidget.timePlot->axisScaleDiv(QwtPlot::yLeft).lowerBound()),
                         .plotub = static_cast<int>(MainForm::mainFormWidget.timePlot->axisScaleDiv(QwtPlot::yLeft).upperBound()),
                         .showgrid = MainForm::timeGrid->yEnabled(),
                         .showLabel = this->plotShowLabel};
    appPref.logData = MainForm::logDataState;
    if (prefsHandler) prefsHandler->save(appPref);
}

void MainForm::loadPrefs() {
    if (!prefsHandler) prefsHandler = make_unique<PrefsHandler>(fullPrefsName);
    PrefsHandler::sDefPref appPref = prefsHandler->load();

    for (int col = 0; col < MAX_TABLE_COLS; col++) {
        if (appPref.colwidth[col] > 0) {
            MainForm::mainFormWidget.mainTableView->setColumnWidth(col, appPref.colwidth[col]);
            columnWidth[col] = appPref.colwidth[col];
        }
    }

    for (int col = 0; col < MAX_TABLE_COLS; col++) {
        bool vis = appPref.colvis[col];
        MainForm::colToQAction[col]->setChecked(vis);
        MainForm::mainFormWidget.mainTableView->setColumnHidden(col, !vis);
    }

    if (appPref.sort.column >= 0 && appPref.sort.column < MAX_TABLE_COLS) {
        MainForm::mainFormWidget.mainTableView->horizontalHeader()->setSortIndicator(appPref.sort.column, Qt::SortOrder(appPref.sort.order));
    }

    this->setGeometry(appPref.maingeom.x, appPref.maingeom.y, appPref.maingeom.width, appPref.maingeom.height);

    for (int i = 0; i < MAX_TABLE_COLS - 1; i++) { // loop through visual order
        if (appPref.visorder[i] != mainFormWidget.mainTableView->horizontalHeader()->logicalIndex(i)) {
            // find the index of the visual column that has the desired logical column
            for (int j = i + 1; j < MAX_TABLE_COLS; j++) { // and swap
                if (appPref.visorder[i] == mainFormWidget.mainTableView->horizontalHeader()->logicalIndex(j)) {
                    mainFormWidget.mainTableView->horizontalHeader()->swapSections(i, j);
                    break;
                }
            }
        }
    }

    MainForm::mainFormWidget.mainTabWgt->setCurrentIndex(appPref.plottab);
    MainForm::mainFormWidget.splitter->setSizes({appPref.mainsplit.topheight, appPref.mainsplit.bottomheight});
    MainForm::mainFormWidget.napTimeSlider->setValue(appPref.naptime);

    // Sanity is ensured by PrefsHandler
    int fntSize = appPref.plotprefs.fntSize;
    int plotLb = appPref.plotprefs.plotlb;
    int plotUb = appPref.plotprefs.plotub;
    bool showGrid = appPref.plotprefs.showgrid;
    applyPlotPrefs(fntSize, plotLb, plotUb, showGrid);
    MainForm::mainFormWidget.mainTableView->setFont(tblFnt);
    this->plotShowLabel = appPref.plotprefs.showLabel;
    MainForm::logDataState = appPref.logData;
}

void MainForm::postDataReadyEvent(const int customData1) {
    // This method (postDataReadyEvent) can be called from any thread
    QApplication::postEvent(this, new DataReadyEvent(customData1));
}

void MainForm::doRun() {
    extern runStates runState;
    if (MainForm::mainFormWidget.runBtn->isChecked()) {
        if (firstScan) {
            firstScan = false;
            MainForm::runStartTime = time(NULL);
        }
        MainForm::mainFormWidget.statusTxt->setText("Scanning ...");
        runState = RUNNING;
        MainForm::pGetter->Getter::postDataWantedEvent(++lastBlockRequested);
        MainForm::drawTable();
        MainForm::drawChan24Plot();
        MainForm::drawChan5Plot();
        MainForm::drawTimePlot();
    } else {
        if (runState != STOPPED) runState = STOPPING;
        MainForm::mainFormWidget.statusTxt->setText("Waiting for wifi to terminate scan ...");
        while (runState != STOPPED) usleep(500 * 1000); // wait half a second and try again
        MainForm::mainFormWidget.statusTxt->setText("Paused ...");
    }
}

void MainForm::doPlotAll() {
    for (int tbi = 0; tbi <= maxTableIndex; tbi++) {
        MainForm::cellDataRay[tbi].pTableItem[PLOT]->setCheckState(Qt::Checked);
    }
    fillPlots();
}

void MainForm::doPlotNone() {
    for (int tbi = 0; tbi <= maxTableIndex; tbi++) {
        MainForm::cellDataRay[tbi].pTableItem[PLOT]->setCheckState(Qt::Unchecked);
    }
    fillPlots();
}

void MainForm::doTableClicked(const QModelIndex &index) {
    // std::cout << "User clicked item (" << index.row() << ", " << index.column() << ")" << endl;
    if (index.column() == PLOT) {
        fillPlots();
    }
}

void MainForm::reDrawTable() {

    MainForm::drawTable();
    MainForm::fillTable();
}

void MainForm::showAboutBox() {
    AboutBox aboutBox1;
    aboutBox1.exec();
}

void MainForm::showPrefsDlg() {
    if (prefsDlg != nullptr) return; // already a prefs dialog open somewhere...
    prefsDlg = make_unique<prefsDialog>(
            QString::number(MainForm::tblFnt.pointSize()),
            int(MainForm::mainFormWidget.timePlot->axisScaleDiv(QwtPlot::yLeft).lowerBound()),
            int(MainForm::mainFormWidget.timePlot->axisScaleDiv(QwtPlot::yLeft).upperBound()),
            MainForm::timeGrid->yEnabled(),
            this->plotShowLabel,
            MainForm::logDataState,
            (QObject*)this);
    prefsDlg->exec();
    prefsDlg.reset();
}

void MainForm::showViewFilterDlg() {
    // Make a modeless dialog
    if (viewFilterDlg_ == nullptr) {// already a prefs dialog open somewhere...
        viewFilterDlg_ = make_unique<ViewFilterDialog>((QObject*)this->proxyModel_.get());
    }

    viewFilterDlg_->show();
    viewFilterDlg_->raise();
    viewFilterDlg_->activateWindow();
}

void MainForm::columnWidthSave(int col, int oldWidth, int newWidth) {
    if (newWidth > 0) MainForm::columnWidth[col] = newWidth;
    else if (oldWidth > 0) MainForm::columnWidth[col] = oldWidth;
}

void MainForm::customEvent(QEvent * event) {
    // When we get here, we've crossed the thread boundary and are now
    // executing in the Qt object's thread
    if (event->type() == MainForm::DATA_READY_EVENT) {
        handleDataReadyEvent(static_cast<DataReadyEvent *> (event));
    }
    // use more else ifs to handle other custom events
}

void MainForm::closeEvent(QCloseEvent * event) {
    extern runStates runState;
    if (runState == RUNNING) runState = STOPPING; // tell getter to stop what it's doing
    MainForm::mainFormWidget.statusTxt->setText("Exiting, waiting for wifi driver ...");
    MainForm::mainFormWidget.statusTxt->repaint();
    while (runState != STOPPED) usleep(500 * 1000); // wait until getter is stopped
    pGetterThread->QThread::quit();
    MainForm::cellDataRay.clear();
    savePrefs();
    pGetterThread->QThread::wait();
    MainForm::mainFormWidget.statusTxt->setText("Closing ...");
    MainForm::mainFormWidget.statusTxt->repaint();
    remove(pipeName.c_str());
//    linssidLog.close();
    dataLogger.reset();
    //    QMainWindow::closeEvent(event);
    event->accept();
    std::exit(0); // that's the system exit, not the Qt version
}

void MainForm::drawTable() {
    model_->setRowCount(MainForm::maxTableIndex + 1);
    setVisibleCols();
    MainForm::mainFormWidget.mainTableView->horizontalHeader()->setSectionsMovable(true);
    MainForm::mainFormWidget.mainTableView->horizontalHeader()
            ->setToolTip("Click to sort\nDrag and Drop to re-order\nClick and drag divider to fit");
    MainForm::mainFormWidget.mainTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainForm::setVisibleCols() {
    // the Plot column is always visible
    MainForm::mainFormWidget.mainTableView->setColumnHidden(SSID,
            !(MainForm::mainFormWidget.actionSSID->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(MAC,
            !(MainForm::mainFormWidget.actionMAC->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(CHANNEL,
            !(MainForm::mainFormWidget.actionChannel->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(MODE,
            !(MainForm::mainFormWidget.actionMode->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(PROTOCOL,
            !(MainForm::mainFormWidget.actionProtocol->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(SECURITY,
            !(MainForm::mainFormWidget.actionSecurity->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(PRIVACY,
            !(MainForm::mainFormWidget.actionPrivacy->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(CIPHER,
            !(MainForm::mainFormWidget.actionCipher->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(FREQUENCY,
            !(MainForm::mainFormWidget.actionFrequency->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(QUALITY,
            !(MainForm::mainFormWidget.actionQuality->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(SIGNAL,
            !(MainForm::mainFormWidget.actionSignal->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(LOAD,
            !(MainForm::mainFormWidget.actionLoad->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(STATION_COUNT,
            !(MainForm::mainFormWidget.actionStationCount->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(BW,
            !(MainForm::mainFormWidget.actionBW->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(MINSIGNAL,
            !(MainForm::mainFormWidget.actionMin_Signal->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(MAXSIGNAL,
            !(MainForm::mainFormWidget.actionMax_Signal->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(CENCHAN,
            !(MainForm::mainFormWidget.actionCenChan->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(FIRST_SEEN,
            !(MainForm::mainFormWidget.actionFirst_Seen->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(LAST_SEEN,
            !(MainForm::mainFormWidget.actionLast_Seen->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(VENDOR,
            !(MainForm::mainFormWidget.actionVendor->isChecked()));
    MainForm::mainFormWidget.mainTableView->setColumnHidden(TYPE, true); // Not implemented. Always hide
}

void MainForm::fillTable() {
    MainForm::stats.reset();
    MainForm::mainFormWidget.mainTableView->setFont(tblFnt);
    
    // fill in the x-y, also set each cell text alignment
    model_->setRowCount(maxTableIndex + 1);
    for (int row = 0; row <= maxTableIndex; row++) {
        MainForm::cellDataRay[row].pTableItem[SSID]->
                setText(MainForm::cellDataRay[row].essid.c_str());
        if (MainForm::cellDataRay[row].essid == "<hidden>") MainForm::stats.totalHidden++;
        MainForm::cellDataRay[row].pTableItem[MAC]->
                setText(MainForm::cellDataRay[row].macAddr.c_str());
        MainForm::cellDataRay[row].pTableItem[MAC]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[CHANNEL]->
                setData(MainForm::cellDataRay[row].channel,Qt::DisplayRole);
        if (MainForm::cellDataRay[row].channel <= 14) MainForm::stats.total2GBss++;
        else MainForm::stats.total5GBss++;
        MainForm::cellDataRay[row].pTableItem[CHANNEL]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[MODE]->
                setText(MainForm::cellDataRay[row].mode.c_str());
        MainForm::cellDataRay[row].pTableItem[MODE]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[PROTOCOL]->
                setText(MainForm::cellDataRay[row].protocol.c_str());
        MainForm::cellDataRay[row].pTableItem[PROTOCOL]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[SECURITY]->
                setText(MainForm::cellDataRay[row].security.c_str());
        MainForm::cellDataRay[row].pTableItem[SECURITY]->setTextAlignment(Qt::AlignCenter);
        if (MainForm::cellDataRay[row].security.empty()) MainForm::stats.totalOpen++;
        MainForm::cellDataRay[row].pTableItem[PRIVACY]->
                setText((MainForm::cellDataRay[row].privacy).c_str());
        MainForm::cellDataRay[row].pTableItem[PRIVACY]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[CIPHER]->
                setText((MainForm::cellDataRay[row].cipher).c_str());
        MainForm::cellDataRay[row].pTableItem[CIPHER]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[FREQUENCY]->
                setText(MainForm::cellDataRay[row].frequency.c_str());
        MainForm::cellDataRay[row].pTableItem[FREQUENCY]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[QUALITY]->
                setData(MainForm::cellDataRay[row].quality, Qt::DisplayRole);
        MainForm::cellDataRay[row].pTableItem[QUALITY]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[SIGNAL]->
                setData(MainForm::cellDataRay[row].signal, Qt::DisplayRole);
        MainForm::cellDataRay[row].pTableItem[SIGNAL]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[LOAD]->
                setData((MainForm::cellDataRay[row].load < 0) ? QVariant("-") : QVariant(MainForm::cellDataRay[row].load),
                        Qt::DisplayRole);
        MainForm::cellDataRay[row].pTableItem[LOAD]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[STATION_COUNT]->
                setData((MainForm::cellDataRay[row].stationCount < 0) ? QVariant("-") : QVariant(MainForm::cellDataRay[row].stationCount),
                        Qt::DisplayRole);
        MainForm::cellDataRay[row].pTableItem[STATION_COUNT]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[BW]->
                setData(MainForm::cellDataRay[row].BW, Qt::DisplayRole);
        MainForm::cellDataRay[row].pTableItem[BW]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[MINSIGNAL]->
                setData(MainForm::cellDataRay[row].minSignal, Qt::DisplayRole);
        MainForm::cellDataRay[row].pTableItem[MINSIGNAL]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[MAXSIGNAL]->
                setData(MainForm::cellDataRay[row].maxSignal, Qt::DisplayRole);
        MainForm::cellDataRay[row].pTableItem[MAXSIGNAL]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[CENCHAN]->
                setData(MainForm::cellDataRay[row].cenChan, Qt::DisplayRole);
        MainForm::cellDataRay[row].pTableItem[CENCHAN]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[FIRST_SEEN]->
                setText(QDateTime::fromTime_t(MainForm::cellDataRay[row].firstSeen).toString("MM/dd-hh:mm:ss"));
        MainForm::cellDataRay[row].pTableItem[FIRST_SEEN]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[LAST_SEEN]->
                setText(QDateTime::fromTime_t(MainForm::cellDataRay[row].lastSeen).toString("MM/dd-hh:mm:ss"));
        MainForm::cellDataRay[row].pTableItem[LAST_SEEN]->setTextAlignment(Qt::AlignCenter);
        MainForm::cellDataRay[row].pTableItem[VENDOR]->
                setText(MainForm::cellDataRay[row].vendor.c_str());
    }
    setVisibleCols();
    MainForm::mainFormWidget.mainTableView->setSortingEnabled(true);
}

class MainForm::Chan24ScaleDraw : public QwtScaleDraw {
public:

    Chan24ScaleDraw() {
    }

    virtual QwtText label(double v) const {
        if (v >= 1 && v <= 14) return (QString::number(int(v)));

        else return (QString(""));
    }
};

class MainForm::Chan5ScaleDraw : public QwtScaleDraw {
public:

    Chan5ScaleDraw() {
    }

    virtual QwtText label(double v) const {
        if ((v >= 1.0) && (v <= 160.0) && (int(v) % 10 == 0)) return (QString::number(int(v)));

        else return (QString(""));
    }
};

void MainForm::drawChan24Plot() {

    MainForm::mainFormWidget.chan24Plot->setAxisScale(QwtPlot::xBottom, -1, 16, 1);
    MainForm::mainFormWidget.chan24Plot->setAxisMaxMinor(QwtPlot::xBottom, 0);
    MainForm::mainFormWidget.chan24Plot->setAxisScaleDraw(QwtPlot::xBottom, new Chan24ScaleDraw());
    MainForm::mainFormWidget.chan24Plot->replot();
}

void MainForm::drawChan5Plot() {

    MainForm::mainFormWidget.chan5Plot->setAxisScale(QwtPlot::xBottom, 0, 170, 10);
    MainForm::mainFormWidget.chan5Plot->setAxisMaxMinor(QwtPlot::xBottom, 5);
    MainForm::mainFormWidget.chan5Plot->setAxisScaleDraw(QwtPlot::xBottom, new Chan5ScaleDraw());
    MainForm::mainFormWidget.chan5Plot->replot();
}

void MainForm::drawTimePlot() {

    MainForm::mainFormWidget.timePlot->setAxisScale(QwtPlot::xBottom,
            MainForm::blockSampleTime - TIME_PLOT_SCALE,
            MainForm::blockSampleTime, 10);
    MainForm::mainFormWidget.timePlot->setAxisMaxMinor(QwtPlot::xBottom, 5);
    MainForm::mainFormWidget.timePlot->replot();
}

void MainForm::fillPlots() {
    // rescale the time plot
    MainForm::mainFormWidget.timePlot->setAxisScale(QwtPlot::xBottom,
            MainForm::blockSampleTime - TIME_PLOT_SCALE,
            MainForm::blockSampleTime, 10);
    for (int tbi = 0; tbi <= maxTableIndex; tbi++) {
        // first attach plots plots we couldn't before because of sparse data
        if (shouldBePlot(tbi)) {
            if (MainForm::cellDataRay[tbi].BW >= 40)
                MainForm::cellDataRay[tbi].pChanSymbol->setStyle(QwtSymbol::Diamond);
            else MainForm::cellDataRay[tbi].pChanSymbol->setStyle(QwtSymbol::Triangle);

            MainForm::cellDataRay[tbi].pChanSymbol->setColor(MainForm::cellDataRay[tbi].color);
            MainForm::cellDataRay[tbi].pChanSymbol->setSize(10, 10);

            if (this->plotShowLabel) {
                QwtText markerLabel = QString::fromStdString(MainForm::cellDataRay[tbi].essid);
                markerLabel.setColor(MainForm::cellDataRay[tbi].color);
                int ub = static_cast<int>(MainForm::mainFormWidget.timePlot->axisScaleDiv(QwtPlot::yLeft).upperBound());
                if (MainForm::cellDataRay[tbi].signal <= ub - 5)
                    MainForm::cellDataRay[tbi].pCntlChanPlot->setLabelAlignment(Qt::AlignCenter | Qt::AlignTop);
                else MainForm::cellDataRay[tbi].pCntlChanPlot->setLabelAlignment(Qt::AlignCenter | Qt::AlignBottom);
                MainForm::cellDataRay[tbi].pCntlChanPlot->setLabel(markerLabel);
            } else {
                MainForm::cellDataRay[tbi].pCntlChanPlot->setLabel(QwtText(""));
            }

            if (MainForm::cellDataRay[tbi].firstPlot) {
                resolveMesh(tbi);
                if (MainForm::cellDataRay[tbi].frequency.substr(0, 1) == "2") {
                    MainForm::cellDataRay[tbi].pBandCurve->attach(MainForm::mainFormWidget.chan24Plot);
                    MainForm::cellDataRay[tbi].pCntlChanPlot->attach(MainForm::mainFormWidget.chan24Plot);
                } else {
                    MainForm::cellDataRay[tbi].pBandCurve->attach(MainForm::mainFormWidget.chan5Plot);
                    MainForm::cellDataRay[tbi].pCntlChanPlot->attach(MainForm::mainFormWidget.chan5Plot);
                }
                MainForm::cellDataRay[tbi].pSignalTimeMarker->attach(MainForm::mainFormWidget.timePlot);
                MainForm::cellDataRay[tbi].firstPlot = false;
            }
        } else {
            MainForm::cellDataRay[tbi].pChanSymbol->setStyle(QwtSymbol::NoSymbol);
            MainForm::cellDataRay[tbi].pCntlChanPlot->setLabel(QwtText(""));
        }
        
        // then the 2.5GHz and 5GHz channel vs signal plots
        float spread = MainForm::cellDataRay[tbi].BW / 10.0;
        MainForm::cellDataRay[tbi].xPlot[0] = MainForm::cellDataRay[tbi].cenChan - spread;
        MainForm::cellDataRay[tbi].xPlot[1] = MainForm::cellDataRay[tbi].cenChan - spread + 1.0;
        MainForm::cellDataRay[tbi].xPlot[2] = MainForm::cellDataRay[tbi].cenChan + spread - 1.0;
        MainForm::cellDataRay[tbi].xPlot[3] = MainForm::cellDataRay[tbi].cenChan + spread;
        MainForm::cellDataRay[tbi].yPlot[0] = MainForm::cellDataRay[tbi].yPlot[3] = -100.0;
        MainForm::cellDataRay[tbi].yPlot[1] = MainForm::cellDataRay[tbi].yPlot[2]
                = MainForm::cellDataRay[tbi].signal;
        if (shouldBePlot(tbi)) {
            MainForm::cellDataRay[tbi].pBandCurve->setRawSamples(MainForm::cellDataRay[tbi].xPlot,
                    MainForm::cellDataRay[tbi].yPlot, 4);
                // here we plot a point for the control channel
                MainForm::cellDataRay[tbi].pCntlChanPlot->setValue( 
                    QPointF((float) MainForm::cellDataRay[tbi].channel, 
                    MainForm::cellDataRay[tbi].signal));
        } else {
            MainForm::cellDataRay[tbi].pBandCurve->setSamples(0, 0, 0);
        }

        // now the signal history plot
        int ixStart;
        int ixLength;
        int numSamples = MainForm::cellDataRay[tbi].pHistory->totalSamples;
        if (numSamples < MAX_SAMPLES) {
            ixLength = numSamples;
            ixStart = 0;
        } else {
            ixLength = MAX_SAMPLES;
            ixStart = numSamples % MAX_SAMPLES;
        }
        if (shouldBePlot(tbi)) {
            MainForm::cellDataRay[tbi].pTimeCurve->setRawSamples(
                    &(MainForm::cellDataRay[tbi].pHistory->sampleSec[ixStart]),
                    &(MainForm::cellDataRay[tbi].pHistory->signal[ixStart]), ixLength);
            // Place the marker where the latest data point show up
            MainForm::cellDataRay[tbi].pSignalTimeMarker->setValue( 
                QPointF((float)MainForm::cellDataRay[tbi].pHistory->sampleSec[ixStart+ixLength-1],
                MainForm::cellDataRay[tbi].signal));
            if (this->plotShowLabel) {
                QwtText markerLabel = QString::fromStdString(MainForm::cellDataRay[tbi].essid);
                markerLabel.setColor(MainForm::cellDataRay[tbi].color);
                MainForm::cellDataRay[tbi].pSignalTimeMarker->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
                MainForm::cellDataRay[tbi].pSignalTimeMarker->setLabel(markerLabel);
            } else {
                MainForm::cellDataRay[tbi].pSignalTimeMarker->setLabel(QwtText(""));
            }
        } else {
            MainForm::cellDataRay[tbi].pTimeCurve->setSamples(0, 0, 0);
            MainForm::cellDataRay[tbi].pSignalTimeMarker->setLabel(QwtText(""));
        }
    }
    MainForm::mainFormWidget.chan24Plot->replot();
    MainForm::mainFormWidget.chan5Plot->replot();
    MainForm::mainFormWidget.timePlot->replot();
}

bool MainForm::shouldBePlot(int tbi)
{
    if (MainForm::cellDataRay[tbi].pTableItem[PLOT]->checkState() == Qt::Checked &&
        !proxyModel_->isFiltered(tbi)) // @TODO: Add a pref to allow plotting filtered table row
        return true;
    return false;
}

void MainForm::resolveMesh(int tbi) {
    string search24 = MainForm::cellDataRay[tbi].macAddr.substr(9);
    if (MainForm::cellDataRay[tbi].essid == "<hidden>") {
        for (int tbi2 = 0; tbi2 <= MainForm::maxTableIndex; tbi2++) {
            if (MainForm::cellDataRay[tbi2].macAddr.substr(9) == search24 &&
                    MainForm::cellDataRay[tbi2].essid != "<hidden>" &&
                    MainForm::cellDataRay[tbi2].essid.substr(0,6) != "<mesh ") {
                MainForm::cellDataRay[tbi].essid = "<mesh " + 
                MainForm::cellDataRay[tbi2].essid + ">";
                break;
            }
        }
    } else { // essid is not "<hidden>", so search for hiddens to resolve
        for (int tbi2 = 0; tbi2 <= MainForm::maxTableIndex; tbi2++) {
            if (MainForm::cellDataRay[tbi2].essid == "<hidden>" &&
                    MainForm::cellDataRay[tbi2].macAddr.substr(9) == search24) {
                MainForm::cellDataRay[tbi2].essid = "<mesh " + 
                MainForm::cellDataRay[tbi].essid + ">";
            }
        }
    }
}

void MainForm::initNewCell(string macAddress, int tbi) {
    // Initialize a newly found cell.
    MainForm::cellDataRay.push_back(CellData());
    MainForm::cellDataRay[tbi].macAddr = macAddress; // insert MAC address
    MainForm::cellDataRay[tbi].essid = "<hidden>"; // nl80211 iw doesn't report SSID line if hidden
    MainForm::cellDataRay[tbi].minSignal = 0;
    MainForm::cellDataRay[tbi].maxSignal = -120;
    MainForm::cellDataRay[tbi].firstSeen = now;
    MainForm::cellDataRay[tbi].firstPlot = true;
    MainForm::cellDataRay[tbi].protocol = "unknown";
    MainForm::cellDataRay[tbi].vendor = vendorDb->lookup(macAddress);
    MainForm::cellDataRay[tbi].pHistory = make_unique<History>(); // give it a history
    MainForm::cellDataRay[tbi].pTimeCurve = make_unique<QwtPlotCurve>(""); // and a history curve
    QColor tempColor = qColorArray[tbi % NUMBER_OF_COLORS];
    MainForm::cellDataRay[tbi].color = tempColor; // assign a color from the palette
    MainForm::cellDataRay[tbi].pTimeCurve->setPen(QPen(tempColor, 3.0));
    MainForm::cellDataRay[tbi].pTimeCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    MainForm::cellDataRay[tbi].pTimeCurve->attach(MainForm::mainFormWidget.timePlot);
    MainForm::cellDataRay[tbi].pBandCurve = make_unique<QwtPlotCurve>("");
    MainForm::cellDataRay[tbi].pBandCurve->setPen(QPen(MainForm::cellDataRay[tbi].color, 3.0));
    MainForm::cellDataRay[tbi].pBandCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    MainForm::cellDataRay[tbi].pCntlChanPlot = make_unique<QwtPlotMarker>(); // create plot for control channel symbol
    MainForm::cellDataRay[tbi].pChanSymbol = new QwtSymbol();
    MainForm::cellDataRay[tbi].pChanSymbol->setStyle(QwtSymbol::Diamond);
    MainForm::cellDataRay[tbi].pChanSymbol->setColor(tempColor);
    MainForm::cellDataRay[tbi].pChanSymbol->setSize(10, 10);
    MainForm::cellDataRay[tbi].pCntlChanPlot->setSymbol(MainForm::cellDataRay[tbi].pChanSymbol);
    MainForm::cellDataRay[tbi].pSignalTimeMarker = make_unique<QwtPlotMarker>();
    // attaching plot curve waits 'till know frequency
    model_->setRowCount(tbi + 1);
    for (int ix = 0; ix < MAX_TABLE_COLS; ix++) {
        MainForm::cellDataRay[tbi].pTableItem[ix] = make_unique<QStandardItem>(); // Give it a table item for each column
        model_->setItem(tbi, ix,
                MainForm::cellDataRay[tbi].pTableItem[ix].get()); // Give it a spot in the table
    }
    MainForm::cellDataRay[tbi].pTableItem[PLOT]->setFlags(
            Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren);
    MainForm::cellDataRay[tbi].pTableItem[PLOT]->setCheckState(Qt::Checked);
    MainForm::cellDataRay[tbi].pTableItem[PLOT]->setText("");
    MainForm::cellDataRay[tbi].pTableItem[SSID]->setForeground(QBrush(MainForm::cellDataRay[tbi].color));
}

void MainForm::extractData(string tl, int &tbi, int &newBSS) {
    // std::cout << "Dump: " << tl << endl;
    // extract the information from each line recovered from the pipe from getter
    boost::smatch sm;
    if (boost::regex_match(tl, sm, boost::regex("^BSS.*?(([A-Fa-f0-9]{2}:){5}[A-Fa-f0-9]{2}).*",
          boost::regex_constants::icase))) {
        pageBlock = BT_BSS;
        string macAddress = sm[1];
        //string macAddress = boost::regex_replace(tl,
        //        boost::regex(".+?((?:[A-Fa-f0-9]{2}:){5}[A-Fa-f0-9]{2}).*"), "$1");
        for (unsigned int ic=0; ic< macAddress.length(); ic++) macAddress[ic] = toupper(macAddress[ic]);
        tbi = MainForm::maxTableIndex + 1;
        for (int ix = 0; ix <= MainForm::maxTableIndex; ix++) {
            if (macAddress == MainForm::cellDataRay[ix].macAddr) {
                tbi = ix;
                newBSS = 0; // Have seen this BSS before
                break;
            }
        }
        if (MainForm::maxTableIndex < tbi) {
            // at this point we have found a **new** mac address. Initialize accordingly.
            MainForm::initNewCell(macAddress, tbi);
            MainForm::maxTableIndex = tbi;
            newBSS = 1; // Flag first time through for new BSS
        }
        MainForm::cellDataRay[tbi].timesSeen++;
        MainForm::cellDataRay[tbi].lastSeen = now;
        MainForm::cellDataRay[tbi].BW = 20; // all have at least 20 MHz bandwidth
        MainForm::cellDataRay[tbi].protocol = "";
    } 
       else if (boost::regex_match(tl, sm, boost::regex("^.+?SSID: +(.*)"))) {
        string tempSSID = sm[1];
        if (MainForm::cellDataRay[tbi].essid == "<hidden>" && tempSSID != "") {
            MainForm::cellDataRay[tbi].essid = tempSSID;
        }
    }  else if (boost::regex_match(tl, sm, boost::regex(
            "^[ \\t]+Supported rates: (.*)", boost::regex_constants::icase))) { // protocol
        string tempStr = sm[1];
        if (Utils::MinIntStr(tempStr) < 11) MainForm::cellDataRay[tbi].protocol += "b";
        if (Utils::MaxIntStr(tempStr) >= 11) MainForm::cellDataRay[tbi].protocol += "g";
    }  else if (boost::regex_match(tl, sm, boost::regex(
            "^[ \\t]+HT Capabilities:", boost::regex_constants::icase))) { // protocol
        pageBlock = BT_HT_CAPABILITIES;
        MainForm::cellDataRay[tbi].protocol += "n";
    }  else if (boost::regex_match(tl, sm, boost::regex(
            "^.*?VHT Capabilities:", boost::regex_constants::icase))) { // protocol
        pageBlock = BT_VHT_CAPABILITIES;
        MainForm::cellDataRay[tbi].protocol += "ac";
    } else if (boost::regex_match(tl, sm, boost::regex(".*?capability:.*?( ESS( .*|$))",
            boost::regex_constants::icase))) {
        MainForm::cellDataRay[tbi].mode = "AP"; // http://unix.stackexchange.com/questions/63069/the-n-mode-on-iwlist-wlan0-scan
    } else if (boost::regex_match(tl, sm, boost::regex(".*?capability:.*?( IBSS( .*|$))",
            boost::regex_constants::icase))) {
        MainForm::cellDataRay[tbi].mode = "Ad Hoc";
    } else if (boost::regex_match(tl, sm,
          boost::regex("^.*?primary channel: +([0-9]+).*", boost::regex_constants::icase))) {
        string tempChan = sm[1];
        MainForm::cellDataRay[tbi].channel = atoi(tempChan.c_str());
        MainForm::cellDataRay[tbi].cenChan = MainForm::cellDataRay[tbi].channel;
    } else if (boost::regex_match(tl, sm,
          boost::regex("^.*?DS Parameter set: +(channel)?.*?([0-9]+).*", boost::regex_constants::icase))
          && MainForm::cellDataRay[tbi].channel == 0) {
        string tempChan = sm[2];
        MainForm::cellDataRay[tbi].channel = atoi(tempChan.c_str());
        MainForm::cellDataRay[tbi].cenChan = MainForm::cellDataRay[tbi].channel;
    } else if (boost::regex_match(tl, sm, boost::regex("^.*?secondary channel offset: *([^ \\t]+).*",
          boost::regex_constants::icase))) { // secondary channel offset
        string tempString = sm[1];
        if (tempString == "above") MainForm::cellDataRay[tbi].cenChan = MainForm::cellDataRay[tbi].channel + 2;
        else if (tempString == "below") MainForm::cellDataRay[tbi].cenChan = MainForm::cellDataRay[tbi].channel - 2;
    } else if (boost::regex_match(tl, sm, 
            boost::regex("^.*?freq:.*?([0-9]+).*",
            boost::regex_constants::icase))) {
        string tempFreq = sm[1];
        MainForm::cellDataRay[tbi].frequency = tempFreq;
    } else if (boost::regex_match(tl, sm, boost::regex("^.*?signal:.*?([\\-0-9]+).*?dBm.*",
            boost::regex_constants::icase))) {
            string tempSig = sm[1];
            MainForm::cellDataRay[tbi].signal = atoi(tempSig.c_str());
            if (MainForm::cellDataRay[tbi].signal < MainForm::cellDataRay[tbi].minSignal)
                MainForm::cellDataRay[tbi].minSignal = MainForm::cellDataRay[tbi].signal;
            if (MainForm::cellDataRay[tbi].signal > MainForm::cellDataRay[tbi].maxSignal)
                MainForm::cellDataRay[tbi].maxSignal = MainForm::cellDataRay[tbi].signal;
            // add to history
            if (MainForm::cellDataRay[tbi].timesSeen == 1) {
                int ixTemp = MainForm::cellDataRay[tbi].pHistory->totalSamples % MAX_SAMPLES;
                MainForm::cellDataRay[tbi].pHistory->sampleSec[ixTemp] = MainForm::blockSampleTime;
                MainForm::cellDataRay[tbi].pHistory->sampleSec[ixTemp + MAX_SAMPLES]
                    = MainForm::blockSampleTime;
                MainForm::cellDataRay[tbi].pHistory->signal[ixTemp] = MainForm::cellDataRay[tbi].signal;
                MainForm::cellDataRay[tbi].pHistory->signal[ixTemp + MAX_SAMPLES] = MainForm::cellDataRay[tbi].signal;
                MainForm::cellDataRay[tbi].pHistory->totalSamples++;
            } else {
                int ixTemp = (MainForm::cellDataRay[tbi].pHistory->totalSamples - 1) % MAX_SAMPLES;
                MainForm::cellDataRay[tbi].pHistory->sampleSec[ixTemp] = MainForm::blockSampleTime;
                MainForm::cellDataRay[tbi].pHistory->sampleSec[ixTemp + MAX_SAMPLES]
                    = MainForm::blockSampleTime;
                MainForm::cellDataRay[tbi].pHistory->signal[ixTemp] = MainForm::cellDataRay[tbi].signal;
                MainForm::cellDataRay[tbi].pHistory->signal[ixTemp + MAX_SAMPLES] = MainForm::cellDataRay[tbi].signal;
            }
            if (MainForm::cellDataRay[tbi].signal <= -100) MainForm::cellDataRay[tbi].quality = 0;
            else if(MainForm::cellDataRay[tbi].signal >= -50) MainForm::cellDataRay[tbi].quality = 100;
            else MainForm::cellDataRay[tbi].quality = 2 * (MainForm::cellDataRay[tbi].signal + 100);
    } else if (boost::regex_match(tl, sm, boost::regex(".*?Group cipher: *(.*)",
            boost::regex_constants::icase))) { // group cipher
        MainForm::cellDataRay[tbi].privacy = sm[1];
    } else if (pageBlock == BT_HT_CAPABILITIES && boost::regex_match(tl, sm, boost::regex(".*?HT20/HT40.*",
            boost::regex_constants::icase))) { // Bandwidth HT 40 MHz
        MainForm::cellDataRay[tbi].BW = 40;
    } else if (pageBlock == BT_VHT_OPERATION && boost::regex_match(tl, sm, boost::regex(".*?\\* channel width:.*?([0-9]).*?([0-9]+) MHz.*",
            boost::regex_constants::icase))) { // Bandwidth VHT
        int val = atoi(string(sm[1]).c_str());
        if (val == 0) return; // 0 (20 or 40 MHz) - BW from HT operation should be used
        string bwString = sm[2];
        MainForm::cellDataRay[tbi].BW = atoi(bwString.c_str());
    } else if (boost::regex_match(tl, sm, boost::regex(".*?Pairwise ciphers: *(.*)",
            boost::regex_constants::icase))) { // pairwise ciphers
        MainForm::cellDataRay[tbi].cipher = sm[1];
    } else if (boost::regex_match(tl, sm, boost::regex(".*?Authentication suites: *(.*)",
            boost::regex_constants::icase))) { // authentication
            MainForm::cellDataRay[tbi].security = sm[1];
    } else if (boost::regex_match(tl, sm, boost::regex(".*?RSN: *(.*)",
            boost::regex_constants::icase))) { pageBlock = BT_RSN;
    } else if (boost::regex_match(tl, sm, boost::regex(".*?BSS Load: *(.*)",
            boost::regex_constants::icase))) { pageBlock = BT_BSS_LOAD;
    } else if (pageBlock == BT_BSS_LOAD && boost::regex_match(tl, sm, boost::regex("^.*?channel util.*?:.*?([0-9]+)/([0-9]+).*",
            boost::regex_constants::icase))) {
            int x = atoi(string(sm[1]).c_str());
            int y = atoi(string(sm[2]).c_str());
            if (y > 0) {
                MainForm::cellDataRay[tbi].load = (int)(x * 100 / y);
            }
    } else if (pageBlock == BT_BSS_LOAD && boost::regex_match(tl, sm, boost::regex("^.*?station count:.*?([0-9]+).*",
            boost::regex_constants::icase))) {
                MainForm::cellDataRay[tbi].stationCount = atoi(string(sm[1]).c_str());
    } else if (boost::regex_match(tl, sm, boost::regex("[^V]*HT operation: *(.*)",
            boost::regex_constants::icase))) { pageBlock = BT_HT_OPERATION;
    } else if (boost::regex_match(tl, sm, boost::regex(".*?Extended capabilities: *(.*)",
            boost::regex_constants::icase))) { pageBlock = BT_EXTENDED_CAPABILITIES;
    } else if (boost::regex_match(tl, sm, boost::regex(".*?VHT operation:.*",
            boost::regex_constants::icase))) { pageBlock = BT_VHT_OPERATION;
    } else if (boost::regex_match(tl, sm, boost::regex(".*?WPA: *(.*)",
            boost::regex_constants::icase))) { pageBlock = BT_WPA;
    } else if (boost::regex_match(tl, sm, boost::regex(".*?WPS: *(.*)",
            boost::regex_constants::icase))) { pageBlock = BT_WPS;
    } else if (boost::regex_match(tl, sm, boost::regex(".*?WMM: *(.*)",
            boost::regex_constants::icase))) { pageBlock = BT_WMM;
    }
} // extractData()

void MainForm::handleDataReadyEvent(const DataReadyEvent * /*event*/) {
    // Now something can be safely done with the Qt objects.
    // Access the custom data using event->getReadyBlockNo() etc.
    extern runStates runState;
    static int newBSS;
    QCoreApplication::processEvents(QEventLoop::AllEvents); // TODO is this necessary?

    static fstream thePipe(pipeName);
    // reset some stuff from last pass
    for (int ix = 0; ix <= maxTableIndex; ix++) {
        MainForm::cellDataRay[ix].protocol = "";
        MainForm::cellDataRay[ix].signal = -110;
        MainForm::cellDataRay[ix].timesSeen = 0;
    }
    // disable column sorting to prevent segfaults while we muck with the table
    MainForm::mainFormWidget.mainTableView->setSortingEnabled(false);
    string tempLine;
    bool lastLine = false;
    int block;
    MainForm::now = time(NULL);
    MainForm::blockSampleTime = now - MainForm::runStartTime;
    int tableIndex = -1; // holds current index pointer into cellData
    while (!lastLine && getline(thePipe, tempLine)) {
        MainForm::extractData(tempLine, tableIndex, newBSS); // the heavy lifting here
        lastLine = tempLine.substr(0, endBlockString.length()) == endBlockString;
        if (lastLine) {
            block = atoi(tempLine.substr(endBlockString.length() + 1, std::string::npos).c_str());
            if (block >= 0) lastBlockReceived = block;
            if ( MainForm::mainFormWidget.runBtn->isChecked()) {
                runState = RUNNING;
                if (block >= 0) lastBlockRequested++;
            } else {
                MainForm::mainFormWidget.statusTxt->setText("Paused");
                if (runState == RUNNING) runState = STOPPING;
            }
            if (block >= 0) {
                MainForm::fillTable();
                MainForm::fillPlots();
                MainForm::fillStatus();
                if (MainForm::logDataState == Qt::Checked) {
                    dataLogger->log(MainForm::cellDataRay);
                }
            }
        }
    }
}