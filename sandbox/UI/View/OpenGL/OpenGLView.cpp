#include "OpenGLView.h"
#include <QOpenGLFunctions>
#include <QKeyEvent>

[[maybe_unused]] static auto OpenGLViewReg = AutoRegisterView<OpenGLView>::registered;

OpenGLView::OpenGLView(QWidget* parent)
    : QOpenGLWidget(parent) {
   auto format = QSurfaceFormat::defaultFormat();
   format.setSamples(4);
   setFormat(format);
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
   m_renderer->init();
   m_renderer->setScene(m_scene);

   QOpenGLWidget::initializeGL();
}

void OpenGLView::resizeGL(int w, int h) {
   auto scalingFactor = devicePixelRatio();
   m_renderer->resize(w * scalingFactor, h * scalingFactor);
   QOpenGLWidget::resizeGL(w, h);
}

void OpenGLView::paintGL() {
   m_renderer->render();

   const auto now = std::chrono::high_resolution_clock::now();
   const auto durationMS = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastRenderTime).count() / 1000.0f;
   m_lastRenderTime = now;

   if (now - m_lastTimeNotify > std::chrono::milliseconds(500)) {
      m_lastTimeNotify = now;
      emit timeChanged(durationMS);
   }

   // schedule next update immediately
   update();
}

void OpenGLView::setScene(Scene* scene) {
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
