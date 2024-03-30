#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QSplitter>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_ui(new Ui::MainWindow) {

   m_ui->setupUi(this);

   auto splitter = new QSplitter(Qt::Horizontal);

   splitter->addWidget(m_sceneBrowser = new SceneBrowser);
   splitter->addWidget((m_view = ViewBase::getCreators()["OpenGLView"](nullptr))->asWidget());
   splitter->addWidget(m_objectEditor = new ObjectEditor);
   splitter->setStyleSheet("QSplitter::handle { background-color: red; }");
   splitter->setHandleWidth(5);

   m_sceneBrowser->setContentsMargins(5, 5, 5, 5);
   m_view->asWidget()->setContentsMargins(5, 5, 5, 5);
   m_objectEditor->setContentsMargins(5, 5, 5, 5);

   setCentralWidget(splitter);
}

MainWindow::~MainWindow() {
   delete m_ui;
}
