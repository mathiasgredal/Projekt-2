#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <optional>

#include <QMainWindow>
#include <QElapsedTimer>

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QMediaPlayer>

#include <fmt/core.h>

#include "../Headers/videoplayer.h"
#include "../Headers/objectdetector.h"
#include "../Headers/predictionvisualizer.h"

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

    void on_applySettings_clicked();

private:
    Ui::MainWindow *ui;
    QMediaPlayer* player;
    std::unique_ptr<ObjectDetector> detector = nullptr;

    // Video Functions
    VideoPlayer* videoSurface = nullptr;
    void RecreateVideoPlayer(QString url);
    void DeleteVideoPlayer();
    void PauseVideoPlayer();
    void StartVideoPlayer();
};
#endif // MAINWINDOW_H
