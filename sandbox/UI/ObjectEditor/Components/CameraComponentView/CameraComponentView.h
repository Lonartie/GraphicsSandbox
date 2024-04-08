#pragma once
#include <QWidget>
#include "Model/Components/CameraComponent.h"
#include "UI/ObjectEditor/Components/ComponentsView.h"

namespace Ui {
   class CameraComponentView;
}

class CameraComponentView : public QWidget, public ComponentsView<CameraComponentView, CameraComponent> {
   Q_OBJECT

public:
   static constexpr auto Order = 1;
   static inline auto Dependencies = { u"Transform"_s };

   explicit CameraComponentView(QWidget* parent = nullptr);
   ~CameraComponentView() override;

   QWidget* asWidget() override;
   void init() override;

signals:
   void objectChanged();

private slots:
   void updateValues();

private:
   Ui::CameraComponentView* m_ui;
};
