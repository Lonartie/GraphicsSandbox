#include "SceneBrowser.h"

#include <QFileDialog>

#include "ui_SceneBrowser.h"
#include <QMenu>
#include <QMetaEnum>
#include <QMouseEvent>
#include <ranges>
#include <QMimeData>
#include <QTimer>
#include <unordered_set>

#include "Importer/AssimpImporter.h"

SceneBrowser::SceneBrowser(QWidget* parent)
   : QWidget(parent), m_ui(new Ui::SceneBrowser) {
   qRegisterMetaType<Object*>();

   m_ui->setupUi(this);
   m_ui->list->setSelectionBehavior(QAbstractItemView::SelectRows);
   m_ui->list->setSelectionMode(QAbstractItemView::SingleSelection);
   connect(m_ui->list, &QTreeWidget::itemSelectionChanged, this, &SceneBrowser::onSelectionChanged);
   m_ui->list->installEventFilter(this);
   m_ui->list->setDragEnabled(true);
   m_ui->list->viewport()->setAcceptDrops(true);
   m_ui->list->setDropIndicatorShown(true);
   m_ui->list->viewport()->installEventFilter(this);
}

SceneBrowser::~SceneBrowser() { delete m_ui; }

void SceneBrowser::setScene(Scene* scene) {
   m_scene = scene;
   rebuild();
}

void SceneBrowser::rebuild() {
   // save selected item id
   std::optional<QUuid> selectedId;
   auto selectedItems = m_ui->list->selectedItems();
   if (!selectedItems.empty()) {
      selectedId = selectedItems.first()->data(0, Qt::UserRole).value<QUuid>();
   }

   // save expanded state
   std::unordered_map<QUuid, bool, QtHasher<QUuid> > expanded;
   for (auto& item: m_items) {
      auto id = item->data(0, Qt::UserRole).value<QUuid>();
      expanded[id] = item->isExpanded();
   }

   // clear items
   QSignalBlocker blocker(m_ui->list);
   m_items.clear();
   m_ui->list->clear();

   // check order consistency
   std::unordered_set<uint64_t> orders;
   uint64_t maxOrder = 0;
   for (auto* obj: m_scene->objects()) {
      if (orders.contains(obj->order())) { obj->setOrder(maxOrder + 1); }
      orders.insert(obj->order());
      maxOrder = std::max(maxOrder, obj->order());
   }

   // rebuild items
   auto objectsView = m_scene->objects()
                      | std::views::filter([](Object* obj) { return !obj->parent(); });
   auto objects = std::vector(objectsView.begin(), objectsView.end());
   std::ranges::sort(objects, [](Object* a, Object* b) { return a->order() < b->order(); });
   for (auto* obj: objects) {
      const auto item = createItemForObject(obj);
      m_ui->list->addTopLevelItem(item);
   }

   // restore expanded state
   for (auto& item: m_items) {
      auto id = item->data(0, Qt::UserRole).value<QUuid>();
      auto it = expanded.find(id);
      if (it != expanded.end()) { item->setExpanded(it->second); }
   }

   // restore selected item id if it still exists
   if (selectedId) {
      for (auto& item: m_items) {
         if (item->data(0, Qt::UserRole).value<QUuid>() == *selectedId) {
            item->setSelected(true);
            return;
         }
      }

      // the selected item no longer exists
      emit objectSelected(nullptr);
   }
}

void SceneBrowser::onSelectionChanged() {
   if (m_ui->list->selectedItems().empty()) {
      emit objectSelected(nullptr);
      return;
   }
   auto selectedItem = m_ui->list->selectedItems().first();
   auto obj = *m_scene->findObject(selectedItem->data(0, Qt::UserRole).value<QUuid>());
   emit objectSelected(obj);
}

QTreeWidgetItem* SceneBrowser::createItemForObject(Object* obj) {
   auto item = new QTreeWidgetItem();
   m_items.push_back(item);

   item->setText(0, obj->name());
   item->setData(0, Qt::UserRole, obj->id());

   auto children = obj->children();
   std::ranges::sort(children, [](Object* a, Object* b) { return a->order() < b->order(); });
   for (auto* child: children) {
      auto childItem = createItemForObject(child);
      item->addChild(childItem);
   }

   return item;
}

