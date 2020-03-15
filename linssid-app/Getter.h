/*
 * File:   Getter.h
 * Author: warren
 *
 * Created on October 31, 2012, 8:59 AM
 */

#ifndef GETTER_H
#define	GETTER_H

#include <QThread>
#include <QEvent>
#include "Custom.h"
using namespace std;

class MainForm; //  forward declare

class Getter : public QThread {
    Q_OBJECT
public:
    Getter();
    virtual ~Getter();

    static MainForm* pMainForm; // a pointer to the instance of the MainForm that calls this Getter

    void run();

    static const QEvent::Type DATA_WANTED_EVENT;

    class DataWantedEvent;

    int getWantedBlockNo();

    void postDataWantedEvent(const int);
    
    inline void waste(int);

protected:
    void customEvent(QEvent *); // This overrides QObject::customEvent()

private:
    void handleDataWantedEvent(const DataWantedEvent *);
};

#endif	/* GETTER_H */

