#include "MeshOpenGLWidget.h"

MeshOpenGLWidget::MeshOpenGLWidget(MeshComponent mesh, QWidget* parent)
   : QOpenGLWidget(parent), m_mesh(std::move(mesh)) {
}

MeshOpenGLWidget::~MeshOpenGLWidget() {
   makeCurrent();
   doneCurrent();
}

void MeshOpenGLWidget::initializeGL() {

}

void MeshOpenGLWidget::resizeGL(int w, int h) {
   QOpenGLWidget::resizeGL(w, h);
}

void MeshOpenGLWidget::paintGL() {
   QOpenGLWidget::paintGL();
}
