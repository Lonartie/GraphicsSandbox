#pragma once
#include "Model/Components/MeshComponent.h"
#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class MeshOpenGLWidget : public QOpenGLWidget {
   Q_OBJECT

public:
   explicit MeshOpenGLWidget(MeshComponent mesh, QWidget* parent = nullptr);
   ~MeshOpenGLWidget() override;

protected:
   void initializeGL() override;
   void resizeGL(int w, int h) override;
   void paintGL() override;

private:
   MeshComponent m_mesh;

   QOpenGLVertexArrayObject* m_vao = nullptr;
   QOpenGLBuffer* m_vbo = nullptr;
   QOpenGLShaderProgram* m_program = nullptr;
};
