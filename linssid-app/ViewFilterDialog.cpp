/*
 * File:   ViewFilterDialog.cpp
 * Author: chunmeng
 */

#include "ViewFilterDialog.h"
#include "CustomEvent.h"
#include "Logger.h"
#include <QRegExpValidator>

using namespace std;
extern Logger AppLogger;

ViewFilterDialog::ViewFilterDialog(QWidget *parent, QObject *filterProxy, const FilterState& options)
    : QWidget(parent)
    , options_(options)
{
    widget.setupUi(this);
    // Initialize to default filter condition
    initUiStates(options_);
    // connect all signals and slots locally
    connect(widget.groupBoxBand, SIGNAL(toggled(bool)), this, SLOT(bandGroupChanged(bool)));
    connect(widget.checkBox5G, SIGNAL(stateChanged(int)), this, SLOT(bandChanged(int)));
    connect(widget.checkBox24G, SIGNAL(stateChanged(int)), this, SLOT(bandChanged(int)));
    connect(widget.checkBoxChannel, &QCheckBox::stateChanged, [this](int state) {
                checkboxChanged((state == Qt::CheckState::Checked), options_.byChannel, *widget.lineEditChannel);
            });
    connect(widget.checkBoxSSID, &QCheckBox::stateChanged, [this](int state) {
                checkboxChanged((state == Qt::CheckState::Checked), options_.bySsid, *widget.lineEditSSID);
            });
    connect(widget.checkBoxMAC, &QCheckBox::stateChanged, [this](int state) {
                checkboxChanged((state == Qt::CheckState::Checked), options_.byMac, *widget.lineEditMAC);
            });
    // QLineEdit emit returnPressed() and editingFinished() only when text match the validator
    connect(widget.lineEditChannel, &QLineEdit::editingFinished, [this]() {
                textChanged("channel", options_.channels, widget.lineEditChannel->text().toStdString());
            });
    connect(widget.lineEditSSID, &QLineEdit::editingFinished, [this]() {
                textChanged("ssid", options_.ssid, widget.lineEditSSID->text().toStdString());
            });
    connect(widget.lineEditMAC, &QLineEdit::editingFinished, [this]() {
                textChanged("mac", options_.mac, widget.lineEditMAC->text().toStdString());
            });
    // Emit updated filter options back to filter model
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
    widget.lineEditSSID->setEnabled(opt.bySsid);
    widget.lineEditSSID->setText(QString::fromStdString(opt.ssid));
    widget.checkBoxMAC->setChecked(opt.byMac);
    widget.lineEditMAC->setEnabled(opt.byMac);
    widget.lineEditMAC->setText(QString::fromStdString(opt.mac));
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

void ViewFilterDialog::checkboxChanged(bool enabled, bool& filterState, QLineEdit& bindedLineEdit)
{
    filterState = enabled;
    bindedLineEdit.setEnabled(enabled);
    emit filterUpdated(options_);
}

void ViewFilterDialog::textChanged(const string& tag, string& filterText, const string& lineEditText)
{
    if (filterText == lineEditText)
        return;
    DebugLog(AppLogger) << tag << " filter changed: " << lineEditText;
    filterText = lineEditText;
    emit filterUpdated(options_);
}

void ViewFilterDialog::closeEvent(QCloseEvent *e)
{
    (void)e;
    if (this->parent()) {// parent can be null
        DebugLog(AppLogger) << "ViewFilterDialog closed, tell parent";
        QApplication::postEvent(this->parent(), new DialogClosedEvent(FILTER_DIALOG_ID));
    }
}