#include "ObjectEditor.h"
#include "UI/Components/ComponentsView.h"
#include "ui_ObjectEditor.h"
#include <QGroupBox>
#include <QSpacerItem>
#include <QTimer>

ObjectEditor::ObjectEditor(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::ObjectEditor) {
   m_ui->setupUi(this);

   connect(m_ui->name, &QLineEdit::textChanged, [this](const QString& text) { m_obj->setName(text); });
   connect(m_ui->name, &QLineEdit::textChanged, this, &ObjectEditor::objectChanged);

   for (auto& [name, _]: ComponentsViewBase::Views())
      m_ui->component->addItem(name);
   m_ui->component->setCurrentIndex(0);
   connect(m_ui->addComponent, &QPushButton::clicked, this, &ObjectEditor::addComponent);

   rebuild();
}

ObjectEditor::~ObjectEditor() {
   delete m_ui;
}

void ObjectEditor::setObject(Object* object) {
   m_obj = object;
   rebuild();
}

void ObjectEditor::rebuild() {
   QSignalBlocker block(this);
   QSignalBlocker block2(m_ui->name);

   m_widgets.clear();
   this->setEnabled(m_obj != nullptr);
   m_ui->name->setText("");
   while (auto* item = m_ui->scrollContent->layout()->takeAt(0)) {
      if (item->widget()) delete item->widget();
      delete item;
   }

   if (!m_obj) return;
   m_ui->name->setText(m_obj->name());
   auto views = ComponentsViewBase::Views();
   for (auto& [name, creator]: views) {
      if (auto* view = creator(this, m_obj)) {
         auto* group = new QGroupBox(name);
         group->setLayout(new QVBoxLayout);
         auto widget = view->asWidget();
         widget->installEventFilter(this);
         group->layout()->addWidget(widget);
         m_ui->scrollContent->layout()->addWidget(group);
         m_widgets.push_back(widget);
      }
   }

   m_ui->scrollContent->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

bool ObjectEditor::eventFilter(QObject* watched, QEvent* event) {
   auto widget = qobject_cast<QWidget*>(watched);
   if (!widget) return false;
   auto iter = std::find(m_widgets.begin(), m_widgets.end(), widget);
   if (iter == m_widgets.end()) return false;

   if (event->type() == QEvent::Resize) {
      auto* group = qobject_cast<QGroupBox*>(widget->parent());
      auto minWidth = group->minimumSizeHint().width() + 20;
      QTimer::singleShot(0, this, [this, minWidth] {
         auto lastMinWidth = m_ui->scrollArea->minimumWidth();
         if (lastMinWidth < minWidth) {
            m_ui->scrollArea->setMinimumWidth(minWidth);
         }
      });
   }

   return false;
}

void ObjectEditor::addComponent() {
   if (!m_obj) return;
   auto componentName = m_ui->component->currentText();
   ComponentsViewBase::AddComponent().at(componentName)(m_obj);
   emit objectChanged();
   rebuild();
}
