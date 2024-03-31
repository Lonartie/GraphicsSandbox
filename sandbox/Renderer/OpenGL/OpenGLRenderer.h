#pragma once
#include "Model/Components/CameraComponent.h"
#include "Model/Hierarchy/Scene.h"
#include <QObject>
#include <QOpenGLFunctions>

class OpenGLRenderer : public QObject, public QOpenGLFunctions {
   Q_OBJECT

public:
   explicit OpenGLRenderer(QOpenGLContext* context = nullptr, QObject* parent = nullptr);

   void render();

public slots:
   void setScene(sptr<Scene> scene);

private:
   void renderCamera(const CameraComponent& camera);

private:
   sptr<Scene> m_scene = nullptr;
};
