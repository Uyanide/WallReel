/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-07 00:32:25
 * @LastEditTime: 2026-01-15 05:23:55
 * @Description: LoadingIndicator implementation.
 */
#include "loading_indicator.h"

LoadingIndicator::LoadingIndicator(int barMinimumWidth, QWidget* parent)
    : QWidget(parent),
      ui(new Ui::LoadingIndicator) {
    ui->setupUi(this);
    ui->progressBar->setMinimumWidth(barMinimumWidth);
}

LoadingIndicator::~LoadingIndicator() {
    delete ui;
}
