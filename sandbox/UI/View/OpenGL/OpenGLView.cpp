#include "OpenGLView.h"
#include <QOpenGLFunctions>

[[maybe_unused]] static auto OpenGLViewReg = AutoRegisterView<OpenGLView>::registered;

OpenGLView::OpenGLView(QWidget* parent)
    : QOpenGLWidget(parent) {
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
