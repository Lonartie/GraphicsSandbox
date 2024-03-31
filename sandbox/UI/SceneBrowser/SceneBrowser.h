#pragma once
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

public slots:
   void setScene(Scene* scene);
   void rebuild();

protected:
   bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
   void onSelectionChanged();

private:
   Ui::SceneBrowser* m_ui = nullptr;
   Scene* m_scene = nullptr;
};
