#pragma once
#include "UI/ObjectEditor/ObjectEditor.h"
#include "UI/SceneBrowser/SceneBrowser.h"
#include "UI/View/ViewBase.h"
#include "Model/Model.h"
#include <QMainWindow>
#include <QSplitter>

namespace Ui {
   class MainWindow;
}

class MainWindow : public QMainWindow {
   Q_OBJECT

public:
   explicit MainWindow(QWidget* parent = nullptr);
   ~MainWindow() override;

private slots:
   void activateRenderer(const QString& name);

private:
   Ui::MainWindow* m_ui = nullptr;
   SceneBrowser* m_sceneBrowser = nullptr;
   ObjectEditor* m_objectEditor = nullptr;
   ViewBase* m_view = nullptr;
   QSplitter* m_splitter = nullptr;
   uptr<Scene> m_scene = Scene::createEmpty();
};