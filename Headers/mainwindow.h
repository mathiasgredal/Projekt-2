#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <optional>

#include <QMainWindow>
#include <QElapsedTimer>

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QHBoxLayout>

#include <Qt3DExtras>
#include <Qt3DRender>

#include <fmt/core.h>

#include "videoplayer.h"
#include "objectdetector.h"
#include "predictionvisualizer.h"
#include "IKSimulator.h"

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
    std::unique_ptr<ObjectDetector> detector = nullptr;

    // Video Functions
    std::unique_ptr<VideoPlayer> videoSurface = nullptr;
    void RecreateVideoPlayer(QString url);
    void DeleteVideoPlayer();
    void PauseVideoPlayer();
    void StartVideoPlayer();

    // IK Simulator
    std::unique_ptr<IKSimulator> simulator = nullptr;
    std::shared_ptr<Qt3DExtras::Qt3DWindow> view;
    std::shared_ptr<Qt3DCore::QEntity> rootEntity;
    QWidget *viewContainer;
};
#endif // MAINWINDOW_H
