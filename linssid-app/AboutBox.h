/* 
 * File:   aboutBox.h
 * Author: warren
 *
 * Created on November 20, 2012, 8:12 PM
 */

#ifndef _ABOUTBOX_H
#define	_ABOUTBOX_H

#include "ui_AboutBox.h"

class AboutBox : public QDialog {
    Q_OBJECT
public:
    AboutBox();
    virtual ~AboutBox();
private:
    Ui::aboutBox widget;
};

#endif	/* _ABOUTBOX_H */
