/*
 * File:   main.cpp
 * Author: warren
 *
 * Created on October 25, 2012, 11:41 AM
 */

#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <QCommandLineParser>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <QThread>
#include <sys/stat.h>
#include "MainForm.h"
#include "ui_MainForm.h"
#include "Getter.h"
#include "Custom.h"
#include "Logger.h"

using namespace std;

// Globals here - TODO make some local where possible
string pipeName;
string genPipeName(int);
string endBlockString = "-=-=-=-=-=-End block";
string beginBlockString = "-=-=-=-=-=-Begin block";
int lastBlockRequested;
int lastBlockReceived;
qint64 startTime;
bool stopping = false;
runStates runState=STOPPED;
int realUID;
struct passwd *realUser;
string storageDir;
Logger AppLogger("App");

int main(int argc, char *argv[]) {
    // initialize resources, if needed
    // Q_INIT_RESOURCE(resfile);

    QApplication app(argc, argv);

    // make sure we're running as root, otherwise bag it.
    if (geteuid() != 0) {
        QMessageBox messageBox;
        messageBox.critical(0,"Error",
        "You are attempting to run LinSSID without root privilege.\n\
It will not work. Try linssid-pkexec instead. Sorry. Goodbye.");
        messageBox.setFixedSize(500,200);
        exit(1);
    }

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOptions({
        {{"d", "debug"},
            QCoreApplication::translate("main", "Set debug logging level in {None, Error, Warn, Info, Debug, Verbose}.\nDefault: Error"),
            QCoreApplication::translate("main", "LEVEL")},
    });
    parser.process(app);
    LogLevel level = LogLevel::Error;
    QString lvlString = parser.value("debug");
    level = Logger::ToLogLevel(lvlString.toStdString());
    AppLogger.setLevel(level);
    QStringList args = app.arguments();
    VerboseLog(AppLogger) << "Start with " << args.count() << " args:";
    for (int i = 0; i < args.count(); ++i)
        VerboseLog(AppLogger) << "  > " << args.at(i).toStdString();

    // Find the real user if launched from pkexec
    // If launched with sudo, then real UID is 0
    if (getenv("PKEXEC_UID") != nullptr) {
        realUID = atoi(getenv("PKEXEC_UID"));
    } else {
        realUID = 0;
    }
    realUser = getpwuid(realUID);
    storageDir = string(realUser->pw_dir) + "/";

    //  create instances of the main GUI and the worker thread and initialize
    Getter getter1; // instance of Getter
    MainForm form1; // instance of MainForm
    // make a thread for the getter and "movetothread"
    static QThread getter1Thread;
    getter1.moveToThread(&getter1Thread);
    pipeName = genPipeName(10);
    mkfifo(pipeName.c_str(), 0666);
    form1.init(); // my own kludgy init, not part of constructor

    // Tell form1 and getter1 how to find each other
    form1.pGetter = &getter1; // form1 has a pointer to getter1
    getter1.pMainForm = &form1; // and getter1 has a pointer to form1
    form1.pGetterThread = &getter1Thread; // and form1 has a pointer to getter1 thread

    //  fire up the event loops and run the app
    getter1Thread.start(); // start getter1 event loop
    form1.show();
    
    return (app.exec());
}

string genPipeName(int len) {
    string charSet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    string name = "/tmp/linssid_";
    srand(time(NULL));
    for (int i = 0; i < len; i++) {
        name = name + charSet[rand() % charSet.size()];
    }
    return (name);
}
