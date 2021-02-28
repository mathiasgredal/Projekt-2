#include "mainwindow.h"
#include "../Forms/ui_mainwindow.h"

const std::vector<std::string> classes = {"person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
                                             "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
                                             "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
                                             "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
                                             "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
                                             "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
                                             "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
                                             "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
                                             "hair drier", "toothbrush" };


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
}

MainWindow::~MainWindow()
{
    delete videoSurface;
    delete ui;
}

void MainWindow::on_draw_rectangles_clicked()
{

}

void MainWindow::on_load_image_clicked()
{

}

void MainWindow::on_load_model_clicked()
{
    ui->console->append("> Loading neural model");
    detector = std::make_unique<ObjectDetector>("../Assets/yolov5s-fp16.tflite", classes);
}

// NEURAL NETWORK STUFF
void MainWindow::on_scan_image_clicked()
{
    ui->console->append("> Fetching frame from video");
    if(videoSurface == nullptr) {
        ui->console->append("> No active videostream!");
        return;
    }

    QImage img = videoSurface->getCurrentImage().copy();

    QElapsedTimer timer;
    timer.start();

    ui->console->append("> Making prediction");
    update();
    if(detector == nullptr) {
        ui->console->append("> Neural model not loaded!");
        update();
        return;
    }

    // The prediction gets run in another thread
    detector->Predict(img);

    // This is the callback for the other thread, and it is run in the main thread to allow ui updates
    QObject::connect(detector.get(), &ObjectDetector::onFinishPrediction, this, [=]( std::vector<Prediction> predictions) {
        ui->prediction->VisualizePrediction(predictions, img, detector->GetInputSize());
        ui->console->append(fmt::format("> Done in {:.{}f} seconds", timer.elapsed()/1000.f, 2).c_str());

        // This disconnects the lambda function after it is called
        QObject::disconnect(detector.get(), &ObjectDetector::onFinishPrediction, this, nullptr);
    }, Qt::BlockingQueuedConnection);
}

// VIDEO STUFF
void MainWindow::StartVideoPlayer() {
    if(videoSurface != nullptr)
        videoSurface->pause = false;
}

void MainWindow::PauseVideoPlayer() {
    if(videoSurface != nullptr)
        videoSurface->pause = true;
}

void MainWindow::DeleteVideoPlayer() {
    delete videoSurface;
    videoSurface = nullptr;
}

void MainWindow::RecreateVideoPlayer(QString url) {
    if(videoSurface == nullptr)
        videoSurface = new VideoPlayer(ui->video, url);
    else {
        DeleteVideoPlayer();
        RecreateVideoPlayer(url);
    }
}

// This is called when the settings have been applied
void MainWindow::on_applySettings_clicked()
{
    ui->console->append("> Applying settings");
    // Check for changes in url
    if(videoSurface == nullptr || videoSurface->getUrl() != ui->uri->text()) {
        ui->console->append("> Video URL changed! Recreating videoplayer...");
        RecreateVideoPlayer(ui->uri->text());
    }
}
