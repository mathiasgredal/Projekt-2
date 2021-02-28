#include "predictionvisualizer.h"

PredictionVisualizer::PredictionVisualizer(QWidget *parent)
{

}

void PredictionVisualizer::VisualizePrediction(std::vector<Prediction> _predictions, QImage img, QSize _targetSize)
{
    backgroundImage = img;
    predictions = _predictions;
    targetSize = _targetSize;
    this->update();
}

void PredictionVisualizer::mousePressEvent(QMouseEvent *event)
{
    std::cout << "Yeet" << std::endl;
    this->update();
}

void PredictionVisualizer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // Abort if no background image
    if (backgroundImage.isNull()){ return; }

    const QImage scaledImage = backgroundImage.scaled(QSize(targetSize.width(), targetSize.height()), Qt::KeepAspectRatio, Qt::FastTransformation);

    // Get the drawing area size
    const double widgetWidth = painter.device()->width();
    const double widgetHeight = painter.device()->height();

    // Scale the image to match the width or height of drawing area
    QPixmap tempQImage = QPixmap::fromImage(backgroundImage.scaled(QSize(widgetWidth, widgetHeight), Qt::KeepAspectRatio, Qt::FastTransformation));

    // Since the images are low resolution, we upscale it to make text look better
    const float imgScale = 4;
    tempQImage = tempQImage.scaled(tempQImage.width()*imgScale, tempQImage.width()*imgScale);

    // We draw the bounding boxes of the predicted objects on the image
    QPainter imgPainter = QPainter(&tempQImage);
    imgPainter.scale(tempQImage.width() / static_cast<float>(scaledImage.width()), tempQImage.height() / static_cast<float>(scaledImage.height()));
    imgPainter.setPen(Qt::red);

    // Reduce fontsize
    QFont font = imgPainter.font();
    font.setPointSize(font.pointSize() * 0.5);
    imgPainter.setFont(font);

    for(auto pred : predictions) {
        imgPainter.drawText(pred.bbox.topLeft() + QPointF(0, -2), (pred.label + " - " + fmt::format("{:.{}f}", pred.confidence, 2)).c_str());
        imgPainter.drawRect(pred.bbox);
    }

    imgPainter.end();

    // Finally we draw the image
    painter.drawPixmap(QRect(QPoint(widgetWidth/2-tempQImage.width()/(2*imgScale), 0), QPoint(widgetWidth/2+tempQImage.width()/(2*imgScale), widgetHeight)), tempQImage);
}
