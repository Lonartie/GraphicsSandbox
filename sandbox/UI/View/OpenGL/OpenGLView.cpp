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
   QOpenGLWidget::initializeGL();
}

void OpenGLView::resizeGL(int w, int h) {
   QOpenGLWidget::resizeGL(w, h);
}

void OpenGLView::paintGL() {
   auto f = context()->functions();
   f->glClearColor(.8f, .8f, .8f, 1.f);
   f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
