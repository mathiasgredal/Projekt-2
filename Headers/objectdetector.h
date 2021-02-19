#ifndef OBJECTDETECTOR_H
#define OBJECTDETECTOR_H

#include <memory>
#include <string>
#include <vector>
#include <map>

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/optional_debug_tools.h>

#include <QImage>
#include <QRect>

struct Prediction {
    QRectF bbox;
    float confidence;
    std::vector<float> labelScores;
    std::string label = "undefined";
};


class ObjectDetector
{
public:
    ObjectDetector(std::string path, std::vector<std::string> classLabels);

    std::vector<Prediction> Predict(QImage image);
private:
    // TFLite data
    std::unique_ptr<tflite::FlatBufferModel> model;
    tflite::ops::builtin::BuiltinOpResolver resolver;
    std::unique_ptr<tflite::Interpreter> interpreter;

    // Model IO data
    int imageWidth = 0;
    int imageHeight = 0;
    int imageChannels = 0; // Color channels, typically 3(RGB)

    int numBoxes = 0;
    int dimensions = 0;

    const int confidenceIndex = 4;
    const int labelStartIndex = 5;

    const float IOU_thresshold = 0.4f;
    const float Confidence_thresshold = 0.2f;

    std::vector<std::string> labels;
};

constexpr float area(const QRectF& rect);

// TODO: This is a naive greedy implementation, consider mean box merging
std::vector<Prediction> nms(const std::vector<Prediction> &pred, float iou_thresh);

#endif // OBJECTDETECTOR_H
