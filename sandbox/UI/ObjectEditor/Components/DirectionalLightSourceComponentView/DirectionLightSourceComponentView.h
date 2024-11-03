#pragma once

#include "Model/Components/DirectionalLightSourceComponent.h"
#include "UI/ObjectEditor/Components/ComponentsView.h"
#include "UI/ObjectEditor/Components/CameraComponentView/CameraComponentView.h"

#include <QWidget>


namespace Ui {
   class DirectionLightSourceComponentView;
}

class DirectionLightSourceComponentView : public QWidget,
                                          public ComponentsView<
                                             DirectionLightSourceComponentView,
                                             DirectionalLightSourceComponent> {
   Q_OBJECT

public:
   static constexpr auto Order = 2;
   static inline auto Dependencies = {u"Transform"_s};

   explicit DirectionLightSourceComponentView(QWidget* parent = nullptr);

   ~DirectionLightSourceComponentView() override;

   QWidget* asWidget() override;

   void init() override;

signals:
   void objectChanged() const;

private slots:
   void setColorFrame(const QColor& color);

   void selectColor();

   void updateValues();

private:
   Ui::DirectionLightSourceComponentView* m_ui;
};