#include "TransformComponentView.h"
#include "ui_TransformComponentView.h"

TransformComponentView::TransformComponentView(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::TransformComponentView) {
   m_ui->setupUi(this);
   connect(m_ui->posX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TransformComponentView::updateValues);
   connect(m_ui->posY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TransformComponentView::updateValues);
   connect(m_ui->posZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TransformComponentView::updateValues);
   connect(m_ui->rotX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TransformComponentView::updateValues);
   connect(m_ui->rotY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TransformComponentView::updateValues);
   connect(m_ui->rotZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TransformComponentView::updateValues);
   connect(m_ui->scaleX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TransformComponentView::updateValues);
   connect(m_ui->scaleY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TransformComponentView::updateValues);
   connect(m_ui->scaleZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TransformComponentView::updateValues);
}

TransformComponentView::~TransformComponentView() {
   delete m_ui;
}

QWidget* TransformComponentView::asWidget() {
   return this;
}

void TransformComponentView::updateValues() {
   if (!m_obj) return;

   QVector3D position(
         m_ui->posX->value(),
         m_ui->posY->value(),
         m_ui->posZ->value());
   QVector3D rotation(
         m_ui->rotX->value(),
         m_ui->rotY->value(),
         m_ui->rotZ->value());
   QVector3D scale(
         m_ui->scaleX->value(),
         m_ui->scaleY->value(),
         m_ui->scaleZ->value());

   auto& transform = m_obj->getComponent<TransformComponent>();
   transform.position = position;
   transform.rotation = QQuaternion::fromEulerAngles(rotation);
   transform.scale = scale;

   emit objectChanged();
}

void TransformComponentView::init() {
   std::array blocks{
         QSignalBlocker(this), QSignalBlocker(m_ui->posX), QSignalBlocker(m_ui->posY), QSignalBlocker(m_ui->posZ),
         QSignalBlocker(m_ui->rotX), QSignalBlocker(m_ui->rotY), QSignalBlocker(m_ui->rotZ),
         QSignalBlocker(m_ui->scaleX), QSignalBlocker(m_ui->scaleY), QSignalBlocker(m_ui->scaleZ)};

   auto& transform = m_obj->getComponent<TransformComponent>();

   m_ui->posX->setValue(transform.position.x());
   m_ui->posY->setValue(transform.position.y());
   m_ui->posZ->setValue(transform.position.z());
   auto eulerAngles = transform.rotation.toEulerAngles();
   m_ui->rotX->setValue(eulerAngles.x());
   m_ui->rotY->setValue(eulerAngles.y());
   m_ui->rotZ->setValue(eulerAngles.z());
   m_ui->scaleX->setValue(transform.scale.x());
   m_ui->scaleY->setValue(transform.scale.y());
   m_ui->scaleZ->setValue(transform.scale.z());
}
