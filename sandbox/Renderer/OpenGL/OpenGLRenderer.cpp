#include "OpenGLRenderer.h"
#include "Model/Components/CameraComponent.h"

#include <QSurface>
#include <utility>

OpenGLRenderer::OpenGLRenderer(QOpenGLContext* context, QObject* parent)
    : QObject(parent), QOpenGLFunctions(context) {
}

void OpenGLRenderer::setScene(sptr<Scene> scene) {
   m_scene = std::move(scene);
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

   // Draw scene
   glViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height());
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
