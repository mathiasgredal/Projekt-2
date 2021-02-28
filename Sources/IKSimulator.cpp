#include "IKSimulator.h"

IKSimulator::IKSimulator(std::shared_ptr<Qt3DExtras::Qt3DWindow> _view, std::shared_ptr<Qt3DCore::QEntity> _rootEntity)
{
    view = _view;
    rootEntity = _rootEntity;

    // Move this to IKSimulator
    cameraEntity = view->camera();

    cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0, 4.f, 8.0f));
    cameraEntity->setUpVector(QVector3D(0, 1, 0));
    cameraEntity->setViewCenter(QVector3D(0, 1, 0));

    lightEntity = new Qt3DCore::QEntity(rootEntity.get());
    light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(2);
    lightEntity->addComponent(light);
    lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(cameraEntity->position());
    lightEntity->addComponent(lightTransform);

    camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity.get());
    camController->setCamera(cameraEntity);

    view->setRootEntity(rootEntity.get());

    // Grundpladen
    Qt3DExtras::QPlaneMesh *planeMesh = new Qt3DExtras::QPlaneMesh();
    planeMesh->setWidth(10);
    planeMesh->setHeight(7);

    Qt3DCore::QTransform *planeTransform = new Qt3DCore::QTransform();
    planeTransform->setScale(1.0f);
    planeTransform->setTranslation(QVector3D(0.0f, 0.f, 0.0f));

    Qt3DRender::QTextureLoader *loader = new Qt3DRender::QTextureLoader(rootEntity.get());
    Qt3DExtras::QDiffuseMapMaterial *difmat = new Qt3DExtras::QDiffuseMapMaterial(rootEntity.get());
    loader->setSource(QUrl::fromLocalFile("../Assets/groundplane.png"));
    difmat->setDiffuse(loader);

    groundPlane = new Qt3DCore::QEntity(rootEntity.get());
    groundPlane->addComponent(planeMesh);
    groundPlane->addComponent(difmat);
    groundPlane->addComponent(planeTransform);

    // Robot base
    Qt3DExtras::QCylinderMesh *robotBaseMesh = new Qt3DExtras::QCylinderMesh();
    robotBaseMesh->setRadius(1);
    robotBaseMesh->setLength(0.5f);
    robotBaseMesh->setRings(100);
    robotBaseMesh->setSlices(20);

    Qt3DCore::QTransform *robotBaseTransform = new Qt3DCore::QTransform();
    robotBaseTransform->setTranslation(QVector3D(-5.0f, 0.25f, 0.0f));

    Qt3DExtras::QPhongMaterial *robotBaseMaterial = new Qt3DExtras::QPhongMaterial();
    robotBaseMaterial->setDiffuse(Qt::blue);

    robotBase = new Qt3DCore::QEntity(rootEntity.get());
    robotBase->addComponent(robotBaseMesh);
    robotBase->addComponent(robotBaseMaterial);
    robotBase->addComponent(robotBaseTransform);

    // Segment 1
    segment1Mesh = new Qt3DExtras::QCuboidMesh();
    segment1Mesh->setXExtent(0.25f);
    segment1Mesh->setZExtent(0.5f);
    segment1Mesh->setYExtent(segment1Length);

    segment1Transform = new Qt3DCore::QTransform();

    Qt3DExtras::QPhongMaterial *segment1Material = new Qt3DExtras::QPhongMaterial();
    segment1Material->setDiffuse(QColor(QRgb(0x00ff00)));

    segment1Entity = new Qt3DCore::QEntity(rootEntity.get());
    segment1Entity->addComponent(segment1Mesh);
    segment1Entity->addComponent(segment1Material);
    segment1Entity->addComponent(segment1Transform);

    // Segment 2
    segment2Mesh = new Qt3DExtras::QCuboidMesh();
    segment2Mesh->setXExtent(0.25f);
    segment2Mesh->setZExtent(0.5f);
    segment2Mesh->setYExtent(segment2Length);

    segment2Transform = new Qt3DCore::QTransform();

    Qt3DExtras::QPhongMaterial *segment2Material = new Qt3DExtras::QPhongMaterial();
    segment2Material->setDiffuse(QColor(QRgb(0xff0000)));

    segment2Entity = new Qt3DCore::QEntity(rootEntity.get());
    segment2Entity->addComponent(segment2Mesh);
    segment2Entity->addComponent(segment2Material);
    segment2Entity->addComponent(segment2Transform);

    setArmPosition(-10, 45, 90);

    // Targets
    targetPieces.push_back(std::make_unique<TargetPiece>("yeet", QVector3D(-1, 0, -0.5), QVector3D(0.5, 0.2, 0.4), Qt::green, rootEntity));

    IKSolve(QVector3D(-1, 0, -0.5));
}

IKSimulator::~IKSimulator()
{

}

