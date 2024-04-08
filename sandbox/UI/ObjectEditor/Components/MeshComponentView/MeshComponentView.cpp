#include "MeshComponentView.h"
#include "Model/Components/MaterialComponent.h"
#include "ui_MeshComponentView.h"

MeshComponentView::MeshComponentView(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::MeshComponentView) {
   REG_ASSERT(Registered);

   m_ui->setupUi(this);

   for (auto& [name, _]: primitives) {
      m_ui->type->addItem(name);
   }

   connect(m_ui->type, qOverload<int>(&QComboBox::currentIndexChanged), this, &MeshComponentView::updateValues);

   m_ui->view->disableLiveUpdates();
   m_ui->view->enableInspectionCamera();
}

MeshComponentView::~MeshComponentView() {
   delete m_ui;
}

QWidget* MeshComponentView::asWidget() {
   return this;
}

void MeshComponentView::init() {
   auto& mesh = m_obj->getComponent<MeshComponent>();
   m_ui->view->setScene(recomposeScene(mesh));
   m_ui->view->update();

   for (auto& [name, data]: primitives) {
      if (mesh.asTuple() == data) {
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
      auto& tuple = primitives[m_ui->type->currentText()];
      std::tie(mesh.vertices, mesh.uvs, mesh.normals, mesh.indices) = tuple;
      mesh.dirty();
   }

   m_ui->vertexCount->setText(QString::number(mesh.vertices.size()));
   m_ui->view->setScene(recomposeScene(mesh));
   m_ui->view->update();
   emit objectChanged();
}

Scene* MeshComponentView::recomposeScene(const MeshComponent& mesh) {
   m_scene = Scene::createEmpty();

   auto obj = Object::create(*m_scene);
   obj->addComponent<MeshComponent>();
   obj->addComponent<MaterialComponent>();
   obj->getComponent<MaterialComponent>().shader = "Default";
   obj->getComponent<MaterialComponent>().properties.emplace(
         "solidColor",
         MaterialComponent::Property{
               .type = "QColor",
               .value = QColor(Qt::white)});
   obj->getComponent<MeshComponent>().asTuple() = mesh.asTuple();

   m_scene->addObject(std::move(obj));

   return m_scene.get();
}
