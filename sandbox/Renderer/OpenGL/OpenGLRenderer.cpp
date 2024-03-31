#include "OpenGLRenderer.h"
#include "Model/Components/CameraComponent.h"

#include <utility>

OpenGLRenderer::OpenGLRenderer(QOpenGLContext* context, QObject* parent)
    : QObject(parent), QOpenGLFunctions(context) {
}

void OpenGLRenderer::setScene(sptr<Scene> scene) {
   m_scene = std::move(scene);
}

void OpenGLRenderer::render() {
   for (auto& [_, comp] : ComponentsRegistry<CameraComponent>::Components()) {
      renderCamera(comp);
   }
}

void OpenGLRenderer::renderCamera(const CameraComponent& camera) {
   auto& transform = camera.parent().getComponent<TransformComponent>();
   auto backgroundColor = camera.backgroundColor;

   glClearColor(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF(), backgroundColor.alphaF());
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}