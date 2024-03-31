#include "CameraComponentView.h"
#include "ui_CameraComponentView.h"
#include <QSlider>

CameraComponentView::CameraComponentView(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::CameraComponentView) {
   Q_ASSERT(Registered);

   m_ui->setupUi(this);

   connect(m_ui->fov, qOverload<int>(&QSlider::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->near, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->far, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->viewportX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->viewportY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->viewportWidth, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->viewportHeight, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CameraComponentView::updateValues);
}

CameraComponentView::~CameraComponentView() {
   delete m_ui;
}

QWidget* CameraComponentView::asWidget() {
   return this;
}

void CameraComponentView::updateValues() {
   if (!m_obj) return;

   QRectF viewport(
         m_ui->viewportX->value(),
         m_ui->viewportY->value(),
         m_ui->viewportWidth->value(),
         m_ui->viewportHeight->value());

   auto& camera = m_obj->getComponent<CameraComponent>();

   camera.fov = m_ui->fov->value();
   camera.nearClip = m_ui->near->value();
   camera.farClip = m_ui->far->value();
   camera.viewport = viewport;

   m_ui->fovValue->setText(QString::number(camera.fov) + QStringLiteral("°"));

   emit objectChanged();
}

void CameraComponentView::init() {
   std::array blocks{
         QSignalBlocker(this),
         QSignalBlocker(m_ui->fov),
         QSignalBlocker(m_ui->near),
         QSignalBlocker(m_ui->far),
         QSignalBlocker(m_ui->viewportX),
         QSignalBlocker(m_ui->viewportY),
         QSignalBlocker(m_ui->viewportWidth),
   };
   auto& camera = m_obj->getComponent<CameraComponent>();

   m_ui->fov->setValue(camera.fov);
   m_ui->near->setValue(camera.nearClip);
   m_ui->far->setValue(camera.farClip);
   m_ui->viewportX->setValue(camera.viewport.x());
   m_ui->viewportY->setValue(camera.viewport.y());
   m_ui->viewportWidth->setValue(camera.viewport.width());
   m_ui->viewportHeight->setValue(camera.viewport.height());

   m_ui->fovValue->setText(QString::number(camera.fov) + QStringLiteral("°"));
}