#pragma once
#include "Model/Components/CameraComponent.h"
#include "Model/Hierarchy/Scene.h"
#include <QObject>
#include <QOpenGLFunctions>

class OpenGLRenderer : public QObject, public QOpenGLFunctions {
   Q_OBJECT

public:
   explicit OpenGLRenderer(QOpenGLContext* context = nullptr, QObject* parent = nullptr);


public slots:
   void setScene(sptr<Scene> scene);
   void render();
   void resize(int w, int h);

   void resetLastStage();
   void setLastStage(int stage);

private:
   void renderCamera(const CameraComponent& camera);

private:
   sptr<Scene> m_scene = nullptr;
   int m_lastStage = std::numeric_limits<int>::max();
   int m_width = 0;
   int m_height = 0;
};
