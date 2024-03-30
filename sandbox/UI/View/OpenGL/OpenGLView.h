#pragma once
#include "Common/ViewRegistrar.h"
#include <QOpenGLWidget>

class OpenGLView : public QOpenGLWidget, public ViewBase {
   Q_OBJECT

public:
   static inline QString Name = "OpenGLView";

   explicit OpenGLView(QWidget* parent = nullptr);
   ~OpenGLView() override;

   [[nodiscard]] QWidget* asWidget() override;

private:
};
