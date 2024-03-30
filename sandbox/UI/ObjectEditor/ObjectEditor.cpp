#include "ObjectEditor.h"
#include "ui_ObjectEditor.h"

ObjectEditor::ObjectEditor(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::ObjectEditor) {
   m_ui->setupUi(this);
}

ObjectEditor::~ObjectEditor() {
   delete m_ui;
}
