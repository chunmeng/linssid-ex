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
    connect(widget.checkBoxSSID, SIGNAL(stateChanged(int)), this, SLOT(ssidChanged(int)));
    connect(widget.checkBoxMAC, &QCheckBox::stateChanged, [this](int state) {
                bool enabled = (state == Qt::CheckState::Checked) ? true : false;
                options_.byMac = enabled;
                widget.lineEditMAC->setEnabled(enabled);
                emit filterUpdated(options_);
            });
    // lineEditChannel returnPressed() and editingFinished()
    connect(widget.lineEditChannel, SIGNAL(editingFinished()), this, SLOT(channelTextChanged()));
    connect(widget.lineEditSSID, SIGNAL(editingFinished()), this, SLOT(ssidTextChanged()));
    connect(widget.lineEditMAC, &QLineEdit::editingFinished, [this]() {
                if (options_.mac == widget.lineEditMAC->text().toStdString())
                    return;
                DebugLog(AppLogger) << "Filter mac changed: " << widget.lineEditMAC->text().toStdString();
                options_.mac = widget.lineEditMAC->text().toStdString();
                emit filterUpdated(options_);
            });
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
    widget.checkBoxSSID->setChecked(opt.bySsid);
    widget.checkBoxMAC->setChecked(opt.byMac);
    // @FIXME: Does not behave the same as regex101 and still accept 6d number. Maybe this is over engineering, and a simple [0-9,-] would do... 
    // https://regex101.com/r/KwvbjH/3
    // v4 - ^([0-9]{1,3})([,-]+[0-9]{1,3})*([0-9]{1,3}[,])?$
    // v3 - ^([0-9]{1,3}){1}([,-]+[0-9]{1,3})*([0-9]{1,3}[,])?$
    widget.lineEditChannel->setValidator(new QRegExpValidator(QRegExp("^([0-9]{1,3})([,-]+[0-9]{1,3})*([0-9]{1,3}[,])?$"), widget.lineEditChannel));
    widget.lineEditMAC->setValidator(new QRegExpValidator(QRegExp("^([0-9A-Fa-f?]{2}[:-]){5}([0-9A-Fa-f?]{2})$"), widget.lineEditMAC));
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

void ViewFilterDialog::ssidChanged(int state)
{
    if (state == Qt::CheckState::Checked) {
        options_.bySsid = true;
        widget.lineEditSSID->setEnabled(true);
    } else {
        options_.bySsid = false;
        widget.lineEditSSID->setEnabled(false);
    }
    emit filterUpdated(options_);
}

void ViewFilterDialog::channelTextChanged()
{
    if (options_.channels == widget.lineEditChannel->text().toStdString())
        return;
    DebugLog(AppLogger) << "Filter channels changed: " << widget.lineEditChannel->text().toStdString();
    options_.channels = widget.lineEditChannel->text().toStdString();
    emit filterUpdated(options_);
}

void ViewFilterDialog::ssidTextChanged()
{
    if (options_.ssid == widget.lineEditSSID->text().toStdString())
        return;
    DebugLog(AppLogger) << "Filter ssid changed: " << widget.lineEditSSID->text().toStdString();
    options_.ssid = widget.lineEditSSID->text().toStdString();
    emit filterUpdated(options_);
}