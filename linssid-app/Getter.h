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

class MainForm; //  forward declare
class DataWantedEvent;

class Getter : public QThread {
    Q_OBJECT
public:
    Getter();
    virtual ~Getter();

    static MainForm* pMainForm; // a pointer to the instance of the MainForm that calls this Getter

    void run();
    int getWantedBlockNo();
    void postDataWantedEvent(const int);

protected:
    void customEvent(QEvent *) override;

private:
    void handleDataWantedEvent(const DataWantedEvent *);
};

#endif	/* GETTER_H */

