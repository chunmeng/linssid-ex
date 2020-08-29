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
#include "CustomEvent.h"
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
#include "Logger.h"

using namespace std;

extern int lastBlockRequested;
extern int lastBlockReceived;
extern qint64 startTime;
extern string endBlockString;
extern string pipeName;
extern runStates runstate;
extern int realUID;
extern struct passwd *realUser;
extern string storageDir;
extern Logger AppLogger;

extern string genPipeName(int);

// define a few things

// declare some variables
Getter* MainForm::pGetter; // a pointer to our data getter
QThread* MainForm::pGetterThread; // a pointer to its thread
pageBlockType pageBlock; // which section of page is data coming from
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

    mainFormWidget.setupUi(this);
    // Button widget actions
    connect(mainFormWidget.runBtn, SIGNAL(clicked()), this, SLOT(doRun()));
    connect(mainFormWidget.allBtn, SIGNAL(clicked()), this, SLOT(doPlotAll()));
    connect(mainFormWidget.noneBtn, SIGNAL(clicked()), this, SLOT(doPlotNone()));
    connect(mainFormWidget.filterBtn, &QPushButton::clicked, [this]() {
            mainFormWidget.filterBtn->setEnabled(!mainFormWidget.filterBtn->isChecked());
            showViewFilterDlg();
        });
    // Menu item actions
    connect(mainFormWidget.actionSSID, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionMAC, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionChannel, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionMode, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionProtocol, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionSecurity, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionPrivacy, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionCipher, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionFrequency, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionQuality, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionSignal, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionLoad, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionStationCount, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionBW, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionMin_Signal, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionMax_Signal, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionCenChan, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionFirst_Seen, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionLast_Seen, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionVendor, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionType, SIGNAL(changed()), this, SLOT(reDrawTable()));
    connect(mainFormWidget.actionAbout, SIGNAL(triggered()), this, SLOT(showAboutBox()));
    connect(mainFormWidget.actionPrefs, SIGNAL(triggered()), this, SLOT(showPrefsDlg()));

    model_ = make_unique<QStandardItemModel>();
    model_->setColumnCount(MAX_TABLE_COLS);
    // Make sure the column labels below are same order as the enum <colTitle>
    model_->setHorizontalHeaderLabels(
            QString("Plot|SSID|MAC|Channel|Mode|Security|Privacy|Cipher|Frequency\
|Quality|Signal|Load|Station Count|BW MHz|Min Sig|Max Sig|Cen Chan|First Seen|Last Seen|Vendor|Protocol|Type").split("|"));
    proxyModel_ = make_unique<DataProxyModel>();
    proxyModel_->load(storageDir + string(FILTER_PREFS_FILE_NAME));
    proxyModel_->setSourceModel(model_.get());
    mainFormWidget.mainTableView->setModel(proxyModel_.get());

    // https://stackoverflow.com/questions/19442050/qtableview-how-can-i-get-the-data-when-user-click-on-a-particular-cell-using-mo
    connect(mainFormWidget.mainTableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(doTableClicked(const QModelIndex &)));
    connect(mainFormWidget.mainTableView->horizontalHeader(),
            SIGNAL(sectionResized(int, int, int)), this, SLOT(columnWidthSave(int, int, int)));

    cellDataRay_.reserve(50); // expect about 50 cells to be found. More or less is OK.
    lastBlockRequested = 0; // these global variables should be made protected
    lastBlockReceived = 0;
    startTime = QDateTime::currentMSecsSinceEpoch();
    firstScan_ = true;
}

MainForm::~MainForm() {
    MainForm::pGetter->quit();
}

void MainForm::init() {
    srand(time(NULL)); //initialize the random number generator seed
    MainForm::addInterfaces(); // find the wifi interface names and add them to the comboBox
    MainForm::setInterface(0); // set the interface select arbitrarily to 0
    maxTableIndex_ = -1;
    MainForm::drawTable();
    MainForm::initPlotGrids(); // must do before reading prefs, since prefs will modify
    MainForm::initColtoAction(); // init pointers to view menu items
    MainForm::initStatusBar();
    MainForm::loadPrefs();
    MainForm::drawTable(); // do it again after application of prefs
    MainForm::drawChan24Plot();
    MainForm::drawChan5Plot();
    MainForm::drawTimePlot();
    dataLogger_ = make_unique<DataLogger>(storageDir + string(LOG_DATA_FILE_NAME));
    vendorDb_ = make_unique<VendorDb>();
}

void MainForm::initColtoAction() {
    // brutal -- set the array of pointers from columns to their menu items
    // these should have the same elements as the enum in 'custom.h'
    // PLOT, SSID, MAC, CHANNEL, MODE, SECURITY, PRIVACY,
    // CIPHER, FREQUENCY, QUALITY, SIGNAL, BW, MINSIGNAL, MAXSIGNAL, CENCHAN,
    // FIRST_SEEN, LAST_SEEN, VENDOR, PROTOCOL
    MainForm::colToQAction[PLOT] = mainFormWidget.actionPlot;
    MainForm::colToQAction[SSID] = mainFormWidget.actionSSID;
    MainForm::colToQAction[MAC] = mainFormWidget.actionMAC;
    MainForm::colToQAction[CHANNEL] = mainFormWidget.actionChannel;
    MainForm::colToQAction[MODE] = mainFormWidget.actionMode;
    MainForm::colToQAction[PROTOCOL] = mainFormWidget.actionProtocol;
    MainForm::colToQAction[SECURITY] = mainFormWidget.actionSecurity;
    MainForm::colToQAction[PRIVACY] = mainFormWidget.actionPrivacy;
    MainForm::colToQAction[CIPHER] = mainFormWidget.actionCipher;
    MainForm::colToQAction[FREQUENCY] = mainFormWidget.actionFrequency;
    MainForm::colToQAction[QUALITY] = mainFormWidget.actionQuality;
    MainForm::colToQAction[SIGNAL] = mainFormWidget.actionSignal;
    MainForm::colToQAction[LOAD] = mainFormWidget.actionLoad;
    MainForm::colToQAction[STATION_COUNT] = mainFormWidget.actionStationCount;
    MainForm::colToQAction[BW] = mainFormWidget.actionBW;
    MainForm::colToQAction[MINSIGNAL] = mainFormWidget.actionMin_Signal;
    MainForm::colToQAction[MAXSIGNAL] = mainFormWidget.actionMax_Signal;
    MainForm::colToQAction[CENCHAN] = mainFormWidget.actionCenChan;
    MainForm::colToQAction[FIRST_SEEN] = mainFormWidget.actionFirst_Seen;
    MainForm::colToQAction[LAST_SEEN] = mainFormWidget.actionLast_Seen;
    MainForm::colToQAction[VENDOR] = mainFormWidget.actionVendor;
}

