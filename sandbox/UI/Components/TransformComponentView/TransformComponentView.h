#pragma once
#include "Model/Components/TransformComponent.h"
#include "UI/Components/ComponentsView.h"
#include <QWidget>

namespace Ui {
   class TransformComponentView;
}

class TransformComponentView : public QWidget, public ComponentsView<TransformComponentView, TransformComponent> {
   Q_OBJECT

public:
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
