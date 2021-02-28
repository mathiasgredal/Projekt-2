#include "../Headers/objectdetector.h"

#include <iostream>

ObjectDetector::ObjectDetector(std::string path, std::vector<std::string> classLabels)
{
    model = tflite::FlatBufferModel::BuildFromFile(path.c_str());

    if(model == nullptr)
        throw std::runtime_error("ERROR: Could not load model(" + path + ")");

    tflite::InterpreterBuilder builder = tflite::InterpreterBuilder(*model, resolver);
    builder(&interpreter);

    if(interpreter == nullptr)
        throw std::runtime_error("ERROR: Could not create an interpreter instance");

    interpreter->SetNumThreads(4);

    // Setup input
    if(interpreter->input_tensor(0)->dims->size != 4)
        throw std::runtime_error("ERROR: Model has incorrect input shape");

    imageWidth  = interpreter->input_tensor(0)->dims[2].size;
    imageHeight = interpreter->input_tensor(0)->dims[3].size;
    imageChannels = interpreter->input_tensor(0)->dims[4].size;
    if(imageChannels != 3)
        throw std::runtime_error("ERROR: Model has unsupported number of color channels");

    // Setup output
    if(interpreter->output_tensor(0)->dims->size != 3)
        throw std::runtime_error("ERROR: Model has incorrect output shape");

    // The number of detected boxes produced, for yolov5s it is 6300
    numBoxes = interpreter->output_tensor(0)->dims[2].size;

    // This seems undocumented, so here is how a coloumn in the output for yolov5 is laid out.
    // 0: x-position of the center of the box
    // 1: y-position of the center of the box
    // 2: width of the box
    // 3: height of the box
    // 4: confidence in the box
    // 5+: Confidence in each label for the box, eg. classification
    dimensions = interpreter->output_tensor(0)->dims[3].size;

    labels = classLabels;
    if(static_cast<size_t>(dimensions-5) != labels.size())
        throw std::runtime_error("ERROR: Mismatch between number of model labels and number of provided labels");

    // Allocate tensor buffers.
    if(interpreter->AllocateTensors() != kTfLiteOk)
        throw std::runtime_error("ERROR: Could not allocate tensorbuffers. Are we out of memory?");
    // tflite::PrintInterpreterState(interpreter.get()); // Use this for getting detailed info about model
}

void ObjectDetector::Predict(QImage _image)
{
    if(t1.joinable())
        t1.join();

    t1 = std::thread([=]() {

        std::vector<float> inputval = std::vector<float>();

        // Make the right format for neural network
        QImage scaledImage = _image.scaled(imageWidth, imageHeight, Qt::AspectRatioMode::KeepAspectRatio, Qt::SmoothTransformation);

        QPixmap squareImage(imageWidth, imageHeight);
        squareImage.fill(Qt::black);
        QPainter painter = QPainter(&squareImage);
        painter.drawImage(QPoint(0, 0), scaledImage);
        painter.end();

        QImage image = squareImage.toImage();
        image.convertTo(QImage::Format::Format_RGB32);

        for(int64_t y = 0; y < image.height(); y++) {
            for(int64_t x = 0; x < image.width(); x++) {
                QRgb pixel = image.pixel(x, y);
                inputval.push_back(((qRed(pixel) / 255.0)-0.5)*2.0);
                inputval.push_back(((qGreen(pixel) / 255.0)-0.5)*2.0);
                inputval.push_back(((qBlue(pixel) / 255.0)-0.5)*2.0);
            }
        }

        // Get input & output layer
        float* inputLayer = interpreter->typed_input_tensor<float>(0);

        // The output is split into 6300 rows. Each row i 85 long. first 4 is
        float* outputLayer = interpreter->typed_output_tensor<float>(0);

        // flatten rgb image to input layer.
        float* inputImg_ptr = inputval.data();
        memcpy(inputLayer, inputImg_ptr, imageWidth * imageHeight * imageChannels * sizeof(float));

        // Run inference
        if(interpreter->Invoke() != kTfLiteOk)
            throw std::runtime_error("ERROR: Could not run inference");

        // Results, these should be replaced with a combined class/struct
        std::vector<Prediction> predictions = {};

        for (int i = 0; i < numBoxes; i++) {
            int index = i * dimensions;
            // Skip detection on low confidence, tveak this value for more detections
            if(outputLayer[index+confidenceIndex] <= Confidence_thresshold) continue;
            Prediction pred = {};
            pred.confidence = outputLayer[index+confidenceIndex];

            // Tveak label confidence based on overall confidence
            for (int j = labelStartIndex; j < dimensions; j++) {
                outputLayer[index+j] = outputLayer[index+j] * outputLayer[index+confidenceIndex];
                pred.labelScores.push_back(outputLayer[index+j]);
            }

            // Create bounding box from first 4 items in row
            const float topLeftX =     (outputLayer[index] - outputLayer[index+2] / 2)*imageWidth;
            const float topLeftY =     (outputLayer[index + 1] - outputLayer[index+3] / 2)*imageHeight;
            const float bottomRightX = (outputLayer[index] + outputLayer[index+2] / 2)*imageWidth;
            const float bottomRightY = (outputLayer[index + 1] + outputLayer[index+3] / 2)*imageHeight;
            const QRectF bbox = QRectF(QPointF(topLeftX, topLeftY), QPointF(bottomRightX, bottomRightY));

            pred.bbox = bbox;
            predictions.push_back(pred);
        }

        std::vector<Prediction> filtered_predictions = nms(predictions, IOU_thresshold);

        // Add labels
        for(auto& pred : filtered_predictions) {
            size_t index = std::max_element(pred.labelScores.begin(), pred.labelScores.end()) - pred.labelScores.begin();
            pred.label = labels[index];
        }
        emit onFinishPrediction(filtered_predictions);
        return;
    });
    return;
}

QSize ObjectDetector::GetInputSize()
{
    return QSize(imageWidth, imageHeight);
}

constexpr float area(const QRectF& rect) {
    return rect.width()*rect.height();
}

// Simple NMS implementation
std::vector<Prediction> nms(const std::vector<Prediction> &pred, float iou_thresh) {
    if (pred.size() == 0) return {};

    std::vector<Prediction> filtered_pred = {};

    // Sort the bounding boxes by the bottom - right y - coordinate of the bounding box
    std::multimap<int, size_t> idxs; // multimap does autosorting as you add items
    for (size_t i = 0; i < pred.size(); ++i){
        idxs.insert(std::pair<int, size_t>(pred[i].bbox.bottomRight().y(), i));
    }

    // keep looping while some indexes still remain in the indexes list
    while (idxs.size() > 0){
        // grab the last rectangle
        auto lastElem = --std::end(idxs);
        const Prediction& last = pred[lastElem->second];
        filtered_pred.push_back(last);
        idxs.erase(lastElem);
        for (auto pos = std::begin(idxs); pos != std::end(idxs); ){
            // grab the current rectangle
            const Prediction& current = pred[pos->second];
            float intArea = area(current.bbox & last.bbox);
            float unionArea = area(last.bbox) + area(current.bbox) - intArea;
            float overlap = intArea/unionArea;

            // if there is sufficient overlap, suppress the current bounding box
            if (overlap > iou_thresh) pos = idxs.erase(pos);
            else ++pos;
        }
    }

    return filtered_pred;
}
