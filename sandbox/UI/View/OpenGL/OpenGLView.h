#pragma once
#include "UI/View/ViewBase.h"
#include <QOpenGLWidget>

class OpenGLView : public QOpenGLWidget, public ViewBase {
   Q_OBJECT

public:
   static inline QString Name = "OpenGLView";

   explicit OpenGLView(QWidget* parent = nullptr);
   ~OpenGLView() override;

   [[nodiscard]] QWidget* asWidget() override;

protected:
   void initializeGL() override;
   void resizeGL(int w, int h) override;
   void paintGL() override;

private:
};
