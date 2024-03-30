#pragma once
#include <QWidget>

namespace Ui {
   class ObjectEditor;
}

class ObjectEditor : public QWidget {
   Q_OBJECT

public:
   explicit ObjectEditor(QWidget* parent = nullptr);
   ~ObjectEditor() override;

private:
   Ui::ObjectEditor* m_ui;
};
