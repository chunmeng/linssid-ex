/* 
 * File:   Preferences.h
 * Author: warren
 *
 * Created on January 2, 2013, 2:40 PM
 */

#ifndef _PREFERENCES_H
#define	_PREFERENCES_H

#include <iostream>
#include <QtWidgets>
#include "ui_prefsDialog.h"
#include "Custom.h"

struct sEntryValue;

class prefsDialog : public QDialog {
    Q_OBJECT
public:
    // int pointsize, int maxplotscale, int min plot scale, 
    prefsDialog(QString, int, int, bool, bool, int, QObject*);
    virtual ~prefsDialog();
    static sEntryValue entryValue;
public slots:
    //    void returnPrefsOK();
    void tblFntSizeSbChanged(QString);
    void minSbChanged(int);
    void maxSbChanged(int);
    void gridChanged(int);
    void showLabelChanged(int);
    void bailOut(int);
    //    void interceptClose(int);
signals:
    void plotPrefsChanged(QString, int, int, bool, bool);
private:
    Ui::prefsDialog widget;
};

#endif	/* _PREFERENCES_H */
