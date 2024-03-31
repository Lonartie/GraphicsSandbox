#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QJsonDocument>
#include <QStyleFactory>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_ui(new Ui::MainWindow) {

   m_ui->setupUi(this);
   buildUI();

   m_ui->loadScene->setShortcut(QKeySequence::Open);
   connect(m_ui->loadScene, &QAction::triggered, this, &MainWindow::loadScene);

   m_ui->saveScene->setShortcut(QKeySequence::Save);
   connect(m_ui->saveScene, &QAction::triggered, this, &MainWindow::saveScene);

   connect(m_sceneBrowser, &SceneBrowser::objectSelected, m_objectEditor, &ObjectEditor::setObject);
   connect(m_objectEditor, &ObjectEditor::objectChanged, m_sceneBrowser, &SceneBrowser::rebuild);
   connect(m_objectEditor, &ObjectEditor::objectChanged, [this] { m_view->asWidget()->repaint(); });
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

void MainWindow::buildUI() {
   for (auto style : QStyleFactory::keys()) {
      auto* action = new QAction(style, this);
      connect(action, &QAction::triggered, [style] { QApplication::setStyle(style); });
      m_ui->styles->addAction(action);
   }

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

   m_sceneBrowser->setScene(m_scene.get());
}

void MainWindow::loadScene() {
   auto filename = QFileDialog::getOpenFileName(this, "Open Scene", "", "Scene Files (*.scene)");
   if (filename.isEmpty()) {
      return;
   }

   QFile file(filename);
   file.open(QIODevice::ReadOnly);
   QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
   file.close();

   auto scene = Scene::createFromJson(doc.object());
   m_sceneBrowser->setScene(scene.get());
   m_scene = std::move(scene);
}

void MainWindow::saveScene() {
   auto filename = QFileDialog::getSaveFileName(this, "Save Scene", "", "Scene Files (*.scene)");
   if (filename.isEmpty()) {
      return;
   }

   QFile file(filename);
   file.open(QIODevice::WriteOnly);
   file.write(QJsonDocument(m_scene->toJson()).toJson());
   file.close();
}