QTreeWidgetItem* SceneBrowser::lastItemBefore(QTreeWidgetItem* item) {
   QTreeWidgetItem* lastWatched = nullptr;
   std::function<bool(QTreeWidgetItem*)> findLast;
   findLast = [&](QTreeWidgetItem* current) {
      if (current == item) return true;
      lastWatched = current;
      for (int i = 0; i < current->childCount(); ++i) {
         if (findLast(current->child(i))) { return true; }
      }
      return false;
   };

   for (int topLevelI = 0; topLevelI < m_ui->list->topLevelItemCount(); ++topLevelI) {
      if (findLast(m_ui->list->topLevelItem(topLevelI))) { return lastWatched; }
   }

   return nullptr;
}

bool SceneBrowser::eventFilter(QObject* watched, QEvent* event) {
   if (event->type() == QEvent::ContextMenu) {
      QTimer::singleShot(0, [this] {
         auto globalPos = QCursor::pos();
         auto item = m_ui->list->itemAt(
               m_ui->list->viewport()->mapFromGlobal(globalPos));
         QMenu menu;

         if (item) {
            auto obj = *m_scene->findObject(
                  item->data(0, Qt::UserRole).value<QUuid>());
            menu.addAction("Delete", [this, obj] {
               m_scene->removeObject(*obj);
               rebuild();
               emit sceneChanged();
            });
            menu.addAction("Copy", [this, obj] {
               auto& nobj = m_scene->copyObject(*obj);
               rebuild();
               for (auto& item: m_items) {
                  if (item->data(0, Qt::UserRole).value<QUuid>()
                      == nobj.id()) {
                     item->setSelected(true);
                     emit objectSelected(
                           *m_scene->findObject(nobj.id()));
                  } else { item->setSelected(false); }
               }
               emit sceneChanged();
            });
            menu.addAction("Add", [this, obj, pitem = item] {
               pitem->setExpanded(true);
               auto nobj = Object::create(*m_scene);
               auto nobjID = nobj->id();
               m_scene->addObject(std::move(nobj));
               m_scene->addChild(
                     *obj, *m_scene->findObject(nobjID).value());
               rebuild();
               for (auto& item: m_items) {
                  if (item->data(0, Qt::UserRole).value<QUuid>()
                      == nobjID) {
                     item->setSelected(true);
                     emit objectSelected(
                           *m_scene->findObject(nobjID));
                  } else { item->setSelected(false); }
               }
               emit sceneChanged();
            });
         } else {
            menu.addAction("Add", [this] {
               auto obj = Object::create(*m_scene);
               auto objID = obj->id();
               m_scene->addObject(std::move(obj));
               rebuild();
               for (auto& item: m_items) {
                  if (item->data(0, Qt::UserRole).value<QUuid>()
                      == objID) {
                     item->setSelected(true);
                     emit objectSelected(
                           *m_scene->findObject(objID));
                  } else { item->setSelected(false); }
               }
               emit sceneChanged();
            });
         }

         menu.addAction("Import", [this] {
            auto path = QFileDialog::getOpenFileName(
                  this, "Import", QString(),
                  "Model Object (*.*)");
            if (path.isEmpty()) return;
            AssimpImporter::loadInto(path, *m_scene);
            rebuild();
            emit sceneChanged();
         });

         menu.exec(globalPos);
      });
      return true;
   }

   if (event->type() == QEvent::Drop) {
      handleDropEvent(dynamic_cast<QDropEvent*>(event));
      return QWidget::eventFilter(watched, event);;
   }

   return QWidget::eventFilter(watched, event);
}

