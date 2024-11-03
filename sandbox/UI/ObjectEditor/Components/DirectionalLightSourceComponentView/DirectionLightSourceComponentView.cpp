#include "DirectionLightSourceComponentView.h"
#include "ui_DirectionLightSourceComponentView.h"

#include <QColorDialog>

DirectionLightSourceComponentView::DirectionLightSourceComponentView(
      QWidget* parent) : QWidget(parent), m_ui(new Ui::DirectionLightSourceComponentView) {
   REG_ASSERT(Registered);
   m_ui->setupUi(this);

   connect(m_ui->colorButton, &QPushButton::pressed, this,
           &DirectionLightSourceComponentView::selectColor);
   connect(m_ui->intensitySpinBox, &QDoubleSpinBox::valueChanged, this,
           &DirectionLightSourceComponentView::updateValues);
   connect(m_ui->shadowComboBox, &QComboBox::currentIndexChanged, this,
           &DirectionLightSourceComponentView::updateValues);
}

DirectionLightSourceComponentView::~DirectionLightSourceComponentView() { delete m_ui; }

QWidget* DirectionLightSourceComponentView::asWidget() { return this; }

void DirectionLightSourceComponentView::init() {
   const auto& comp = m_obj->getComponent<DirectionalLightSourceComponent>();
   setColorFrame(comp.color);
   m_ui->intensitySpinBox->setValue(comp.intensity);
   m_ui->shadowComboBox->setCurrentIndex(comp.shadowType.type);
}

void DirectionLightSourceComponentView::setColorFrame(const QColor& color) {
   QImage image(35, 35, QImage::Format_RGB888);
   image.fill(color);
   m_ui->colorFrame->setPixmap(QPixmap::fromImage(image));
}

void DirectionLightSourceComponentView::selectColor() {
   const auto& comp = m_obj->getComponent<DirectionalLightSourceComponent>();
   QColorDialog diag(this);
   diag.setCurrentColor(comp.color);
   if (diag.exec() == QDialog::Accepted) {
      setColorFrame(diag.currentColor());
      updateValues();
   }
}

void DirectionLightSourceComponentView::updateValues() {
   auto& comp = m_obj->getComponent<DirectionalLightSourceComponent>();
   comp.intensity = m_ui->intensitySpinBox->value();
   comp.shadowType.type = static_cast<ShadowType::Type>(m_ui->shadowComboBox->currentIndex());
   comp.color = m_ui->colorFrame->pixmap().toImage().pixelColor(0, 0);
}