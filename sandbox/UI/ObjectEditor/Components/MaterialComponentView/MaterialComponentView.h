#pragma once
#include "Model/Components/MaterialComponent.h"
#include "UI/ObjectEditor/Components/ComponentsView.h"
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

public slots:
   void init() override;
   void selectShader(QString name, MaterialComponent& mat);

signals:
   void objectChanged();

private slots:
   void updateValues(const QString& name, const QString& type, const QVariant& value);

   void createColorField(const QString& name, MaterialComponent& mat);
   void createFloatField(const QString& name, MaterialComponent& mat);
   void createIntField(const QString& name, MaterialComponent& mat);
   void createBoolField(const QString& name, MaterialComponent& mat);
   void createImageField(const QString& name, MaterialComponent& mat);
   void createVector2DField(const QString& name, MaterialComponent& mat);
   void createVector3DField(const QString& name, MaterialComponent& mat);
   void createSizeField(const QString& name, MaterialComponent& mat);

private:
   Ui::MaterialComponentView* m_ui;
   std::vector<QWidget*> m_widgets;
};