void MainForm::initPlotGrids() {
    // add some grids to our plots
    chan24Grid_ = make_unique<QwtPlotGrid>();
    chan24Grid_->enableX(false);
    chan24Grid_->attach(mainFormWidget.chan24Plot);
    chan5Grid_ = make_unique<QwtPlotGrid>();
    chan5Grid_->enableX(false);
    chan5Grid_->attach(mainFormWidget.chan5Plot);
    timeGrid_ = make_unique<QwtPlotGrid>();
    timeGrid_->enableX(false);
    timeGrid_->attach(mainFormWidget.timePlot);
}

void MainForm::initStatusBar() {
    statusCounts_ = make_unique<QLabel>();
    statusCounts_->setText(QString::fromStdString((stats_.toString())));
    mainFormWidget.statusbar->addPermanentWidget(statusCounts_.get());
}

void MainForm::fillStatus() {
    statusCounts_->setText(QString::fromStdString((stats_.toString())));
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
    mainFormWidget.interfaceCbx->setCurrentIndex(ifIndx);
}

string MainForm::getCurrentInterface() {
    return ((mainFormWidget.interfaceCbx->currentText()).toStdString());
}

int MainForm::getNapTime() {
    return mainFormWidget.napTimeSlider->value();
}

void MainForm::applyPlotPrefs(int fntSize, int plotMin, int plotMax, bool showGrid) {
    MainForm::tblFnt.setPointSize(fntSize);
    mainFormWidget.timePlot->setAxisScale(QwtPlot::yLeft, plotMin, plotMax, 20);
    mainFormWidget.chan24Plot->setAxisScale(QwtPlot::yLeft, plotMin, plotMax, 20);
    mainFormWidget.chan5Plot->setAxisScale(QwtPlot::yLeft, plotMin, plotMax, 20);
    timeGrid_->enableY(showGrid);
    chan24Grid_->enableY(showGrid);
    chan5Grid_->enableY(showGrid);
}

void MainForm::updatePlotPrefs(QString tblFntSize, int plotMin, int plotMax, bool showGrid, bool showLabel) {
    // a slot called from the prefs dialog to dynamically update the plots
    applyPlotPrefs(tblFntSize.toInt(), plotMin, plotMax, showGrid);
    plotShowLabel_ = showLabel;
    MainForm::reDrawTable();
    mainFormWidget.timePlot->replot();
    mainFormWidget.chan24Plot->replot();
    mainFormWidget.chan5Plot->replot();
}

void MainForm::logPrefChanged(int state) {
    logDataState_ = state;
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
    appPref.sort = {.column = mainFormWidget.mainTableView->horizontalHeader()->sortIndicatorSection(),
                    .order = mainFormWidget.mainTableView->horizontalHeader()->sortIndicatorOrder()};
    appPref.maingeom = {.x = this->x(), .y = this->y(),
                        .width = this->width(), .height = this->height()};
    appPref.mainsplit = {.topheight = mainFormWidget.splitter->sizes().value(0),
                         .bottomheight = mainFormWidget.splitter->sizes().value(1)};
    appPref.plottab = mainFormWidget.mainTabWgt->currentIndex();
    appPref.naptime = mainFormWidget.napTimeSlider->value();
    appPref.plotprefs = {.fntSize = MainForm::tblFnt.pointSize(),
                         .plotlb = static_cast<int>(mainFormWidget.timePlot->axisScaleDiv(QwtPlot::yLeft).lowerBound()),
                         .plotub = static_cast<int>(mainFormWidget.timePlot->axisScaleDiv(QwtPlot::yLeft).upperBound()),
                         .showgrid = timeGrid_->yEnabled(),
                         .showLabel = plotShowLabel_};
    appPref.logData = logDataState_;
    if (prefsHandler_) prefsHandler_->save(appPref);
}

