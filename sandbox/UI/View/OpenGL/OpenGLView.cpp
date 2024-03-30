#include "OpenGLView.h"

[[maybe_unused]] static AutoRegisterView<OpenGLView> OpenGLViewReg = {};

OpenGLView::OpenGLView(QWidget* parent)
    : QOpenGLWidget(parent) {
}

OpenGLView::~OpenGLView() {
}

QWidget* OpenGLView::asWidget() {
   return this;
}
