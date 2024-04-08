#pragma once
#include "Model/Components/MeshComponent.h"
#include "UI/ObjectEditor/Components/ComponentsView.h"
#include <QWidget>

namespace Ui {
   class MeshComponentView;
}

class MeshComponentView : public QWidget, public ComponentsView<MeshComponentView, MeshComponent> {
   Q_OBJECT

public:
   static constexpr auto Order = 1;
   static inline auto Dependencies = {u"Transform"_s};

   explicit MeshComponentView(QWidget* parent = nullptr);
   ~MeshComponentView() override;

signals:
   void objectChanged();

public:
   QWidget* asWidget() override;
   void init() override;

private slots:
   void updateValues();

private:
   Scene* recomposeScene(const MeshComponent& mesh);

private:
   Ui::MeshComponentView* m_ui;
   uptr<Scene> m_scene;
};
