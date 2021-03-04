#include "mainwindow.h"
#include "../Forms/ui_mainwindow.h"

const std::vector<std::string> classes = {"Gul 2X2", "Rød 2X2", "Grøn 2X2", "Rød 2X4", "Gul 2X3" };


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

    // Add IK Simulator view
    view = std::make_shared<Qt3DExtras::Qt3DWindow>();
    view->defaultFrameGraph()->setClearColor(Qt::gray);
    viewContainer = QWidget::createWindowContainer(view.get());

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setMargin(0);
    hLayout->addWidget(viewContainer);
    ui->IKTab->setLayout(hLayout);

    rootEntity = std::make_shared<Qt3DCore::QEntity>();

    // This sets up the entire scene graph
    simulator = std::make_unique<IKSimulator>(view, rootEntity);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_draw_rectangles_clicked()
{
    if(simulator != nullptr) {
        simulator->SortPiece();
    }
}

void MainWindow::on_load_image_clicked()
{
    ui->console->append("> Opening stream with URL: " + ui->uri->text());
    RecreateVideoPlayer(ui->uri->text());
}

void MainWindow::on_load_model_clicked()
{
    ui->console->append("> Loading neural model");
    detector = std::make_unique<ObjectDetector>("../Assets/brikker.tflite", classes);
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
    videoSurface.reset();
}

void MainWindow::RecreateVideoPlayer(QString url) {
    if(videoSurface == nullptr)
        videoSurface = std::make_unique<VideoPlayer>(ui->video, url);
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
