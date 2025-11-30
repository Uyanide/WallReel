/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-05 00:37:58
 * @LastEditTime: 2025-12-01 00:37:35
 * @Description: MainWindow implementation.
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "config.h"
#include "images_carousel.h"
#include "loading_indicator.h"

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(const Config& config, QWidget* parent = nullptr);
    ~MainWindow();

  public slots:
    void onConfirm();

  protected:
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

  private:
    void _setupUI();
    void _stopLoadingAndQuit(const std::function<void()>& onStopped = nullptr);

  private slots:
    void _onImageFocused(const QString& path, const int index, const int count);
    void _onLoadingStarted(const qsizetype amount);
    void _onLoadingCompleted(const qsizetype amount);

    void _onCancelPressed();
    void _onConfirmPressed();

  private:
    enum _State {
        Init,
        Loading,
        Stopping,
        Ready,
    } m_state = Init;

    Ui::MainWindow* ui;
    ImagesCarousel* m_carousel           = nullptr;
    LoadingIndicator* m_loadingIndicator = nullptr;
    int m_carouselIndex, m_loadingIndicatorIndex;
    const Config& m_config;

  signals:
    void stop();
};
#endif  // MAINWINDOW_H
