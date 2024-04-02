#include "CameraComponentView.h"
#include "ui_CameraComponentView.h"
#include <QColorDialog>
#include <QSlider>

CameraComponentView::CameraComponentView(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::CameraComponentView) {
   REG_ASSERT(Registered);

   m_ui->setupUi(this);

   connect(m_ui->fov, qOverload<int>(&QSlider::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->near, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->far, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->viewportX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->viewportY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->viewportWidth, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->viewportHeight, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CameraComponentView::updateValues);
   connect(m_ui->wireframe, &QCheckBox::stateChanged, this, &CameraComponentView::updateValues);
   connect(m_ui->colorSelect, &QPushButton::pressed, [this]() {
      auto& camera = m_obj->getComponent<CameraComponent>();
      auto colorBefore = camera.backgroundColor;
      QColorDialog diag(camera.backgroundColor);
      connect(&diag, &QColorDialog::currentColorChanged, this, [this](const QColor& color) {
         if (color.isValid()) {
            auto& camera = m_obj->getComponent<CameraComponent>();
            camera.backgroundColor = color;
            m_ui->color->setStyleSheet(QStringLiteral("background-color: %1").arg(color.name(QColor::HexRgb)));
            emit objectChanged();
         }
      });
      if (diag.exec() == QDialog::Rejected) {
         auto& camera = m_obj->getComponent<CameraComponent>();
         camera.backgroundColor = colorBefore;
         m_ui->color->setStyleSheet(QStringLiteral("background-color: %1").arg(colorBefore.name(QColor::HexRgb)));
         emit objectChanged();
      }
   });
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
   camera.wireframe = m_ui->wireframe->isChecked();

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
         QSignalBlocker(m_ui->viewportHeight),
         QSignalBlocker(m_ui->wireframe),
   };
   auto& camera = m_obj->getComponent<CameraComponent>();

   m_ui->fov->setValue(camera.fov);
   m_ui->near->setValue(camera.nearClip);
   m_ui->far->setValue(camera.farClip);
   m_ui->viewportX->setValue(camera.viewport.x());
   m_ui->viewportY->setValue(camera.viewport.y());
   m_ui->viewportWidth->setValue(camera.viewport.width());
   m_ui->viewportHeight->setValue(camera.viewport.height());
   m_ui->wireframe->setChecked(camera.wireframe);

   m_ui->fovValue->setText(QString::number(camera.fov) + QStringLiteral("°"));
   m_ui->color->setStyleSheet(QStringLiteral("background-color: %1").arg(camera.backgroundColor.name(QColor::HexRgb)));
}
