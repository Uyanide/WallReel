/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-07 00:32:25
 * @LastEditTime: 2025-08-07 21:14:09
 * @Description: LoadingIndicator implementation.
 */
#include "loading_indicator.h"

LoadingIndicator::LoadingIndicator(QWidget* parent) : QWidget(parent),
                                                      ui(new Ui::LoadingIndicator) {
    ui->setupUi(this);
}

LoadingIndicator::~LoadingIndicator() {
    delete ui;
}
