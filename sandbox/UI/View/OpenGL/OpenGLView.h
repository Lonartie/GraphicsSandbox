#pragma once
#include "Model/Hierarchy/Scene.h"
#include "Renderer/OpenGL/OpenGLRenderer.h"
#include "UI/View/ViewBase.h"
#include <QOpenGLWidget>
#include <QTimer>
#include <QVector2D>

class OpenGLView : public QOpenGLWidget, public ViewBase {
   Q_OBJECT

public:
   static inline QString Name = "OpenGLView";

   explicit OpenGLView(QWidget* parent = nullptr);
   ~OpenGLView() override;

   [[nodiscard]] QWidget* asWidget() override;

signals:
   void timeChanged(float time, float renderTime);

public slots:
   void setScene(Scene* scene) override;

protected:
   void initializeGL() override;
   void resizeGL(int w, int h) override;
   void paintGL() override;

   bool eventFilter(QObject* watched, QEvent* event) override;

private:
   void enableEditorCam(bool enable);
   bool editorEnabled() const;

private:
   std::chrono::high_resolution_clock::time_point m_lastTimeNotify;
   std::chrono::high_resolution_clock::time_point m_lastRenderTime;
   OpenGLRenderer* m_renderer = nullptr;
   Scene* m_scene = nullptr;
   bool m_editorCamRotating = false;
   float m_speed = .2f;
   float m_mouseSpeed = .2f;
   QVector3D m_movement;
   QVector2D m_rotation;
   QPoint m_lastMousePos;
   QTimer m_movementTimer;
   CameraComponent m_editorCam = CameraComponent(nullptr);
   TransformComponent m_editorTrans = TransformComponent(nullptr);
};