void SceneBrowser::handleDropEvent(QDropEvent* event) {
   QTimer::singleShot(0, [this] {
      // identify item that has been dropped
      std::optional<QTreeWidgetItem*> unseenItem;
      std::function<void(QTreeWidgetItem*)> findItemFunction;
      findItemFunction = [&](QTreeWidgetItem* item) {
         if (unseenItem) return;
         auto iter = std::ranges::find(m_items, item);
         if (iter == m_items.end()) {
            unseenItem = item;
            return;
         }
         for (int i = 0; i < item->childCount() && !unseenItem; ++i) {
            findItemFunction(item->child(i));
         }
      };
      for (int i = 0; i < m_ui->list->topLevelItemCount() && !unseenItem; ++i) {
         findItemFunction(m_ui->list->topLevelItem(i));
      }

      if (!unseenItem) {
         GS_DEBUG() << "Couldn't identify dropped item";
         return;
      }

      auto droppedItem = *unseenItem;
      auto droppedID = droppedItem->data(0, Qt::UserRole).value<QUuid>();
      auto droppedObject = m_scene->findObject(droppedID);

      if (!droppedObject) {
         GS_DEBUG() << "Couldn't find dropped object";
         return;
      }

      auto droppedObjGlobalTrans = (*droppedObject)->getComponent<TransformComponent>().toGlobal();

      // identify its parent
      auto droppedItemParent = droppedItem->parent();
      auto droppedObjectOrder = (*droppedObject)->order();
      uint64_t newOrder = 0;
      if (!droppedItemParent) {
         GS_DEBUG() << "Dropped object has no parent";
         (*droppedObject)->unsetParent();

         if (m_ui->list->indexOfTopLevelItem(droppedItem) > 0) {
            if (const auto* lastItemBeforeOurs = lastItemBefore(droppedItem)) {
               newOrder = (*m_scene->findObject(
                                lastItemBeforeOurs->data(0, Qt::UserRole).value<QUuid>()))->order()
                          + 1;
            }
         }

         auto& droppedObjTrans = (*droppedObject)->getComponent<TransformComponent>();
         droppedObjTrans.position = droppedObjGlobalTrans.position;
         droppedObjTrans.rotation = droppedObjGlobalTrans.rotation;
         droppedObjTrans.scale = droppedObjGlobalTrans.scale;
      } else {
         auto parentID = droppedItemParent->data(0, Qt::UserRole).value<QUuid>();
         auto parentObject = m_scene->findObject(parentID);
         if (!parentObject) {
            GS_DEBUG() << "Couldn't find parent object";
            return;
         }

         // cycle detection
         if (droppedID == parentID) {
            GS_DEBUG() << "Dropped onto itself, borting";
            rebuild();
            emit sceneChanged();
            return;
         }

         auto allChildren = m_scene->allChildrenOf(**droppedObject);
         if (std::ranges::find(allChildren, *parentObject) != allChildren.end()) {
            GS_DEBUG() << "Cycle detected, borting";
            rebuild();
            emit sceneChanged();
            return;
         }

         m_scene->addChild(**parentObject, **droppedObject);

         if (const auto* lastItemBeforeOurs = lastItemBefore(droppedItem)) {
            // it must have an item before because it has a parent!
            newOrder = (*m_scene->findObject(
                             lastItemBeforeOurs->data(0, Qt::UserRole).value<QUuid>()))->order() +
                       1;
         } else { qFatal() << "Couldn't find last item before dropped item"; }

         auto& droppedObjTrans = (*droppedObject)->getComponent<TransformComponent>();
         auto localTrans = droppedObjTrans.fromGlobal(droppedObjGlobalTrans);
         droppedObjTrans.position = localTrans.position;
         droppedObjTrans.rotation = localTrans.rotation;
         droppedObjTrans.scale = localTrans.scale;
      }

      // shift up
      for (auto* obj: m_scene->objects()) {
         if (obj->order() >= newOrder && obj->id() != droppedID) {
            obj->setOrder(obj->order() + 1);
         }
      }

      // set new order
      (*droppedObject)->setOrder(newOrder);

      // close gaps
      uint64_t order = 0;
      auto orderedObjects = m_scene->objects();
      std::ranges::sort(orderedObjects,
                        [](Object* a, Object* b) { return a->order() < b->order(); });
      for (auto* obj: orderedObjects) { obj->setOrder(order++); }

      rebuild();
      emit sceneChanged();
   });
}