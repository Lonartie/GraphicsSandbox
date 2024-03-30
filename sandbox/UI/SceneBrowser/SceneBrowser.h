#pragma once
#include <QWidget>

namespace Ui {
   class SceneBrowser;
}

class SceneBrowser : public QWidget {
   Q_OBJECT

public:
   explicit SceneBrowser(QWidget* parent = nullptr);
   ~SceneBrowser() override;

private:
   Ui::SceneBrowser* m_ui;
};
