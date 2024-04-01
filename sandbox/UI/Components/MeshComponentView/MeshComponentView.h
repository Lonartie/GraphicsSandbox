#pragma once
#include "Model/Components/MeshComponent.h"
#include "UI/Components/ComponentsView.h"
#include <QWidget>

namespace Ui {
   class MeshComponentView;
}

class MeshComponentView : public QWidget, public ComponentsView<MeshComponentView, MeshComponent> {
   Q_OBJECT

public:
   static constexpr auto Name = "Mesh";
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
   Ui::MeshComponentView* m_ui;
};
