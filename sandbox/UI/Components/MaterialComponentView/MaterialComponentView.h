#pragma once
#include "Model/Components/MaterialComponent.h"
#include "UI/Components/ComponentsView.h"
#include <QWidget>

namespace Ui {
   class MaterialComponentView;
}

class MaterialComponentView : public QWidget, public ComponentsView<MaterialComponentView, MaterialComponent> {
   Q_OBJECT

public:
   static constexpr auto Order = 2;
   static inline auto Dependencies = { u"Mesh"_s };

   explicit MaterialComponentView(QWidget* parent = nullptr);
   ~MaterialComponentView() override;

   QWidget* asWidget() override;
   void init() override;

signals:
   void objectChanged();

private slots:
   void selectColor();
   void updateValues();

private:
   Ui::MaterialComponentView* m_ui;
};
