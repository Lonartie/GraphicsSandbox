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
      for (auto& [_, cam]: m_scene->components<CameraComponent>()) {
         renderCamera(cam, cam.parent().getComponent<TransformComponent>());
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

   QMatrix4x4 projection = camera.projectionMatrix(float(viewport.width()) / float(viewport.height()));
   QMatrix4x4 view = camera.viewMatrix(transform);

   // Draw scene
   glViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height());
   for (auto& [_, mesh]: m_scene->components<MeshComponent>()) {
      auto& meshTransform = mesh.parent().getComponent<TransformComponent>();
      QMatrix4x4 model = meshTransform.modelMatrix();
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
   auto& mesh = obj->getComponent<MeshComponent>();
   auto* prgm = program(obj);

   // setting up the shader program
   m_vao->bind();
   prgm->bind();
   mesh.prepare(prgm);
   mesh.bind(prgm);

   // bind uniform data for all shaders
   prgm->setUniformValue("model", model);
   prgm->setUniformValue("view", view);
   prgm->setUniformValue("projection", projection);

   // bind shader specific uniform data
   if (obj->hasComponent<MaterialComponent>()) {
      auto& material = obj->getComponent<MaterialComponent>();
      material.prepare(prgm);
      material.bind(prgm);
   }

   glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_SHORT, nullptr);

   m_vao->release();
   mesh.release(prgm);

   // release shader specific uniform data
   if (obj->hasComponent<MaterialComponent>()) {
      auto& material = obj->getComponent<MaterialComponent>();
      material.release(prgm);
   }
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
