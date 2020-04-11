/* 
 * File:   ViewFilterDialog.h
 * Author: chunmeng
 */

#ifndef _VIEWFILTERDIALOG_H
#define	_VIEWFILTERDIALOG_H

#include "ui_ViewFilterDialog.h"

class ViewFilterDialog : public QDialog {
    Q_OBJECT
public:
    ViewFilterDialog(QObject* parent);
    virtual ~ViewFilterDialog();

public slots:
    void bandChanged(int);

signals:
    void bandChanged(bool, bool);
private:
    Ui::ViewFilterDialog widget;
};

#endif	/* _VIEWFILTERDIALOG_H */
