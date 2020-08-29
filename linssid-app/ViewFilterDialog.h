/* 
 * File:   ViewFilterDialog.h
 * Author: chunmeng
 */

#ifndef _VIEWFILTERDIALOG_H
#define	_VIEWFILTERDIALOG_H

#include "ui_ViewFilterDialog.h"
#include "DataProxyModel.h"

class ViewFilterDialog : public QWidget {
    Q_OBJECT
public:
    ViewFilterDialog(QWidget *parent, QObject *filterProxy, const FilterState& options);
    virtual ~ViewFilterDialog();

public slots:
    void bandGroupChanged(bool);
    void bandChanged(int);

signals:
    void filterUpdated(const FilterState& opt);

private:
    void initUiStates(const FilterState& opt);
    void checkboxChanged(bool enabled, bool& filterState, QLineEdit& bindedLineEdit);
    void textChanged(const std::string& tag, std::string& filterText, const std::string& lineEditText);
    virtual void closeEvent(QCloseEvent *e) final;

private:
    Ui::ViewFilterDialog widget;
    FilterState options_;
};

#endif	/* _VIEWFILTERDIALOG_H */
