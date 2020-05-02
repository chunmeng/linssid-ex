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
    // Initialize to default filter condition
    initUiStates(options_);
    // connect all signals and slots locally
    connect(widget.groupBoxBand, SIGNAL(toggled(bool)), this, SLOT(bandGroupChanged(bool)));
    connect(widget.checkBox5G, SIGNAL(stateChanged(int)), this, SLOT(bandChanged(int)));
    connect(widget.checkBox24G, SIGNAL(stateChanged(int)), this, SLOT(bandChanged(int)));
    // connect to filter model
    connect(this, SIGNAL(filterUpdated(FilterState)), filterProxy, SLOT(setFilter(FilterState)));
}

ViewFilterDialog::~ViewFilterDialog() = default;

void ViewFilterDialog::initUiStates(const FilterState& opt)
{
    widget.groupBoxBand->setChecked(opt.byBand);
    widget.checkBox5G->setChecked(opt.showBand5G);
    widget.checkBox24G->setChecked(opt.showBand24G);
}

void ViewFilterDialog::bandGroupChanged(bool dontCare)
{
    (void)dontCare;
    // Update all band filter params
    options_.byBand = widget.groupBoxBand->isChecked();
    emit filterUpdated(options_);
}

void ViewFilterDialog::bandChanged(int dontCare)
{
    (void)dontCare;
    options_.showBand5G = widget.checkBox5G->isChecked();
    options_.showBand24G = widget.checkBox24G->isChecked();
    emit filterUpdated(options_);
}