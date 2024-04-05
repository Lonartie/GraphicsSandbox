#include "OpenGLRenderer.h"
#include "Common/ShaderProvider.h"
#include "Model/Components/CameraComponent.h"
#include "Model/Components/MaterialComponent.h"
#include "Model/Components/MeshComponent.h"

#include <QOpenGLTexture>
#include <QSurface>
#include <utility>

OpenGLRenderer::OpenGLRenderer(QOpenGLContext* context)
    : QObject(nullptr), QOpenGLFunctions_3_3_Core() {
}

OpenGLRenderer::~OpenGLRenderer() {
   delete m_vao;
   delete m_vertexBuffer;
   delete m_indexBuffer;
}

void OpenGLRenderer::setScene(Scene* scene) {
   m_scene = scene;
}

void OpenGLRenderer::init() {
   if (!initializeOpenGLFunctions()) {
      qFatal("Failed to initialize OpenGL functions");
   }

   glEnable(GL_MULTISAMPLE);// enables multisampling
   glEnable(GL_DEPTH_TEST); // enables depth testing
   glEnable(GL_CULL_FACE);  // enables face culling to only drawObject front faces
   glCullFace(GL_FRONT);    // this will only drawObject the front faces
   glFrontFace(GL_CW);      // this will make the front faces be the ones that are clockwise

   m_vao = new QOpenGLVertexArrayObject();
   m_vao->create();

   m_vertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
   m_vertexBuffer->create();

   m_indexBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
   m_indexBuffer->create();
}

void OpenGLRenderer::render() {
   // Draw default background color
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   if (m_lastStage <= 1) return;

   if (m_editorCam && m_editorTrans) {
      renderCamera(*m_editorCam, *m_editorTrans);
   } else {
      for (auto& [_, comp]: ComponentsRegistry<CameraComponent>::Components()) {
         renderCamera(comp, comp.parent().getComponent<TransformComponent>());
      }
   }
}

void OpenGLRenderer::renderCamera(const CameraComponent& camera, const TransformComponent& transform) {
   QRect viewport = drawBackground(camera);
   if (m_lastStage <= 2) return;
   drawScene(camera, transform, viewport);
}
void OpenGLRenderer::drawScene(const CameraComponent& camera, const TransformComponent& transform, const QRect& viewport) {
   if (camera.wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   }

   QMatrix4x4 projection;
   projection.perspective(camera.fov, float(viewport.width()) / float(viewport.height()), camera.nearClip, camera.farClip);
   QMatrix4x4 view;
   view.translate(0, 0, 0);
   view.rotate(transform.rotation);
   view.translate(transform.position);

   // Draw scene
   glViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height());
   for (auto& [_, mesh]: ComponentsRegistry<MeshComponent>::Components()) {
      auto& meshTransform = mesh.parent().getComponent<TransformComponent>();
      QMatrix4x4 model;
      model.translate(meshTransform.position);
      model.rotate(meshTransform.rotation);
      model.scale(meshTransform.scale);
      drawObject(model, view, projection, &mesh.parent());
   }
}

