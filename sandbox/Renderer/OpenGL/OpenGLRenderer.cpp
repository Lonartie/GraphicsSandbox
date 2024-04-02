#include "OpenGLRenderer.h"
#include "Model/Components/CameraComponent.h"
#include "Model/Components/MeshComponent.h"

#include <QSurface>
#include <utility>

OpenGLRenderer::OpenGLRenderer(QOpenGLContext* context, QObject* parent)
    : QObject(parent), QOpenGLFunctions(context) {
}

void OpenGLRenderer::setScene(Scene* scene) {
   m_scene = scene;
}

void OpenGLRenderer::render() {
   // Draw default background color
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   if (m_lastStage <= 1) return;

   for (auto& [_, comp] : ComponentsRegistry<CameraComponent>::Components()) {
      renderCamera(comp);
   }
}

void OpenGLRenderer::renderCamera(const CameraComponent& camera) {
   auto& transform = camera.parent().getComponent<TransformComponent>();
   auto backgroundColor = camera.backgroundColor;

   // Draw background color
   QRect viewport(camera.viewport.x() * m_width,
              camera.viewport.y() * m_height,
              camera.viewport.width() * m_width,
              camera.viewport.height() * m_height);
   glEnable(GL_SCISSOR_TEST);
   glScissor(viewport.x(), viewport.y(), viewport.width(), viewport.height());
   glClearColor(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF(), backgroundColor.alphaF());
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glDisable(GL_SCISSOR_TEST);

   if (m_lastStage <= 2) return;

   QMatrix4x4 projection;
   projection.perspective(camera.fov, float(viewport.width()) / float(viewport.height()), camera.nearClip, camera.farClip);
   QMatrix4x4 view;
   view.translate(transform.position);
   view.rotate(transform.rotation);

   // Draw scene
   glViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height());
   for (auto& [_, mesh]: ComponentsRegistry<MeshComponent>::Components()) {
      auto& meshTransform = mesh.parent().getComponent<TransformComponent>();
      QMatrix4x4 model;
      model.translate(meshTransform.position);
      model.rotate(meshTransform.rotation);
      model.scale(meshTransform.scale);
      draw(model, view, projection, mesh.vertices, mesh.indices);
   }
}

void OpenGLRenderer::resize(int w, int h) {
   m_width = w;
   m_height = h;
}

void OpenGLRenderer::resetLastStage() {
   setLastStage(std::numeric_limits<int>::max());
}

void OpenGLRenderer::setLastStage(int stage) {
   qDebug() << "Set last stage: " << stage;
   m_lastStage = stage;
}

void OpenGLRenderer::init() {
   initializeOpenGLFunctions();

   m_vao = new QOpenGLVertexArrayObject();
   m_vao->create();

   m_vertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
   m_vertexBuffer->create();

   m_indexBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
   m_indexBuffer->create();

   m_program = new QOpenGLShaderProgram();
   m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "Shaders/Default/default.vert");
   m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "Shaders/Default/default.frag");
   m_program->link();

   m_vao->bind();
   m_vertexBuffer->bind();
   m_indexBuffer->bind();
   m_program->bind();

   m_program->enableAttributeArray("worldPos");
   m_program->enableAttributeArray("worldUV");
   m_program->setAttributeBuffer("worldPos", GL_FLOAT, offsetof(VertexData, position), 3, sizeof(VertexData));
   m_program->setAttributeBuffer("worldUV", GL_FLOAT, offsetof(VertexData, uv), 2, sizeof(VertexData));

   m_vao->release();
   m_vertexBuffer->release();
   m_indexBuffer->release();
   m_program->release();
}

void OpenGLRenderer::draw(QMatrix4x4 model, QMatrix4x4 view, QMatrix4x4 projection,
                          const std::vector<VertexData>& vertexData,
                          const std::vector<uint16_t>& indexData,
                          const std::optional<QColor>& solidColor) {
   m_vao->bind();

   m_vertexBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
   m_vertexBuffer->bind();
   m_vertexBuffer->allocate(vertexData.data(), vertexData.size() * sizeof(VertexData));

   m_indexBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
   m_indexBuffer->bind();
   m_indexBuffer->allocate(indexData.data(), indexData.size() * sizeof(uint16_t));

   m_program->setUniformValue("model", model);
   m_program->setUniformValue("view", view);
   m_program->setUniformValue("projection", projection);

   m_vertexBuffer->release();
   m_indexBuffer->release();
   m_vao->release();

   m_vao->bind();
   m_vertexBuffer->bind();
   m_indexBuffer->bind();
   m_program->bind();

   glDrawElements(GL_TRIANGLES, indexData.size(), GL_UNSIGNED_SHORT, nullptr);

   m_vao->release();
   m_vertexBuffer->release();
   m_indexBuffer->release();
}