void IKSimulator::IKSolve(QVector3D endPosition) {
    QVector3D baseToTarget = endPosition - QVector3D(-5.0f, 0.25f, 0.0f);
    float baseAngle = -qRadiansToDegrees(tan(baseToTarget.z()/baseToTarget.x()));

    // Segment 1 angle

    // This is effectivly our x-coord
    float x = sqrt(baseToTarget.z()*baseToTarget.z()+baseToTarget.x()*baseToTarget.x());
    // This is effectivly our y-coord
    float y = baseToTarget.y();
    float l1 = segment1Length;
    float l2 = segment2Length;

    float q2 = acos((x*x+y*y-l1*l1-l2*l2)/(2*l1*l2));
//    float q2 = 3.1415926 - acos((l1*l1+l2*l2-x*x-y*y)/(2*l1*l2));
    float q1 = atan(x/y)-atan((l2*sin(q2))/(l1+l2*cos(q2)));

    std::cout << "Joint 2 angle: " << qRadiansToDegrees(q1+q2)+180 << std::endl;

    setArmPosition(baseAngle, qRadiansToDegrees(q1) - 180, qRadiansToDegrees(q2));
}

void IKSimulator::setArmPosition(float _baseRotation, float _joint1Rotation, float _joint2Rotation)
{
    // Segment 1
    baseRotation = _baseRotation;
    joint1Rotation = _joint1Rotation;
    joint2Rotation = _joint2Rotation;

    auto segment1Matrix = QMatrix4x4();
    segment1Matrix.setToIdentity();

    segment1Matrix.translate(QVector3D(-5.0f, segment1Length*0.5+0.5, 0.0f));
    segment1Matrix.rotate(baseRotation, QVector3D(0.0f, 1.f, 0.0f)); // This is the base rotation

    segment1Matrix.translate(QVector3D(0.0f, -0.5*segment1Length, 0.0f));
    segment1Matrix.rotate(-joint1Rotation, QVector3D(0.0f, 0.f, 1.0f)); // This is arm rotation
    segment1Matrix.translate(QVector3D(0.0f, 0.5*segment1Length, 0.0f));
    segment1Transform->setMatrix(segment1Matrix);

    // Segment 2
    auto segment2Matrix = QMatrix4x4();
    segment2Matrix.setToIdentity();

    // Calculate the position of the end of segment 1
    float baseRad = qDegreesToRadians(-baseRotation);
    float joint1Rad = qDegreesToRadians(joint1Rotation);
    float baseHypotenus = cos(joint1Rad)*segment1Length;

    float segment1Mod = sin(joint1Rad)*segment1Length;   // Forsk y-akse
    float baseHos = cos(baseRad)*baseHypotenus; // Forskyd x-akse
    float baseMod = sin(baseRad)*baseHypotenus; // Forskyd z-akse

//    QVector3D offset = QVector3D(baseHos, segment1Mod, baseMod);
//    QVector3D offset = QVector3D(-5.f+4*cos(baseRad)*sin(joint1Rad), 0.5f+4*sin(baseRad), 4*cos(baseRad)*cos(joint1Rad));

    segment2Matrix.translate(QVector3D(-5.0f, segment2Length*0.5+0.5, 0.0f));

    segment2Matrix.rotate(baseRotation, QVector3D(0.0f, 1.f, 0.0f)); // This is the base rotation

    segment2Matrix.translate(QVector3D(0.0f, -0.5*segment2Length, 0.0f));
    segment2Matrix.rotate(-joint1Rotation, QVector3D(0.0f, 0.f, 1.0f)); // This is arm rotation
    segment2Matrix.translate(QVector3D(0.0f, 0.5*segment2Length, 0.0f));

    segment2Matrix.translate(QVector3D(0.0f, segment1Length, 0.0f));

    segment2Matrix.translate(QVector3D(0.0f, -0.5*segment2Length, 0.0f));
    segment2Matrix.rotate(-joint2Rotation, QVector3D(0.0f, 0.f, 1.0f)); // This is arm rotation
    segment2Matrix.translate(QVector3D(0.0f, 0.5*segment2Length, 0.0f));
    segment2Transform->setMatrix(segment2Matrix);
}


TargetPiece::TargetPiece(QString name, QVector3D position, QVector3D size, QColor color, std::shared_ptr<Qt3DCore::QEntity> rootEntity)
{
    targetType = name;
    targetMesh = new Qt3DExtras::QCuboidMesh();
    targetMesh->setXExtent(size.x());
    targetMesh->setYExtent(size.y());
    targetMesh->setZExtent(size.z());

    targetTransform = new Qt3DCore::QTransform();
    targetTransform->setTranslation(QVector3D(position.x(), position.y()+size.y()*0.5, position.z()));

    Qt3DExtras::QPhongMaterial *targetMaterial = new Qt3DExtras::QPhongMaterial();
    targetMaterial->setDiffuse(color);

    targetEntity = new Qt3DCore::QEntity(rootEntity.get());
    targetEntity->addComponent(targetMesh);
    targetEntity->addComponent(targetMaterial);
    targetEntity->addComponent(targetTransform);
}

TargetPiece::~TargetPiece()
{
    targetEntity->setEnabled(false);
}

Qt3DCore::QEntity *TargetPiece::getTargetEntity() const
{
    return targetEntity;
}
