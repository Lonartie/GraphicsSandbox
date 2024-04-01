#include "MeshComponentView.h"
#include "ui_MeshComponentView.h"

MeshComponentView::MeshComponentView(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::MeshComponentView) {
   REG_ASSERT(Registered);

   m_ui->setupUi(this);

   for (auto& [name, _]: primitives) {
      m_ui->type->addItem(name);
   }

   connect(m_ui->type, qOverload<int>(&QComboBox::currentIndexChanged), this, &MeshComponentView::updateValues);
}

MeshComponentView::~MeshComponentView() {
   delete m_ui;
}

QWidget* MeshComponentView::asWidget() {
   return this;
}

void MeshComponentView::init() {
   auto& mesh = m_obj->getComponent<MeshComponent>();

   for (auto& [name, data]: primitives) {
      if (mesh.vertices == data.first && mesh.indices == data.second) {
         m_ui->type->setCurrentText(name);
         break;
      }
   }

   m_ui->vertexCount->setText(QString::number(mesh.vertices.size()));
}
void MeshComponentView::updateValues() {
   if (!m_obj) return;

   auto& mesh = m_obj->getComponent<MeshComponent>();

   if (m_ui->type->currentIndex() != 0) {
      auto& [vertices, indices] = primitives[m_ui->type->currentText()];
      mesh.vertices = vertices;
      mesh.indices = indices;
   }

   m_ui->vertexCount->setText(QString::number(mesh.vertices.size()));
   emit objectChanged();
}
