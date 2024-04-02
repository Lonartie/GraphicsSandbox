#pragma once
#include "Model/Hierarchy/Scene.h"
#include "Renderer/OpenGL/OpenGLRenderer.h"
#include "UI/View/ViewBase.h"
#include <QOpenGLWidget>
#include <QTimer>

class OpenGLView : public QOpenGLWidget, public ViewBase {
   Q_OBJECT

public:
   static inline QString Name = "OpenGLView";

   explicit OpenGLView(QWidget* parent = nullptr);
   ~OpenGLView() override;

   [[nodiscard]] QWidget* asWidget() override;

signals:
   void timeChanged(float time);

public slots:
   void setScene(Scene* scene) override;

protected:
   void initializeGL() override;
   void resizeGL(int w, int h) override;
   void paintGL() override;

   bool eventFilter(QObject* watched, QEvent* event) override;

private:
   std::chrono::high_resolution_clock::time_point m_lastTimeNotify;
   std::chrono::high_resolution_clock::time_point m_lastRenderTime;
   OpenGLRenderer* m_renderer = nullptr;
   Scene* m_scene = nullptr;
};
