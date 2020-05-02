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
    ViewFilterDialog(QWidget *parent, QObject* filterProxy);
    virtual ~ViewFilterDialog();

public slots:
    void bandGroupChanged(bool);
    void bandChanged(int);

signals:
    void filterUpdated(const FilterState& opt);

private:
    Ui::ViewFilterDialog widget;
    FilterState options_;
};

#endif	/* _VIEWFILTERDIALOG_H */
