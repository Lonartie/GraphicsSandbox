#pragma once
#include "Model/Components/MeshComponent.h"
#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_4_0_Core>

class MeshOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_0_Core {
   Q_OBJECT

public:
   explicit MeshOpenGLWidget(QWidget* parent = nullptr);
   ~MeshOpenGLWidget() override;

public slots:
   void setMesh(MeshComponent* mesh);

protected:
   void initializeGL() override;
   void resizeGL(int w, int h) override;
   void paintGL() override;

   bool eventFilter(QObject* watched, QEvent* event) override;


private:
   QMatrix4x4 projection() const;

private:
   MeshComponent* m_mesh = nullptr;
   QQuaternion m_rotation;
   QPoint m_lastMousePos;
   bool m_rotating = false;
   float m_zoom = 1.0;

   QOpenGLVertexArrayObject m_vao;
   QOpenGLBuffer* m_vbo = nullptr;
   QOpenGLBuffer* m_vib = nullptr;
   QOpenGLShaderProgram m_program;
};
