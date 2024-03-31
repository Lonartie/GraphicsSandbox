#include "SceneBrowser.h"
#include "ui_SceneBrowser.h"
#include <QMenu>
#include <QMetaEnum>
#include <QMouseEvent>

SceneBrowser::SceneBrowser(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::SceneBrowser) {
   m_ui->setupUi(this);
   m_ui->list->setSelectionBehavior(QAbstractItemView::SelectRows);
   m_ui->list->setSelectionMode(QAbstractItemView::SingleSelection);
   connect(m_ui->list, &QListWidget::itemSelectionChanged, this, &SceneBrowser::onSelectionChanged);
   m_ui->list->installEventFilter(this);
}

SceneBrowser::~SceneBrowser() {
   delete m_ui;
}

void SceneBrowser::setScene(Scene* scene) {
   m_scene = scene;
   rebuild();
}

void SceneBrowser::rebuild() {
   // save selected item id
   std::optional<QUuid> selectedId;
   auto selectedItems = m_ui->list->selectedItems();
   if (!selectedItems.empty()) {
      selectedId = selectedItems.first()->data(Qt::UserRole).value<Object*>()->id();
   }

   QSignalBlocker blocker(m_ui->list);
   m_ui->list->clear();
   for (auto* obj : m_scene->objects()) {
      auto item = new QListWidgetItem(obj->name());
      item->setData(Qt::UserRole, QVariant::fromValue(obj));
      m_ui->list->addItem(item);
   }

   // restore selected item id if it still exists
   if (selectedId) {
      for (int i = 0; i < m_ui->list->count(); ++i) {
         auto item = m_ui->list->item(i);
         if (item->data(Qt::UserRole).value<Object*>()->id() == *selectedId) {
            item->setSelected(true);
            return;
         }
      }

      // the selected item no longer exists
      emit objectSelected(nullptr);
   }
}

void SceneBrowser::onSelectionChanged() {
   auto selectedItem = m_ui->list->selectedItems().first();
   auto obj = selectedItem->data(Qt::UserRole).value<Object*>();
   emit objectSelected(obj);
}

bool SceneBrowser::eventFilter(QObject* watched, QEvent* event) {
   if (event->type() == QEvent::ContextMenu) {
      auto inputEvent = dynamic_cast<QInputEvent*>(event);
      auto globalPos = QCursor::pos();
      auto item = m_ui->list->itemAt(m_ui->list->mapFromGlobal(globalPos));
      QMenu menu;

      if (item) {
         auto obj = item->data(Qt::UserRole).value<Object*>();
         menu.addAction("Delete", [this, obj] {
            m_scene->removeObject(*obj);
            rebuild();
         });
      }

      menu.addAction("Add", [this] {
         auto obj = Object::create();
         m_scene->addObject(std::move(obj));
         rebuild();
      });

      menu.exec(globalPos);
   }

   return QWidget::eventFilter(watched, event);
}
