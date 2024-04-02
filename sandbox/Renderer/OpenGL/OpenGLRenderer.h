#pragma once
#include "Model/Components/CameraComponent.h"
#include "Model/Components/MeshComponent.h"
#include "Model/Hierarchy/Scene.h"
#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

class OpenGLRenderer : public QObject, public QOpenGLFunctions {
   Q_OBJECT

public:
   explicit OpenGLRenderer(QOpenGLContext* context = nullptr, QObject* parent = nullptr);


public slots:
   void init();

   void setScene(Scene* scene);
   void render();
   void resize(int w, int h);

   void resetLastStage();
   void setLastStage(int stage);

private:
   void renderCamera(const CameraComponent& camera);
   void draw(QMatrix4x4 model, QMatrix4x4 view, QMatrix4x4 projection,
             const std::vector<VertexData>& vertexData,
             const std::vector<uint16_t>& indexData,
             const std::optional<QColor>& solidColor = std::nullopt);

private:
   Scene* m_scene = nullptr;
   int m_lastStage = std::numeric_limits<int>::max();
   int m_width = 0;
   int m_height = 0;

   QOpenGLVertexArrayObject* m_vao = nullptr;
   QOpenGLBuffer* m_vertexBuffer = nullptr;
   QOpenGLBuffer* m_indexBuffer = nullptr;
   QOpenGLShaderProgram* m_program = nullptr;
};
