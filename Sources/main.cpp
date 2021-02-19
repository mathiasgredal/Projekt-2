#include "mainwindow.h"
//#include "objectdetector.h"

#include <iostream>

#include <QApplication>

#include <array>
#include <vector>


const std::vector<std::string> classes = {"person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
                                             "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
                                             "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
                                             "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
                                             "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
                                             "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
                                             "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
                                             "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
                                             "hair drier", "toothbrush" };

int main(int argc, char *argv[])
{
//    ObjectDetector detector = ObjectDetector( "../Assets/yolov5s-fp16.tflite", classes);

    // Load image
    QImage img("../Assets/bus.jpg");
    if(img.isNull())
        throw std::runtime_error("ERROR: Could not load image");

//    auto predictions = detector.Predict(img);

//    for(auto pred : predictions) {
//        std::cout << pred.label << "(" << pred.confidence << "): ";
//        std::cout << "TL(" << pred.bbox.topLeft().x() << "; " << pred.bbox.topLeft().y() << "), ";
//        std::cout << "BR(" << pred.bbox.bottomRight().x() << "; " << pred.bbox.bottomRight().y() << ")";
//        std::cout << std::endl;
//    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
