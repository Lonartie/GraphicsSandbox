#include "OpenGLView.h"
#include <QOpenGLFunctions>
#include <QKeyEvent>

[[maybe_unused]] static auto OpenGLViewReg = AutoRegisterView<OpenGLView>::registered;

OpenGLView::OpenGLView(QWidget* parent)
    : QOpenGLWidget(parent) {
   setFocusPolicy(Qt::FocusPolicy::StrongFocus);
   installEventFilter(this);
}

OpenGLView::~OpenGLView() {
   makeCurrent();
   doneCurrent();
}

QWidget* OpenGLView::asWidget() {
   return this;
}

void OpenGLView::initializeGL() {
   m_renderer = new OpenGLRenderer(context(), this);
   m_renderer->initializeOpenGLFunctions();
   m_renderer->setScene(m_scene);

   QOpenGLWidget::initializeGL();
}

void OpenGLView::resizeGL(int w, int h) {
   m_renderer->resize(w, h);
   QOpenGLWidget::resizeGL(w, h);
}

void OpenGLView::paintGL() {
   m_renderer->render();
}

void OpenGLView::setScene(sptr<Scene> scene) {
   m_scene = std::move(scene);
   if (m_renderer) {
      m_renderer->setScene(m_scene);
   }
}
bool OpenGLView::eventFilter(QObject* watched, QEvent* ev) {
   if (ev->type() == QEvent::KeyPress) {
      auto event = static_cast<QKeyEvent*>(ev);
      auto key = event->key();
      switch (key) {
         case Qt::Key_0: m_renderer->resetLastStage(); break;
         case Qt::Key_1: m_renderer->setLastStage(1); break;
         case Qt::Key_2: m_renderer->setLastStage(2); break;
         case Qt::Key_3: m_renderer->setLastStage(3); break;
         case Qt::Key_4: m_renderer->setLastStage(4); break;
         case Qt::Key_5: m_renderer->setLastStage(5); break;
         case Qt::Key_6: m_renderer->setLastStage(6); break;
         case Qt::Key_7: m_renderer->setLastStage(7); break;
         case Qt::Key_8: m_renderer->setLastStage(8); break;
         case Qt::Key_9: m_renderer->setLastStage(9); break;
      }
      repaint();
   }

   return false;
}
