#pragma once
#include "Model/Components/TransformComponent.h"
#include "UI/ObjectEditor/Components/ComponentsView.h"
#include <QWidget>

namespace Ui {
   class TransformComponentView;
}

class TransformComponentView : public QWidget, public ComponentsView<TransformComponentView, TransformComponent> {
   Q_OBJECT

public:
   static constexpr auto Order = 0;

   explicit TransformComponentView(QWidget* parent = nullptr);
   ~TransformComponentView() override;

   QWidget* asWidget() override;
   void init() override;

signals:
   void objectChanged();

private slots:
   void updateValues();

private:
   Ui::TransformComponentView* m_ui;
};
