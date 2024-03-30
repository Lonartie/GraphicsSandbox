#pragma once
#include "Common/ViewRegistrar.h"
#include "UI/ObjectEditor/ObjectEditor.h"
#include "UI/SceneBrowser/SceneBrowser.h"
#include <QMainWindow>

namespace Ui {
   class MainWindow;
}

class MainWindow : public QMainWindow {
   Q_OBJECT

public:
   explicit MainWindow(QWidget* parent = nullptr);
   ~MainWindow() override;

private:
   Ui::MainWindow* m_ui = nullptr;
   SceneBrowser* m_sceneBrowser = nullptr;
   ObjectEditor* m_objectEditor = nullptr;
   ViewBase* m_view = nullptr;
};