void MainForm::loadPrefs() {
    string fullPrefsName = storageDir + string(PREFS_FILE_NAME);
    if (!prefsHandler_) prefsHandler_ = make_unique<PrefsHandler>(fullPrefsName);
    PrefsHandler::sDefPref appPref = prefsHandler_->load();

    for (int col = 0; col < MAX_TABLE_COLS; col++) {
        if (appPref.colwidth[col] > 0) {
            mainFormWidget.mainTableView->setColumnWidth(col, appPref.colwidth[col]);
            columnWidth[col] = appPref.colwidth[col];
        }
    }

    for (int col = 0; col < MAX_TABLE_COLS; col++) {
        bool vis = appPref.colvis[col];
        MainForm::colToQAction[col]->setChecked(vis);
        mainFormWidget.mainTableView->setColumnHidden(col, !vis);
    }

    if (appPref.sort.column >= 0 && appPref.sort.column < MAX_TABLE_COLS) {
        mainFormWidget.mainTableView->horizontalHeader()->setSortIndicator(appPref.sort.column, Qt::SortOrder(appPref.sort.order));
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

    mainFormWidget.mainTabWgt->setCurrentIndex(appPref.plottab);
    mainFormWidget.splitter->setSizes({appPref.mainsplit.topheight, appPref.mainsplit.bottomheight});
    mainFormWidget.napTimeSlider->setValue(appPref.naptime);

    // Sanity is ensured by PrefsHandler
    int fntSize = appPref.plotprefs.fntSize;
    int plotLb = appPref.plotprefs.plotlb;
    int plotUb = appPref.plotprefs.plotub;
    bool showGrid = appPref.plotprefs.showgrid;
    applyPlotPrefs(fntSize, plotLb, plotUb, showGrid);
    mainFormWidget.mainTableView->setFont(tblFnt);
    plotShowLabel_ = appPref.plotprefs.showLabel;
    logDataState_ = appPref.logData;
}

void MainForm::postDataReadyEvent(const int customData1) {
    // This method (postDataReadyEvent) can be called from any thread
    QApplication::postEvent(this, new DataReadyEvent(customData1));
}

void MainForm::doRun() {
    extern runStates runState;
    if (mainFormWidget.runBtn->isChecked()) {
        if (firstScan_) {
            firstScan_ = false;
            runStartTime_ = time(NULL);
        }
        mainFormWidget.statusTxt->setText("Scanning ...");
        runState = RUNNING;
        MainForm::pGetter->Getter::postDataWantedEvent(++lastBlockRequested);
        MainForm::drawTable();
        MainForm::drawChan24Plot();
        MainForm::drawChan5Plot();
        MainForm::drawTimePlot();
    } else {
        if (runState != STOPPED) runState = STOPPING;
        mainFormWidget.statusTxt->setText("Waiting for wifi to terminate scan ...");
        while (runState != STOPPED) usleep(500 * 1000); // wait half a second and try again
        mainFormWidget.statusTxt->setText("Paused ...");
    }
}

void MainForm::doPlotAll() {
    for (int tbi = 0; tbi <= maxTableIndex_; tbi++) {
        cellDataRay_[tbi].pTableItem[PLOT]->setCheckState(Qt::Checked);
    }
    fillPlots();
}

void MainForm::doPlotNone() {
    for (int tbi = 0; tbi <= maxTableIndex_; tbi++) {
        cellDataRay_[tbi].pTableItem[PLOT]->setCheckState(Qt::Unchecked);
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
    if (prefsDlg_ != nullptr) return; // already a prefs dialog open somewhere...
    prefsDlg_ = make_unique<prefsDialog>(
            QString::number(MainForm::tblFnt.pointSize()),
            int(mainFormWidget.timePlot->axisScaleDiv(QwtPlot::yLeft).lowerBound()),
            int(mainFormWidget.timePlot->axisScaleDiv(QwtPlot::yLeft).upperBound()),
            timeGrid_->yEnabled(),
            plotShowLabel_,
            logDataState_,
            (QObject*)this);
    prefsDlg_->exec();
    prefsDlg_.reset();
}

void MainForm::showViewFilterDlg()
{
    if (viewFilterDlg_ == nullptr) {
        viewFilterDlg_ = make_unique<ViewFilterDialog>(this, (QObject*)this->proxyModel_.get(), this->proxyModel_->getFilter());
        // First time - adjust position relative to MainForm
        QPoint pos(this->x() + this->width() - viewFilterDlg_->width(), this->y() + 200);
        viewFilterDlg_->move(pos);
        DebugLog(AppLogger) << "Adjusting filter dialog to (" << pos.x() << ", " << pos.y()
            << "), parent is at (" << this->x() << "+" << this->width() << ", " << this->y() << "+" << this->height() << ")";

    }
    // Make modeless dialog that stay on top of the parent window (when it's on focus)
    // Follow this example: https://doc.qt.io/qt-5/qtwidgets-widgets-windowflags-example.html
    auto flags = viewFilterDlg_->windowFlags();
    viewFilterDlg_->setWindowFlags(flags | Qt::Tool);
    viewFilterDlg_->show();
}

void MainForm::handleFilterDialogClosed()
{
    mainFormWidget.filterBtn->setCheckable(true);
    mainFormWidget.filterBtn->setChecked(false);
    mainFormWidget.filterBtn->setEnabled(true);
}

void MainForm::columnWidthSave(int col, int oldWidth, int newWidth) {
    if (newWidth > 0) MainForm::columnWidth[col] = newWidth;
    else if (oldWidth > 0) MainForm::columnWidth[col] = oldWidth;
}

void MainForm::customEvent(QEvent * event) {
    // When we get here, we've crossed the thread boundary and are now
    // executing in the Qt object's thread
    if (event->type() == DataReadyEvent::Type()) {
        handleDataReadyEvent(static_cast<DataReadyEvent *> (event));
    } else if (event->type() == DialogClosedEvent::Type()) {
        auto e = static_cast<DialogClosedEvent *> (event);
        if (e->id() == FILTER_DIALOG_ID) {
            DebugLog(AppLogger) << "Event FilterDialogClosed received";
            handleFilterDialogClosed();
        }
    }
    // use more else ifs to handle other custom events
}

void MainForm::closeEvent(QCloseEvent * event) {
    extern runStates runState;
    if (runState == RUNNING) runState = STOPPING; // tell getter to stop what it's doing
    mainFormWidget.statusTxt->setText("Exiting, waiting for wifi driver ...");
    mainFormWidget.statusTxt->repaint();
    while (runState != STOPPED) usleep(500 * 1000); // wait until getter is stopped
    pGetterThread->QThread::quit();
    cellDataRay_.clear();
    savePrefs();
    if (proxyModel_) proxyModel_->save(storageDir + string(FILTER_PREFS_FILE_NAME));
    pGetterThread->QThread::wait();
    mainFormWidget.statusTxt->setText("Closing ...");
    mainFormWidget.statusTxt->repaint();
    remove(pipeName.c_str());
    dataLogger_.reset();
    //    QMainWindow::closeEvent(event);
    event->accept();
    std::exit(0); // that's the system exit, not the Qt version
}

void MainForm::drawTable() {
    model_->setRowCount(maxTableIndex_ + 1);
    setVisibleCols();
    mainFormWidget.mainTableView->horizontalHeader()->setSectionsMovable(true);
    mainFormWidget.mainTableView->horizontalHeader()
            ->setToolTip("Click to sort\nDrag and Drop to re-order\nClick and drag divider to fit");
    mainFormWidget.mainTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainForm::setVisibleCols() {
    // the Plot column is always visible
    mainFormWidget.mainTableView->setColumnHidden(SSID,
            !(mainFormWidget.actionSSID->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(MAC,
            !(mainFormWidget.actionMAC->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(CHANNEL,
            !(mainFormWidget.actionChannel->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(MODE,
            !(mainFormWidget.actionMode->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(PROTOCOL,
            !(mainFormWidget.actionProtocol->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(SECURITY,
            !(mainFormWidget.actionSecurity->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(PRIVACY,
            !(mainFormWidget.actionPrivacy->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(CIPHER,
            !(mainFormWidget.actionCipher->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(FREQUENCY,
            !(mainFormWidget.actionFrequency->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(QUALITY,
            !(mainFormWidget.actionQuality->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(SIGNAL,
            !(mainFormWidget.actionSignal->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(LOAD,
            !(mainFormWidget.actionLoad->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(STATION_COUNT,
            !(mainFormWidget.actionStationCount->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(BW,
            !(mainFormWidget.actionBW->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(MINSIGNAL,
            !(mainFormWidget.actionMin_Signal->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(MAXSIGNAL,
            !(mainFormWidget.actionMax_Signal->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(CENCHAN,
            !(mainFormWidget.actionCenChan->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(FIRST_SEEN,
            !(mainFormWidget.actionFirst_Seen->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(LAST_SEEN,
            !(mainFormWidget.actionLast_Seen->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(VENDOR,
            !(mainFormWidget.actionVendor->isChecked()));
    mainFormWidget.mainTableView->setColumnHidden(TYPE, true); // Not implemented. Always hide
}

void MainForm::fillTable() {
    stats_.reset();
    mainFormWidget.mainTableView->setFont(tblFnt);

    // fill in the x-y, also set each cell text alignment
    model_->setRowCount(maxTableIndex_ + 1);
    for (int row = 0; row <= maxTableIndex_; row++) {
        cellDataRay_[row].pTableItem[SSID]->
                setText(cellDataRay_[row].essid.c_str());
        if (cellDataRay_[row].essid == "<hidden>") stats_.totalHidden++;
        cellDataRay_[row].pTableItem[MAC]->
                setText(cellDataRay_[row].macAddr.c_str());
        cellDataRay_[row].pTableItem[MAC]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[CHANNEL]->
                setData(cellDataRay_[row].channel,Qt::DisplayRole);
        if (cellDataRay_[row].channel <= 14) stats_.total2GBss++;
        else stats_.total5GBss++;
        cellDataRay_[row].pTableItem[CHANNEL]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[MODE]->
                setText(cellDataRay_[row].mode.c_str());
        cellDataRay_[row].pTableItem[MODE]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[PROTOCOL]->
                setText(cellDataRay_[row].protocol.c_str());
        cellDataRay_[row].pTableItem[PROTOCOL]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[SECURITY]->
                setText(cellDataRay_[row].security.c_str());
        cellDataRay_[row].pTableItem[SECURITY]->setTextAlignment(Qt::AlignCenter);
        if (cellDataRay_[row].security.empty()) stats_.totalOpen++;
        cellDataRay_[row].pTableItem[PRIVACY]->
                setText((cellDataRay_[row].privacy).c_str());
        cellDataRay_[row].pTableItem[PRIVACY]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[CIPHER]->
                setText((cellDataRay_[row].cipher).c_str());
        cellDataRay_[row].pTableItem[CIPHER]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[FREQUENCY]->
                setText(cellDataRay_[row].frequency.c_str());
        cellDataRay_[row].pTableItem[FREQUENCY]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[QUALITY]->
                setData(cellDataRay_[row].quality, Qt::DisplayRole);
        cellDataRay_[row].pTableItem[QUALITY]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[SIGNAL]->
                setData(cellDataRay_[row].signal, Qt::DisplayRole);
        cellDataRay_[row].pTableItem[SIGNAL]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[LOAD]->
                setData((cellDataRay_[row].load < 0) ? QVariant("-") : QVariant(cellDataRay_[row].load),
                        Qt::DisplayRole);
        cellDataRay_[row].pTableItem[LOAD]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[STATION_COUNT]->
                setData((cellDataRay_[row].stationCount < 0) ? QVariant("-") : QVariant(cellDataRay_[row].stationCount),
                        Qt::DisplayRole);
        cellDataRay_[row].pTableItem[STATION_COUNT]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[BW]->
                setData(cellDataRay_[row].BW, Qt::DisplayRole);
        cellDataRay_[row].pTableItem[BW]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[MINSIGNAL]->
                setData(cellDataRay_[row].minSignal, Qt::DisplayRole);
        cellDataRay_[row].pTableItem[MINSIGNAL]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[MAXSIGNAL]->
                setData(cellDataRay_[row].maxSignal, Qt::DisplayRole);
        cellDataRay_[row].pTableItem[MAXSIGNAL]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[CENCHAN]->
                setData(cellDataRay_[row].cenChan, Qt::DisplayRole);
        cellDataRay_[row].pTableItem[CENCHAN]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[FIRST_SEEN]->
                setText(QDateTime::fromTime_t(cellDataRay_[row].firstSeen).toString("MM/dd-hh:mm:ss"));
        cellDataRay_[row].pTableItem[FIRST_SEEN]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[LAST_SEEN]->
                setText(QDateTime::fromTime_t(cellDataRay_[row].lastSeen).toString("MM/dd-hh:mm:ss"));
        cellDataRay_[row].pTableItem[LAST_SEEN]->setTextAlignment(Qt::AlignCenter);
        cellDataRay_[row].pTableItem[VENDOR]->
                setText(cellDataRay_[row].vendor.c_str());
    }
    setVisibleCols();
    mainFormWidget.mainTableView->setSortingEnabled(true);
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

    mainFormWidget.chan24Plot->setAxisScale(QwtPlot::xBottom, -1, 16, 1);
    mainFormWidget.chan24Plot->setAxisMaxMinor(QwtPlot::xBottom, 0);
    mainFormWidget.chan24Plot->setAxisScaleDraw(QwtPlot::xBottom, new Chan24ScaleDraw());
    mainFormWidget.chan24Plot->replot();
}

void MainForm::drawChan5Plot() {

    mainFormWidget.chan5Plot->setAxisScale(QwtPlot::xBottom, 0, 170, 10);
    mainFormWidget.chan5Plot->setAxisMaxMinor(QwtPlot::xBottom, 5);
    mainFormWidget.chan5Plot->setAxisScaleDraw(QwtPlot::xBottom, new Chan5ScaleDraw());
    mainFormWidget.chan5Plot->replot();
}

void MainForm::drawTimePlot() {

    mainFormWidget.timePlot->setAxisScale(QwtPlot::xBottom,
            blockSampleTime_ - TIME_PLOT_SCALE,
            blockSampleTime_, 10);
    mainFormWidget.timePlot->setAxisMaxMinor(QwtPlot::xBottom, 5);
    mainFormWidget.timePlot->replot();
}

void MainForm::fillPlots() {
    // rescale the time plot
    mainFormWidget.timePlot->setAxisScale(QwtPlot::xBottom,
            blockSampleTime_ - TIME_PLOT_SCALE,
            blockSampleTime_, 10);
    for (int tbi = 0; tbi <= maxTableIndex_; tbi++) {
        // first attach plots plots we couldn't before because of sparse data
        auto markerSymbol = const_cast<QwtSymbol *>(cellDataRay_[tbi].pCntlChanPlot->symbol());
        if (shouldBePlot(tbi)) {
            markerSymbol->setStyle(cellDataRay_[tbi].BW >= 40 ? QwtSymbol::Diamond : QwtSymbol::Triangle);

            if (plotShowLabel_) {
                QwtText markerLabel = QString::fromStdString(cellDataRay_[tbi].essid);
                markerLabel.setColor(cellDataRay_[tbi].color);
                int ub = static_cast<int>(mainFormWidget.timePlot->axisScaleDiv(QwtPlot::yLeft).upperBound());
                if (cellDataRay_[tbi].signal <= ub - 5)
                    cellDataRay_[tbi].pCntlChanPlot->setLabelAlignment(Qt::AlignCenter | Qt::AlignTop);
                else cellDataRay_[tbi].pCntlChanPlot->setLabelAlignment(Qt::AlignCenter | Qt::AlignBottom);
                cellDataRay_[tbi].pCntlChanPlot->setLabel(markerLabel);
            } else {
                cellDataRay_[tbi].pCntlChanPlot->setLabel(QwtText(""));
            }

            if (cellDataRay_[tbi].firstPlot) {
                resolveMesh(tbi);
                if (cellDataRay_[tbi].frequency.substr(0, 1) == "2") {
                    cellDataRay_[tbi].pBandCurve->attach(mainFormWidget.chan24Plot);
                    cellDataRay_[tbi].pCntlChanPlot->attach(mainFormWidget.chan24Plot);
                } else {
                    cellDataRay_[tbi].pBandCurve->attach(mainFormWidget.chan5Plot);
                    cellDataRay_[tbi].pCntlChanPlot->attach(mainFormWidget.chan5Plot);
                }
                cellDataRay_[tbi].pSignalTimeMarker->attach(mainFormWidget.timePlot);
                cellDataRay_[tbi].firstPlot = false;
            }
        } else {
            markerSymbol->setStyle(QwtSymbol::NoSymbol);
            cellDataRay_[tbi].pCntlChanPlot->setLabel(QwtText(""));
        }

        // then the 2.5GHz and 5GHz channel vs signal plots
        float spread = cellDataRay_[tbi].BW / 10.0;
        cellDataRay_[tbi].xPlot[0] = cellDataRay_[tbi].cenChan - spread;
        cellDataRay_[tbi].xPlot[1] = cellDataRay_[tbi].cenChan - spread + 1.0;
        cellDataRay_[tbi].xPlot[2] = cellDataRay_[tbi].cenChan + spread - 1.0;
        cellDataRay_[tbi].xPlot[3] = cellDataRay_[tbi].cenChan + spread;
        cellDataRay_[tbi].yPlot[0] = cellDataRay_[tbi].yPlot[3] = -100.0;
        cellDataRay_[tbi].yPlot[1] = cellDataRay_[tbi].yPlot[2]
                = cellDataRay_[tbi].signal;
        if (shouldBePlot(tbi)) {
            cellDataRay_[tbi].pBandCurve->setRawSamples(cellDataRay_[tbi].xPlot,
                    cellDataRay_[tbi].yPlot, 4);
                // here we plot a point for the control channel
                cellDataRay_[tbi].pCntlChanPlot->setValue(
                    QPointF((float) cellDataRay_[tbi].channel,
                    cellDataRay_[tbi].signal));
        } else {
            cellDataRay_[tbi].pBandCurve->setSamples(0, 0, 0);
        }

        // now the signal history plot
        int ixStart;
        int ixLength;
        int numSamples = cellDataRay_[tbi].pHistory->totalSamples;
        if (numSamples < MAX_SAMPLES) {
            ixLength = numSamples;
            ixStart = 0;
        } else {
            ixLength = MAX_SAMPLES;
            ixStart = numSamples % MAX_SAMPLES;
        }
        if (shouldBePlot(tbi)) {
            cellDataRay_[tbi].pTimeCurve->setRawSamples(
                    &(cellDataRay_[tbi].pHistory->sampleSec[ixStart]),
                    &(cellDataRay_[tbi].pHistory->signal[ixStart]), ixLength);
            // Place the marker where the latest data point show up
            cellDataRay_[tbi].pSignalTimeMarker->setValue(
                QPointF((float)cellDataRay_[tbi].pHistory->sampleSec[ixStart+ixLength-1],
                cellDataRay_[tbi].signal));
            if (plotShowLabel_) {
                QwtText markerLabel = QString::fromStdString(cellDataRay_[tbi].essid);
                markerLabel.setColor(cellDataRay_[tbi].color);
                cellDataRay_[tbi].pSignalTimeMarker->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
                cellDataRay_[tbi].pSignalTimeMarker->setLabel(markerLabel);
            } else {
                cellDataRay_[tbi].pSignalTimeMarker->setLabel(QwtText(""));
            }
        } else {
            cellDataRay_[tbi].pTimeCurve->setSamples(0, 0, 0);
            cellDataRay_[tbi].pSignalTimeMarker->setLabel(QwtText(""));
        }
    }
    mainFormWidget.chan24Plot->replot();
    mainFormWidget.chan5Plot->replot();
    mainFormWidget.timePlot->replot();
}

bool MainForm::shouldBePlot(int tbi)
{
    if (cellDataRay_[tbi].pTableItem[PLOT]->checkState() == Qt::Checked &&
        !proxyModel_->isFiltered(tbi)) // @TODO: Add a pref to allow plotting filtered table row
        return true;
    return false;
}

void MainForm::resolveMesh(int tbi) {
    string search24 = cellDataRay_[tbi].macAddr.substr(9);
    if (cellDataRay_[tbi].essid == "<hidden>") {
        for (int tbi2 = 0; tbi2 <= maxTableIndex_; tbi2++) {
            if (cellDataRay_[tbi2].macAddr.substr(9) == search24 &&
                    cellDataRay_[tbi2].essid != "<hidden>" &&
                    cellDataRay_[tbi2].essid.substr(0,6) != "<mesh ") {
                cellDataRay_[tbi].essid = "<mesh " +
                cellDataRay_[tbi2].essid + ">";
                break;
            }
        }
    } else { // essid is not "<hidden>", so search for hiddens to resolve
        for (int tbi2 = 0; tbi2 <= maxTableIndex_; tbi2++) {
            if (cellDataRay_[tbi2].essid == "<hidden>" &&
                    cellDataRay_[tbi2].macAddr.substr(9) == search24) {
                cellDataRay_[tbi2].essid = "<mesh " +
                cellDataRay_[tbi].essid + ">";
            }
        }
    }
}

void MainForm::initNewCell(string macAddress, int tbi) {
    // Initialize a newly found cell.
    cellDataRay_.push_back(CellData());
    cellDataRay_[tbi].macAddr = macAddress; // insert MAC address
    cellDataRay_[tbi].essid = "<hidden>"; // nl80211 iw doesn't report SSID line if hidden
    cellDataRay_[tbi].minSignal = 0;
    cellDataRay_[tbi].maxSignal = -120;
    cellDataRay_[tbi].firstSeen = now_;
    cellDataRay_[tbi].firstPlot = true;
    cellDataRay_[tbi].protocol = "unknown";
    cellDataRay_[tbi].vendor = vendorDb_->lookup(macAddress);
    cellDataRay_[tbi].pHistory = make_unique<History>(); // give it a history
    cellDataRay_[tbi].pTimeCurve = make_unique<QwtPlotCurve>(""); // and a history curve
    QColor tempColor = qColorArray[tbi % NUMBER_OF_COLORS];
    cellDataRay_[tbi].color = tempColor; // assign a color from the palette
    cellDataRay_[tbi].pTimeCurve->setPen(QPen(tempColor, 3.0));
    cellDataRay_[tbi].pTimeCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    cellDataRay_[tbi].pTimeCurve->attach(mainFormWidget.timePlot);
    cellDataRay_[tbi].pBandCurve = make_unique<QwtPlotCurve>("");
    cellDataRay_[tbi].pBandCurve->setPen(QPen(cellDataRay_[tbi].color, 3.0));
    cellDataRay_[tbi].pBandCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    cellDataRay_[tbi].pCntlChanPlot = make_unique<QwtPlotMarker>(); // create plot for control channel symbol
    // @NOTE: symbol is owned and freed by QwtPlotMarker
    auto markerSymbol = new QwtSymbol(QwtSymbol::Diamond);
    markerSymbol->setColor(tempColor);
    markerSymbol->setSize(10, 10);
    cellDataRay_[tbi].pCntlChanPlot->setSymbol(markerSymbol);
    cellDataRay_[tbi].pSignalTimeMarker = make_unique<QwtPlotMarker>();
    // attaching plot curve waits 'till know frequency
    model_->setRowCount(tbi + 1);
    for (int ix = 0; ix < MAX_TABLE_COLS; ix++) {
        cellDataRay_[tbi].pTableItem[ix] = make_unique<QStandardItem>(); // Give it a table item for each column
        model_->setItem(tbi, ix,
                cellDataRay_[tbi].pTableItem[ix].get()); // Give it a spot in the table
    }
    cellDataRay_[tbi].pTableItem[PLOT]->setFlags(
            Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren);
    cellDataRay_[tbi].pTableItem[PLOT]->setCheckState(Qt::Checked);
    cellDataRay_[tbi].pTableItem[PLOT]->setText("");
    cellDataRay_[tbi].pTableItem[SSID]->setForeground(QBrush(cellDataRay_[tbi].color));
}

void MainForm::extractData(string tl, int &tbi, int &newBSS) {
    // extract the information from each line recovered from the pipe from getter
    boost::smatch sm;
    if (boost::regex_match(tl, sm, boost::regex("^BSS.*?(([A-Fa-f0-9]{2}:){5}[A-Fa-f0-9]{2}).*",
          boost::regex_constants::icase))) {
        pageBlock = BT_BSS;
        string macAddress = sm[1];
        //string macAddress = boost::regex_replace(tl,
        //        boost::regex(".+?((?:[A-Fa-f0-9]{2}:){5}[A-Fa-f0-9]{2}).*"), "$1");
        for (unsigned int ic=0; ic< macAddress.length(); ic++)
            macAddress[ic] = toupper(macAddress[ic]);
        tbi = maxTableIndex_ + 1;
        for (int ix = 0; ix <= maxTableIndex_; ix++) {
            if (macAddress == cellDataRay_[ix].macAddr) {
                tbi = ix;
                newBSS = 0; // Have seen this BSS before
                break;
            }
        }
        if (maxTableIndex_ < tbi) {
            // at this point we have found a **new** mac address. Initialize accordingly.
            MainForm::initNewCell(macAddress, tbi);
            maxTableIndex_ = tbi;
            newBSS = 1; // Flag first time through for new BSS
        }
        cellDataRay_[tbi].timesSeen++;
        cellDataRay_[tbi].lastSeen = now_;
        cellDataRay_[tbi].BW = 20; // all have at least 20 MHz bandwidth
        cellDataRay_[tbi].protocol = "";
    } else if (boost::regex_match(tl, sm, boost::regex("^.+?SSID: +(.*)"))) {
        string tempSSID = sm[1];
        cellDataRay_[tbi].essid = !tempSSID.empty() ? tempSSID : "<hidden>";
    } else if (boost::regex_match(tl, sm, boost::regex(
            "^[ \\t]+Supported rates: (.*)", boost::regex_constants::icase))) { // protocol
        string tempStr = sm[1];
        if (Utils::MinIntStr(tempStr) < 11) cellDataRay_[tbi].protocol += "b";
        if (Utils::MaxIntStr(tempStr) >= 11) cellDataRay_[tbi].protocol += "g";
    }  else if (boost::regex_match(tl, sm, boost::regex(
            "^[ \\t]+HT Capabilities:", boost::regex_constants::icase))) { // protocol
        pageBlock = BT_HT_CAPABILITIES;
        cellDataRay_[tbi].protocol += "n";
    }  else if (boost::regex_match(tl, sm, boost::regex(
            "^.*?VHT Capabilities:", boost::regex_constants::icase))) { // protocol
        pageBlock = BT_VHT_CAPABILITIES;
        cellDataRay_[tbi].protocol += "ac";
    } else if (boost::regex_match(tl, sm, boost::regex(".*?capability:.*?( ESS( .*|$))",
            boost::regex_constants::icase))) {
        cellDataRay_[tbi].mode = "AP"; // http://unix.stackexchange.com/questions/63069/the-n-mode-on-iwlist-wlan0-scan
    } else if (boost::regex_match(tl, sm, boost::regex(".*?capability:.*?( IBSS( .*|$))",
            boost::regex_constants::icase))) {
        cellDataRay_[tbi].mode = "Ad Hoc";
    } else if (boost::regex_match(tl, sm,
          boost::regex("^.*?primary channel: +([0-9]+).*", boost::regex_constants::icase))) {
        string tempChan = sm[1];
        cellDataRay_[tbi].channel = atoi(tempChan.c_str());
        cellDataRay_[tbi].cenChan = cellDataRay_[tbi].channel;
    } else if (boost::regex_match(tl, sm,
          boost::regex("^.*?DS Parameter set: +(channel)?.*?([0-9]+).*", boost::regex_constants::icase))
          && cellDataRay_[tbi].channel == 0) {
        string tempChan = sm[2];
        cellDataRay_[tbi].channel = atoi(tempChan.c_str());
        cellDataRay_[tbi].cenChan = cellDataRay_[tbi].channel;
    } else if (boost::regex_match(tl, sm, boost::regex("^.*?secondary channel offset: *([^ \\t]+).*",
          boost::regex_constants::icase))) { // secondary channel offset
        string tempString = sm[1];
        if (tempString == "above") cellDataRay_[tbi].cenChan = cellDataRay_[tbi].channel + 2;
        else if (tempString == "below") cellDataRay_[tbi].cenChan = cellDataRay_[tbi].channel - 2;
    } else if (boost::regex_match(tl, sm,
            boost::regex("^.*?freq:.*?([0-9]+).*",
            boost::regex_constants::icase))) {
        string tempFreq = sm[1];
        cellDataRay_[tbi].frequency = tempFreq;
    } else if (boost::regex_match(tl, sm, boost::regex("^.*?signal:.*?([\\-0-9]+).*?dBm.*",
            boost::regex_constants::icase))) {
            string tempSig = sm[1];
            cellDataRay_[tbi].signal = atoi(tempSig.c_str());
            if (cellDataRay_[tbi].signal < cellDataRay_[tbi].minSignal)
                cellDataRay_[tbi].minSignal = cellDataRay_[tbi].signal;
            if (cellDataRay_[tbi].signal > cellDataRay_[tbi].maxSignal)
                cellDataRay_[tbi].maxSignal = cellDataRay_[tbi].signal;
            // add to history
            if (cellDataRay_[tbi].timesSeen == 1) {
                int ixTemp = cellDataRay_[tbi].pHistory->totalSamples % MAX_SAMPLES;
                cellDataRay_[tbi].pHistory->sampleSec[ixTemp] = blockSampleTime_;
                cellDataRay_[tbi].pHistory->sampleSec[ixTemp + MAX_SAMPLES]
                    = blockSampleTime_;
                cellDataRay_[tbi].pHistory->signal[ixTemp] = cellDataRay_[tbi].signal;
                cellDataRay_[tbi].pHistory->signal[ixTemp + MAX_SAMPLES] = cellDataRay_[tbi].signal;
                cellDataRay_[tbi].pHistory->totalSamples++;
            } else {
                int ixTemp = (cellDataRay_[tbi].pHistory->totalSamples - 1) % MAX_SAMPLES;
                cellDataRay_[tbi].pHistory->sampleSec[ixTemp] = blockSampleTime_;
                cellDataRay_[tbi].pHistory->sampleSec[ixTemp + MAX_SAMPLES]
                    = blockSampleTime_;
                cellDataRay_[tbi].pHistory->signal[ixTemp] = cellDataRay_[tbi].signal;
                cellDataRay_[tbi].pHistory->signal[ixTemp + MAX_SAMPLES] = cellDataRay_[tbi].signal;
            }
            if (cellDataRay_[tbi].signal <= -100) cellDataRay_[tbi].quality = 0;
            else if(cellDataRay_[tbi].signal >= -50) cellDataRay_[tbi].quality = 100;
            else cellDataRay_[tbi].quality = 2 * (cellDataRay_[tbi].signal + 100);
    } else if (boost::regex_match(tl, sm, boost::regex(".*?Group cipher: *(.*)",
            boost::regex_constants::icase))) { // group cipher
        cellDataRay_[tbi].privacy = sm[1];
    } else if (pageBlock == BT_HT_CAPABILITIES && boost::regex_match(tl, sm, boost::regex(".*?HT20/HT40.*",
            boost::regex_constants::icase))) { // HT Capabilities - HT20/HT40 if 40 MHz capable, actual BW is determined from HT Op
        cellDataRay_[tbi].BW = 40;
    } else if (pageBlock == BT_HT_OPERATION && boost::regex_match(tl, sm, boost::regex("^.*?STA channel width: (any|\\d+).*",
            boost::regex_constants::icase))) {
        string bwString = sm[1];
        if (bwString == "any") return; // Dont change, use bw derived from HT Capabilities
        cellDataRay_[tbi].BW = atoi(bwString.c_str());
    } else if (pageBlock == BT_VHT_OPERATION && boost::regex_match(tl, sm, boost::regex(".*?\\* channel width:.*?([0-9]).*?([0-9]+) MHz.*",
            boost::regex_constants::icase))) { // Bandwidth VHT
        int val = atoi(string(sm[1]).c_str());
        if (val == 0) return; // 0 (20 or 40 MHz) - BW from HT operation should be used
        string bwString = sm[2];
        cellDataRay_[tbi].BW = atoi(bwString.c_str());
    } else if (boost::regex_match(tl, sm, boost::regex(".*?Pairwise ciphers: *(.*)",
            boost::regex_constants::icase))) { // pairwise ciphers
        cellDataRay_[tbi].cipher = sm[1];
    } else if (boost::regex_match(tl, sm, boost::regex(".*?Authentication suites: *(.*)",
            boost::regex_constants::icase))) { // authentication
            cellDataRay_[tbi].security = sm[1];
    } else if (boost::regex_match(tl, sm, boost::regex(".*?RSN: *(.*)",
            boost::regex_constants::icase))) { pageBlock = BT_RSN;
    } else if (boost::regex_match(tl, sm, boost::regex(".*?BSS Load: *(.*)",
            boost::regex_constants::icase))) { pageBlock = BT_BSS_LOAD;
    } else if (pageBlock == BT_BSS_LOAD && boost::regex_match(tl, sm, boost::regex("^.*?channel util.*?:.*?([0-9]+)/([0-9]+).*",
            boost::regex_constants::icase))) {
            int x = atoi(string(sm[1]).c_str());
            int y = atoi(string(sm[2]).c_str());
            if (y > 0) {
                cellDataRay_[tbi].load = (int)(x * 100 / y);
            }
    } else if (pageBlock == BT_BSS_LOAD && boost::regex_match(tl, sm, boost::regex("^.*?station count:.*?([0-9]+).*",
            boost::regex_constants::icase))) {
                cellDataRay_[tbi].stationCount = atoi(string(sm[1]).c_str());
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
    for (int ix = 0; ix <= maxTableIndex_; ix++) {
        cellDataRay_[ix].protocol = "";
        cellDataRay_[ix].signal = -110;
        cellDataRay_[ix].timesSeen = 0;
    }
    // disable column sorting to prevent segfaults while we muck with the table
    mainFormWidget.mainTableView->setSortingEnabled(false);
    string tempLine;
    bool lastLine = false;
    int block;
    now_ = time(NULL);
    blockSampleTime_ = now_ - runStartTime_;
    int tableIndex = -1; // holds current index pointer into cellData
    while (!lastLine && getline(thePipe, tempLine)) {
        MainForm::extractData(tempLine, tableIndex, newBSS); // the heavy lifting here
        lastLine = tempLine.substr(0, endBlockString.length()) == endBlockString;
        if (lastLine) {
            block = atoi(tempLine.substr(endBlockString.length() + 1, std::string::npos).c_str());
            if (block >= 0) lastBlockReceived = block;
            if (mainFormWidget.runBtn->isChecked()) {
                runState = RUNNING;
                if (block >= 0) lastBlockRequested++;
            } else {
                mainFormWidget.statusTxt->setText("Paused");
                if (runState == RUNNING) runState = STOPPING;
            }
            if (block >= 0) {
                MainForm::fillTable();
                MainForm::fillPlots();
                MainForm::fillStatus();
                if (logDataState_ == Qt::Checked) {
                    dataLogger_->log(cellDataRay_);
                }
            }
        }
    }
}