#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QSerialPort>
#include <QSerialPortInfo>

#include "../Headers/videoplayer.h"
#include <QMediaPlayer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_draw_rectangles_clicked();
    void on_load_image_clicked();
    void on_load_model_clicked();
    void on_scan_image_clicked();

private:
    Ui::MainWindow *ui;
    QMediaPlayer* player;
    VideoPlayer* videoSurface = nullptr;
//    VideoView* video;
};
#endif // MAINWINDOW_H
