#ifndef PREDICTIONVISUALIZER_H
#define PREDICTIONVISUALIZER_H

#include <iostream>
#include <fmt/core.h>

#include <QWidget>
#include <QPainter>
#include <QTextItem>

#include "../Headers/objectdetector.h"

class PredictionVisualizer : public QWidget
{
    Q_OBJECT
public:
    PredictionVisualizer(QWidget *parent = nullptr);
    void VisualizePrediction(std::vector<Prediction> _predictions, QImage img, QSize targetSize);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QImage backgroundImage;
    std::vector<Prediction> predictions = {};
    QSize targetSize;
};

#endif // PREDICTIONVISUALIZER_H
