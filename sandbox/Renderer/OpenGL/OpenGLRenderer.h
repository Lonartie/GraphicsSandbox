#pragma once
#include "Model/Components/CameraComponent.h"
#include "Model/Components/MeshComponent.h"
#include "Model/Hierarchy/Scene.h"
#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

class OpenGLRenderer : public QObject, public QOpenGLFunctions_3_3_Core {
   Q_OBJECT

public:
   explicit OpenGLRenderer(QOpenGLContext* context = nullptr);
   ~OpenGLRenderer() override;

public slots:
   void init();

   void setScene(Scene* scene);
   void render();
   void resize(int w, int h);

   void resetLastStage();
   void setLastStage(int stage);

   const std::optional<CameraComponent>& editorCam() const;
   void setEditorCam(const std::optional<CameraComponent>& editorCam);
   const std::optional<TransformComponent>& editorTrans() const;
   void setEditorTrans(const std::optional<TransformComponent>& editorTrans);

private:
   void renderCamera(const CameraComponent& camera, const TransformComponent& transform);
   void drawObject(QMatrix4x4 model, QMatrix4x4 view, QMatrix4x4 projection,
             const std::vector<VertexData>& vertexData,
             const std::vector<uint16_t>& indexData,
             const std::optional<QColor>& solidColor = std::nullopt);

private:
   Scene* m_scene = nullptr;
   int m_lastStage = std::numeric_limits<int>::max();
   int m_width = 0;
   int m_height = 0;

   std::optional<CameraComponent> m_editorCam;
   std::optional<TransformComponent> m_editorTrans;

   QOpenGLVertexArrayObject* m_vao = nullptr;
   QOpenGLBuffer* m_vertexBuffer = nullptr;
   QOpenGLBuffer* m_indexBuffer = nullptr;
   QOpenGLShaderProgram* m_program = nullptr;

   QRect drawBackground(const CameraComponent& camera);
   void drawScene(const CameraComponent& camera, const TransformComponent& transform, const QRect& viewport);
};
