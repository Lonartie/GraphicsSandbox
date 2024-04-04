#include "MainWindow.h"
#include "UI/View/OpenGL/OpenGLView.h"
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
   m_ui->newScene->setShortcut(QKeySequence::New);
   connect(m_ui->newScene, &QAction::triggered, [this] {
      m_scene = Scene::createEmpty();
      m_ui->sceneBrowser->setScene(m_scene.get());
      m_view->setScene(m_scene.get());
   });

   connect(m_ui->sceneBrowser, &SceneBrowser::objectSelected, m_ui->objectEditor, &ObjectEditor::setObject);
   connect(m_ui->objectEditor, &ObjectEditor::objectChanged, m_ui->sceneBrowser, &SceneBrowser::rebuild);

   if (QFile::exists("test/test.scene")) {
      QFile file("test/test.scene");
      file.open(QIODevice::ReadOnly);
      m_scene = Scene::createFromJson(QJsonDocument::fromJson(file.readAll()).object());
      m_ui->sceneBrowser->setScene(m_scene.get());
      m_view->setScene(m_scene.get());
   }
}

MainWindow::~MainWindow() {
   delete m_ui;
}

void MainWindow::activateRenderer(const QString& name) {
   // change widget
   while (m_ui->tabs->count() != 0) {
      auto tab = m_ui->tabs->widget(0);
      m_ui->tabs->removeTab(0);
      delete tab;
   }

   m_view = ViewBase::getCreators()[name]();
   auto widget = m_view->asWidget();
   m_ui->tabs->addTab(widget, name);
   auto tab = m_ui->tabs->widget(0);
   ((QGridLayout*)m_ui->centralwidget->layout())->setColumnStretch(1, 1);
   tab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   m_ui->tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

   if (auto* openglView = dynamic_cast<OpenGLView*>(m_view)) {
      connect(openglView, &OpenGLView::timeChanged, [this](float time) {
         m_ui->statusbar->showMessage(QString("Time: %1 ms FPS: %2").arg(time).arg(1000.0f / time));
      });
   }

   // set correct action checked, uncheck others
   for (auto action: m_ui->renderer->actions()) {
      action->setChecked(action->text() == name);
   }

   m_view->setScene(m_scene.get());
}

void MainWindow::buildUI() {
   for (auto style : QStyleFactory::keys()) {
      auto* action = new QAction(style, this);
      connect(action, &QAction::triggered, [style] { QApplication::setStyle(style); });
      m_ui->styles->addAction(action);
   }

   for (auto& name: ViewBase::getViewNames()) {
      auto action = new QAction(name, this);
      action->setCheckable(true);
      if (name == "OpenGLView") {
         action->setChecked(true);
      }
      connect(action, &QAction::triggered, [this, name] { activateRenderer(name); });
      m_ui->renderer->addAction(action);
   }

   m_ui->sceneBrowser->setScene(m_scene.get());

   activateRenderer("OpenGLView");
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
   m_ui->sceneBrowser->setScene(scene.get());
   m_view->setScene(scene.get());
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