QRect OpenGLRenderer::drawBackground(const CameraComponent& camera) {
   auto backgroundColor = camera.backgroundColor;
   QRect viewport(camera.viewport.x() * m_width,
                  camera.viewport.y() * m_height,
                  camera.viewport.width() * m_width,
                  camera.viewport.height() * m_height);
   glEnable(GL_SCISSOR_TEST);
   glScissor(viewport.x(), viewport.y(), viewport.width(), viewport.height());
   glClearColor(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF(), backgroundColor.alphaF());
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glDisable(GL_SCISSOR_TEST);
   //   glFlush();
   return viewport;
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

void OpenGLRenderer::drawObject(QMatrix4x4 model, QMatrix4x4 view, QMatrix4x4 projection, Object* obj) {
   auto& vertexData = obj->getComponent<MeshComponent>().vertices;
   auto& indexData = obj->getComponent<MeshComponent>().indices;
   auto* prgm = program(obj);

   {// setting up the shader program
      m_vao->bind();

      m_vertexBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
      m_vertexBuffer->bind();
      m_vertexBuffer->allocate(vertexData.data(), vertexData.size() * sizeof(VertexData));

      m_indexBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
      m_indexBuffer->bind();
      m_indexBuffer->allocate(indexData.data(), indexData.size() * sizeof(uint16_t));

      prgm->bind();
      prgm->enableAttributeArray("worldPos");
      prgm->enableAttributeArray("worldUV");
      prgm->setAttributeBuffer("worldPos", GL_FLOAT, offsetof(VertexData, position), 3, sizeof(VertexData));
      prgm->setAttributeBuffer("worldUV", GL_FLOAT, offsetof(VertexData, uv), 2, sizeof(VertexData));
   }

   // bind uniform data for all shaders
   prgm->setUniformValue("model", model);
   prgm->setUniformValue("view", view);
   prgm->setUniformValue("projection", projection);

   for (auto* texture: m_textures) {
      delete texture;
   }
   m_textures.clear();

   // bind shader specific uniform data
   int texCount = 0;
   if (obj->hasComponent<MaterialComponent>()) {
      auto& material = obj->getComponent<MaterialComponent>();
      for (auto& [name, prop]: material.properties) {
         if (prop.type == "bool") prgm->setUniformValue(name.toStdString().c_str(), prop.value.toBool());
         else if (prop.type == "int")
            prgm->setUniformValue(name.toStdString().c_str(), prop.value.toInt());
         else if (prop.type == "float")
            prgm->setUniformValue(name.toStdString().c_str(), prop.value.toFloat());
         else if (prop.type == "QImage") {
            auto image = prop.value.value<QImage>();
            if (!image.isNull()) {
               auto texture = new QOpenGLTexture(image.mirrored());
               texture->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Linear);
               texture->setWrapMode(QOpenGLTexture::ClampToBorder);
               texture->bind(GL_TEXTURE0 + texCount);
               prgm->setUniformValue(name.toStdString().c_str(), GL_TEXTURE0 + texCount);
               m_textures.push_back(texture);
               texCount++;
            }
         } else if (prop.type == "QColor")
            prgm->setUniformValue(name.toStdString().c_str(), prop.value.value<QColor>());
         else if (prop.type == "QVector2D") {
            auto vec = prop.value.value<QVector2D>();
            prgm->setUniformValue(name.toStdString().c_str(), vec);
         } else if (prop.type == "QVector3D") {
            auto vec = prop.value.value<QVector3D>();
            prgm->setUniformValue(name.toStdString().c_str(), vec);
         } else if (prop.type == "QSizeF") {
            auto vec = prop.value.value<QSizeF>();
            prgm->setUniformValue(name.toStdString().c_str(), vec);
         }
      }
   }

   glDrawElements(GL_TRIANGLES, indexData.size(), GL_UNSIGNED_SHORT, nullptr);

   m_vao->release();
   m_vertexBuffer->release();
   m_indexBuffer->release();
}

const std::optional<CameraComponent>& OpenGLRenderer::editorCam() const {
   return m_editorCam;
}

void OpenGLRenderer::setEditorCam(const std::optional<CameraComponent>& editorCam) {
   m_editorCam = editorCam;
}

const std::optional<TransformComponent>& OpenGLRenderer::editorTrans() const {
   return m_editorTrans;
}

void OpenGLRenderer::setEditorTrans(const std::optional<TransformComponent>& editorTrans) {
   m_editorTrans = editorTrans;
}

QOpenGLShaderProgram* OpenGLRenderer::program(Object* obj) {
   auto shaders = ShaderProvider::instance().getShaders();
   if (obj->hasComponent<MaterialComponent>() && shaders.contains(obj->getComponent<MaterialComponent>().shader)) {
      auto shader = obj->getComponent<MaterialComponent>().shader;
      return shaders.at(shader);
   }
   return ShaderProvider::instance().getShaders().at("Default");
}
