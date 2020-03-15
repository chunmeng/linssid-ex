/*
 * File:   aboutBox.cpp
 * Author: warren
 *
 * Created on November 20, 2012, 8:12 PM
 */

#include <string>
#include "AboutBox.h"
#include "Custom.h"
using namespace std;

AboutBox::AboutBox() {
    widget.setupUi(this);
    std::string version = std::string("Version ") + std::string(LINSSIDVERSION);
    widget.aboutVersion->setText(version.c_str());
}

AboutBox::~AboutBox() {
}
