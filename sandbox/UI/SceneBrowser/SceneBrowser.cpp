#include "SceneBrowser.h"
#include "ui_SceneBrowser.h"

SceneBrowser::SceneBrowser(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::SceneBrowser) {
   m_ui->setupUi(this);
}

SceneBrowser::~SceneBrowser() {
   delete m_ui;
}
