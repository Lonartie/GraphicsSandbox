#include "MaterialComponentView.h"
#include "ui_MaterialComponentView.h"
#include <QColorDialog>

MaterialComponentView::MaterialComponentView(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::MaterialComponentView) {
   REG_ASSERT(Registered);

   m_ui->setupUi(this);

   connect(m_ui->select, &QPushButton::pressed, this, &MaterialComponentView::selectColor);
}

MaterialComponentView::~MaterialComponentView() {
   delete m_ui;
}

QWidget* MaterialComponentView::asWidget() {
   return this;
}

void MaterialComponentView::init() {
   auto& material = m_obj->getComponent<MaterialComponent>();
   m_ui->color->setStyleSheet("background-color: " + material.solidColor.name() + ";");
}

void MaterialComponentView::updateValues() {
   auto& material = m_obj->getComponent<MaterialComponent>();
   m_ui->color->setStyleSheet("background-color: " + material.solidColor.name() + ";");
}

void MaterialComponentView::selectColor() {
   auto& material = m_obj->getComponent<MaterialComponent>();
   auto before = material.solidColor;

   QColorDialog diag(before, this);

   connect(&diag, &QColorDialog::currentColorChanged, this, [this, &material](const QColor& color) {
      material.solidColor = color;
      updateValues();
      emit objectChanged();
   });

   if (diag.exec() == QDialog::Rejected) {
      material.solidColor = before;
      updateValues();
   }
}
