/*
 * File:   ViewFilterDialog.cpp
 * Author: chunmeng
 */

#include "ViewFilterDialog.h"

using namespace std;

ViewFilterDialog::ViewFilterDialog(QWidget *parent, QObject *filterProxy)
    : QWidget(parent)
{
    widget.setupUi(this);
    connect(widget.checkBox5G, SIGNAL(stateChanged(int)), this, SLOT(bandChanged(int)));
    connect(widget.checkBox24G, SIGNAL(stateChanged(int)), this, SLOT(bandChanged(int)));
    connect(this, SIGNAL(bandChanged(bool, bool)), filterProxy, SLOT(setBand(bool, bool)));
}

ViewFilterDialog::~ViewFilterDialog() = default;

void ViewFilterDialog::bandChanged(int dontCare)
{
    (void)dontCare;
    emit bandChanged(widget.checkBox5G->isChecked(), widget.checkBox24G->isChecked());
}