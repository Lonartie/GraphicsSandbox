#include "MainWindow.h"
#include "Model/Components/CameraComponent.h"
#include "Model/Model.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QJsonDocument>
#include <QSplitter>
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_ui(new Ui::MainWindow) {

   m_ui->setupUi(this);

   m_splitter = new QSplitter(Qt::Horizontal);

   m_splitter->addWidget(m_sceneBrowser = new SceneBrowser);
   m_splitter->addWidget((m_view = ViewBase::getCreators()["OpenGLView"](nullptr))->asWidget());
   m_splitter->addWidget(m_objectEditor = new ObjectEditor);

   m_splitter->setStyleSheet("QSplitter::handle { background-color: red; }");
   m_splitter->setHandleWidth(5);

   for (auto& name: ViewBase::getViewNames()) {
      auto action = new QAction(name, this);
      action->setCheckable(true);
      if (name == "OpenGLView") {
         action->setChecked(true);
      }
      connect(action, &QAction::triggered, [this, name] { activateRenderer(name); });
      m_ui->renderer->addAction(action);
   }

   setCentralWidget(m_splitter);
}

MainWindow::~MainWindow() {
   delete m_ui;
}

void MainWindow::activateRenderer(const QString& name) {
   // change widget
   m_view = ViewBase::getCreators()[name](nullptr);
   auto oldWidget = m_splitter->replaceWidget(1, m_view->asWidget());
   delete oldWidget;

   // set correct action checked, uncheck others
   for (auto action: m_ui->renderer->actions()) {
      action->setChecked(action->text() == name);
   }
}
