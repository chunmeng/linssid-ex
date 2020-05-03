/*
 * File:   ViewFilterDialog.cpp
 * Author: chunmeng
 */

#include "ViewFilterDialog.h"
#include "Logger.h"

using namespace std;
extern Logger AppLogger;

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
    connect(widget.checkBoxChannel, SIGNAL(stateChanged(int)), this, SLOT(channelChanged(int)));
    // lineEditChannel returnPressed() and editingFinished()
    connect(widget.lineEditChannel, SIGNAL(editingFinished()), this, SLOT(channelTextChanged()));
    // connect to filter model
    connect(this, SIGNAL(filterUpdated(FilterState)), filterProxy, SLOT(setFilter(FilterState)));
}

ViewFilterDialog::~ViewFilterDialog() = default;

void ViewFilterDialog::initUiStates(const FilterState& opt)
{
    widget.groupBoxBand->setChecked(opt.byBand);
    widget.checkBox5G->setChecked(opt.showBand5G);
    widget.checkBox24G->setChecked(opt.showBand24G);
    widget.checkBoxChannel->setChecked(opt.byChannel);
    widget.lineEditChannel->setEnabled(opt.byChannel);
    widget.lineEditChannel->setText(QString::fromStdString(opt.channels));
    // @FIXME: Does not behave the same as regex101 and still accept 6d number. Maybe this is over engineering, and a simple [0-9,-] would do... 
    // https://regex101.com/r/KwvbjH/3
    // v4 - ^([0-9]{1,3})([,-]+[0-9]{1,3})*([0-9]{1,3}[,])?$
    // v3 - ^([0-9]{1,3}){1}([,-]+[0-9]{1,3})*([0-9]{1,3}[,])?$
    widget.lineEditChannel->setValidator(new QRegExpValidator(QRegExp("^([0-9]{1,3})([,-]+[0-9]{1,3})*([0-9]{1,3}[,])?$"), widget.lineEditChannel));
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

void ViewFilterDialog::channelChanged(int state)
{
    if (state == Qt::CheckState::Checked) {
        options_.byChannel = true;
        widget.lineEditChannel->setEnabled(true);
    } else {
        options_.byChannel = false;
        widget.lineEditChannel->setEnabled(false);
    }
    emit filterUpdated(options_);
}

void ViewFilterDialog::channelTextChanged()
{
    DebugLog(AppLogger) << "Channels entered: " << widget.lineEditChannel->text().toStdString();
    if (options_.channels == widget.lineEditChannel->text().toStdString())
        return;
    options_.channels = widget.lineEditChannel->text().toStdString();
    emit filterUpdated(options_);
}