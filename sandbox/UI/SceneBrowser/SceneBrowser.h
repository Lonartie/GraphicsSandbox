#pragma once
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "Model/Hierarchy/Scene.h"
#include <QWidget>

namespace Ui {
   class SceneBrowser;
}

class SceneBrowser : public QWidget {
   Q_OBJECT

public:
   explicit SceneBrowser(QWidget* parent = nullptr);
   ~SceneBrowser() override;

signals:
   void objectSelected(Object* object);
   void sceneChanged();

public slots:
   void setScene(Scene* scene);
   void rebuild();

protected:
   bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
   void onSelectionChanged();
   void handleDropEvent(QDropEvent* event);

private:
   QTreeWidgetItem* createItemForObject(Object* obj);
   QTreeWidgetItem* lastItemBefore(QTreeWidgetItem* item);

private:
   Ui::SceneBrowser* m_ui = nullptr;
   std::vector<QTreeWidgetItem*> m_items;
   Scene* m_scene = nullptr;
};
