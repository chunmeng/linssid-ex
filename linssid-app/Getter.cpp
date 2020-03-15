/*
 * File:   Getter.cpp
 * Author: warren
 *
 * Created on October 31, 2012, 8:59 AM
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <random>
#include <QtCore>
#include <QString>
#include <QThread>
#include <QtWidgets>
#include <stdio.h>
#include "Custom.h"
#include "Getter.h"
#include "MainForm.h"
#include "ui_MainForm.h"

extern string pipeName;
extern qint64 startTime;
extern string beginBlockString;
extern string endBlockString;
extern string pipeName;
// extern ofstream linssidLog;
extern runStates runState;

using namespace std;

Getter::Getter() {
};

Getter::~Getter() {
};

void Getter::run() {
    // do stuff here
    exec();
}

MainForm* Getter::pMainForm;

const QEvent::Type Getter::DATA_WANTED_EVENT = static_cast<QEvent::Type> (DATAWANTED);
// Define the custom event subclass

class Getter::DataWantedEvent : public QEvent {
public:

    DataWantedEvent(const int customData1) :
    QEvent(Getter::DATA_WANTED_EVENT), wantedBlockNo(customData1) {
    }

    int getWantedBlockNo() const {
        return wantedBlockNo;
    }
private:
    int wantedBlockNo;
};

void Getter::postDataWantedEvent(const int customData1) {
    // This method (postDataWantedEvent) can be called from any thread
    QApplication::postEvent(this, new DataWantedEvent(customData1));
}

void Getter::customEvent(QEvent * event) {
    // When we get here, we've crossed the thread boundary and are now
    // executing in the Qt object's thread
    if (event->type() == Getter::DATA_WANTED_EVENT) {
        handleDataWantedEvent(static_cast<DataWantedEvent *> (event));
    }
    // use more else ifs to handle other custom events
}

void Getter::handleDataWantedEvent(const DataWantedEvent *event) {
    string endCommand;
    static fstream thePipe(pipeName);
    thePipe.flush();
    int blockNo = event->getWantedBlockNo();
    int rcSysCall = 0;
    ostringstream cppIsStupid;
    while (true) {
        cppIsStupid.str("");
        cppIsStupid << " " << blockNo;
        string lineCommand = 
                "echo \"" + beginBlockString + cppIsStupid.str() + "\" >> " + pipeName;
//                "echo \"" + beginBlockString + cppIsStupid.str() + "\\n\" >> " + pipeName;
        lineCommand += " ; iw ";
        lineCommand += Getter::pMainForm->getCurrentInterface() + " scan >> " + pipeName;
        for (int retries = 0; retries < 10; retries++) {
            rcSysCall = system(lineCommand.c_str());
            if ((rcSysCall == 0) || (runState == STOPPING)) break;
            else QThread::msleep(500); // wait 500 msec and try again
        }
        if (rcSysCall != 0) { // retries exhausted on multiple failures
            endCommand = "echo \"" + endBlockString + " -1\\n\" >> " + pipeName;
            // linssidLog << "Getter: command failed block " << blockNo << endl;
        } else { // success!
            endCommand = "echo \"" + endBlockString + cppIsStupid.str() + "\\n\" >> " + pipeName;
            // linssidLog << "Getter: block returned " << blockNo << endl;
        }

        waste(system(endCommand.c_str())); // put the end block tag in the stream

        Getter::pMainForm->postDataReadyEvent(blockNo);
        blockNo+=1;
        
        if (runState == RUNNING) QThread::msleep(500);
        for (int wink=0; (wink < Getter::pMainForm->getNapTime()) && (runState == RUNNING); wink++) {
            QThread::msleep(1000);        
        }
        if (runState != RUNNING) break; // exit the infinite while loop
    }
    runState = STOPPED;
}

inline void Getter::waste(int) {
    // This silliness is to ignore an argument function's return code without
    // having the compiler whine about it.
}
