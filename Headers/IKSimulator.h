#ifndef IKSIMULATOR_H
#define IKSIMULATOR_H

#include <iostream>
#include <vector>

#include <QObject>

#include <Qt3DExtras>
#include <Qt3DRender>

class TargetPiece : public QObject
{
    Q_OBJECT
public:
    TargetPiece(QString name, QVector3D position, QVector3D size, QColor color, std::shared_ptr<Qt3DCore::QEntity> rootEntity);
    ~TargetPiece();

    Qt3DCore::QEntity *getTargetEntity() const;

private:
    std::shared_ptr<Qt3DCore::QEntity> rootEntity;
    Qt3DCore::QEntity *targetEntity;
    Qt3DExtras::QCuboidMesh *targetMesh;
    Qt3DCore::QTransform *targetTransform;
    QString targetType;
};

class IKSimulator : public QObject
{
    Q_OBJECT
public:
    explicit IKSimulator(std::shared_ptr<Qt3DExtras::Qt3DWindow> _view, std::shared_ptr<Qt3DCore::QEntity> _rootEntity);
    ~IKSimulator();

    void setArmPosition(float _baseRotation, float _joint1Rotation, float _joint2Rotation);
    void IKSolve(QVector3D endPosition);

private:
    std::shared_ptr<Qt3DExtras::Qt3DWindow> view;
    std::shared_ptr<Qt3DCore::QEntity> rootEntity;

    Qt3DRender::QCamera *cameraEntity;
    Qt3DExtras::QFirstPersonCameraController *camController;

    Qt3DCore::QEntity *lightEntity;
    Qt3DRender::QPointLight *light;
    Qt3DCore::QTransform *lightTransform;

    Qt3DCore::QEntity *robotBase;
    Qt3DCore::QEntity *groundPlane;

    // Targets
    std::vector<std::unique_ptr<TargetPiece>> targetPieces = {};

    // Arm Info
    Qt3DCore::QEntity *segment1Entity;
    Qt3DExtras::QCuboidMesh *segment1Mesh;
    Qt3DCore::QTransform *segment1Transform;

    Qt3DCore::QEntity *segment2Entity;
    Qt3DExtras::QCuboidMesh *segment2Mesh;
    Qt3DCore::QTransform *segment2Transform;

    float segment1Length = 4;
    float segment2Length = 3;

    float baseRotation = -10.f;
    float joint1Rotation = 45.f;
    float joint2Rotation = 90.f;
};

#endif // IKSIMULATOR_H
