#include "mainwindow.h"
#include "../Forms/ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set sidebar size
    ui->centerSplit->setStretchFactor(0, 2);
    ui->centerSplit->setStretchFactor(1, 20);
    ui->sidesplit->setStretchFactor(1, 10);
    ui->mainView->setStretchFactor(1,1);
    ui->mainView->setStretchFactor(0,1);


    ui->console->append("> Starting Project 2 UI");

    for(auto port : QSerialPortInfo::availablePorts()) {
        ui->port->addItem(port.portName());
    }

    for(auto baud : QSerialPortInfo::standardBaudRates()) {
        ui->baud->addItem(QString::number(baud));
    }



//    videoSurface = new VideoPlayer(ui->video, "rtsp://2.130.136.12:8554/live.sdp");

//    http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4
}

MainWindow::~MainWindow()
{
    delete videoSurface;
    delete ui;
}

void MainWindow::on_draw_rectangles_clicked()
{
    delete videoSurface;
    videoSurface = nullptr;
}

void MainWindow::on_load_image_clicked()
{
    ui->console->append("> Load image");
}

void MainWindow::on_load_model_clicked()
{
    videoSurface->pause = !videoSurface->pause;
}

void MainWindow::on_scan_image_clicked()
{
    if(videoSurface == nullptr)
        videoSurface = new VideoPlayer(ui->video, "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4");
    else
        std::cout << "Decoder running" << std::endl;
}
