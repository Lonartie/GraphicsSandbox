#pragma once
#include "Model/Hierarchy/Object.h"
#include <QWidget>

namespace Ui {
   class ObjectEditor;
}

class ObjectEditor : public QWidget {
   Q_OBJECT

public:
   explicit ObjectEditor(QWidget* parent = nullptr);
   ~ObjectEditor() override;

signals:
   void objectChanged();

public slots:
   void setObject(Object* object);
   void rebuild();

protected:
   bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
   void addComponent();

private:
   Ui::ObjectEditor* m_ui = nullptr;
   std::vector<QWidget*> m_widgets;
   Object* m_obj = nullptr;
};
