/*
 * File:   PrefsDialog.cpp
 * Author: warren
 *
 * Created on January 2, 2013, 2:40 PM
 */

#include <string>
#include <iostream>
#include "prefsDialog.h"
#include "MainForm.h"

struct sEntryValue {
    QString tblFntSize;
    int plotMin;
    int plotMax;
    bool showGrid;
    bool showLabel;
    bool logDataState;
} prefsDialog::entryValue;

prefsDialog::prefsDialog(QString tblFntSize, int plotMin, int plotMax, bool showGrid, bool showLabel, int logDataState, QObject* mommy) {
    widget.setupUi(this);
    prefsDialog::entryValue.tblFntSize = tblFntSize;
    prefsDialog::entryValue.plotMin = plotMin;
    prefsDialog::entryValue.plotMax = plotMax;
    prefsDialog::entryValue.showGrid = showGrid;
    prefsDialog::entryValue.showLabel = showLabel;
    prefsDialog::entryValue.logDataState = logDataState;
    // now set initial values of widgets in the prefs dialog
    for (int item = 0; item < MainForm::numFntSizes; item++) {
        widget.tblFntSizeSb->addItem(MainForm::fntSizes[item]);
        if (MainForm::fntSizes[item] == tblFntSize) {
            widget.tblFntSizeSb->setCurrentIndex(item);
        }
    }
    // widget.tblFntSizeSb->setCurrentIndex(widget.tblFntSizeSb->findData(tblFntSize));
    widget.dbMinsb->setValue(plotMin);
    widget.dbMaxsb->setValue(plotMax);
    widget.plotGridcbx->setChecked(showGrid);
    widget.showLabelcbx->setChecked(showLabel);
    widget.logDatacbx->setCheckState((Qt::CheckState) logDataState);
    widget.dbMinsb->setMinimum(-100);
    widget.dbMaxsb->setMaximum(0);
    widget.dbMinsb->setMaximum(widget.dbMaxsb->value() - 10);
    widget.dbMaxsb->setMinimum(widget.dbMinsb->value() + 10);
    // disable user text editing the spin boxes. Yeah, this is the only way.
    widget.dbMinsb->findChild<QLineEdit*>()->setReadOnly(true);
    widget.dbMaxsb->findChild<QLineEdit*>()->setReadOnly(true);
    connect(prefsDialog::widget.tblFntSizeSb, SIGNAL(currentTextChanged(QString)), this, SLOT(tblFntSizeSbChanged(QString)));
    connect(prefsDialog::widget.dbMinsb, SIGNAL(valueChanged(int)), this, SLOT(minSbChanged(int)));
    connect(prefsDialog::widget.dbMaxsb, SIGNAL(valueChanged(int)), this, SLOT(maxSbChanged(int)));
    connect(prefsDialog::widget.plotGridcbx, SIGNAL(stateChanged(int)), this, SLOT(gridChanged(int)));
    connect(prefsDialog::widget.showLabelcbx, SIGNAL(stateChanged(int)), this, SLOT(showLabelChanged(int)));
    connect(prefsDialog::widget.logDatacbx, SIGNAL(stateChanged(int)), mommy, SLOT(logPrefChanged(int)));
    connect(this, SIGNAL(plotPrefsChanged(QString, int, int, bool, bool)),
            mommy, SLOT(updatePlotPrefs(QString, int, int, bool, bool)));
    connect(this, SIGNAL(finished(int)), this, SLOT(bailOut(int)));
}

prefsDialog::~prefsDialog() {
}

// the emit values in this order:
// QString fontsize, int minplot, int maxplot, bool plotgrid

void prefsDialog::tblFntSizeSbChanged(QString newValue) {
    emit plotPrefsChanged(newValue,
        // plotPrefsChanged(widget.tblFntSizeSb->currentText(),
        widget.dbMinsb->value(),
        widget.dbMaxsb->value(),
        widget.plotGridcbx->isChecked(),
        widget.showLabelcbx->isChecked());
}

void prefsDialog::minSbChanged(int newValue) {

    emit plotPrefsChanged(widget.tblFntSizeSb->currentText(),
        newValue,
        widget.dbMaxsb->value(),
        widget.plotGridcbx->isChecked(),
        widget.showLabelcbx->isChecked());
    widget.dbMaxsb->setMinimum(newValue + 10);
}

void prefsDialog::maxSbChanged(int newValue) {

    emit plotPrefsChanged(widget.tblFntSizeSb->currentText(),
        widget.dbMinsb->value(),
        newValue,
        widget.plotGridcbx->isChecked(),
        widget.showLabelcbx->isChecked());
    widget.dbMinsb->setMaximum(newValue - 10);
}

void prefsDialog::gridChanged(int newValue) {

    emit plotPrefsChanged(widget.tblFntSizeSb->currentText(),
        widget.dbMinsb->value(),
        widget.dbMaxsb->value(),
        newValue,
        widget.showLabelcbx->isChecked());
}

void prefsDialog::showLabelChanged(int newValue) {

    emit plotPrefsChanged(widget.tblFntSizeSb->currentText(),
        widget.dbMinsb->value(),
        widget.dbMaxsb->value(),
        widget.plotGridcbx->isChecked(),
        newValue);
}

void prefsDialog::bailOut(int result) {
    if (result == 0)
        emit plotPrefsChanged(entryValue.tblFntSize, entryValue.plotMin, entryValue.plotMax, entryValue.showGrid, entryValue.showLabel);
}
