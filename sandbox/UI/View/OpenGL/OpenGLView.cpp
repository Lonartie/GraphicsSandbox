#include "OpenGLView.h"
#include <QApplication>
#include <QKeyEvent>
#include <QOpenGLFunctions>

[[maybe_unused]] static auto OpenGLViewReg = AutoRegisterView<OpenGLView>::registered;

OpenGLView::OpenGLView(QWidget* parent)
    : QOpenGLWidget(parent) {
   auto format = QSurfaceFormat::defaultFormat();
   format.setSamples(4);
   setFormat(format);
   setFocusPolicy(Qt::FocusPolicy::StrongFocus);
   installEventFilter(this);
   m_movementTimer.setInterval(1);
   m_movementTimer.start();
   connect(&m_movementTimer, &QTimer::timeout, this, [this] {
      if (editorEnabled() && !m_inspectionCamera) {
         const auto direction = m_movement.normalized();
         const auto globalVector = direction * m_speed;
         const auto localVector = m_editorTrans.rotation.rotatedVector(globalVector);
         m_editorTrans.position += localVector;
         m_renderer->setEditorTrans(m_editorTrans);
      }
   });
}

OpenGLView::~OpenGLView() {
   makeCurrent();
   delete m_renderer;
   doneCurrent();
}

QWidget* OpenGLView::asWidget() {
   return this;
}

void OpenGLView::initializeGL() {
   m_renderer = new OpenGLRenderer(context());
   m_renderer->init();
   m_renderer->setScene(m_scene);
   if (m_inspectionCamera) {
      m_renderer->setEditorTrans(m_editorTrans);
      m_renderer->setEditorCam(m_editorCam);
   }
   enableEditorCam(m_inspectionCamera);

   QOpenGLWidget::initializeGL();
}

void OpenGLView::resizeGL(int w, int h) {
   auto scalingFactor = devicePixelRatio();
   m_renderer->resize(w * scalingFactor, h * scalingFactor);
   QOpenGLWidget::resizeGL(w, h);
}

void OpenGLView::paintGL() {
   const auto before = std::chrono::high_resolution_clock::now();
   m_renderer->render();
   const auto now = std::chrono::high_resolution_clock::now();
   const auto durationMS = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastRenderTime).count() / 1000.0f;
   m_lastRenderTime = now;

   if (now - m_lastTimeNotify > std::chrono::milliseconds(500)) {
      m_lastTimeNotify = now;
      const auto rawRenderTimeMS = std::chrono::duration_cast<std::chrono::microseconds>(now - before).count() / 1000.0f;
      emit timeChanged(durationMS, rawRenderTimeMS);
   }

   if (m_live) {
      // schedule next update immediately
      update();
   }
}

void OpenGLView::setScene(Scene* scene) {
   m_scene = scene;
   if (m_renderer) {
      m_renderer->setScene(m_scene);
   }
}

bool OpenGLView::eventFilter(QObject* watched, QEvent* ev) {
   if (ev->type() == QEvent::KeyPress) {
      auto event = dynamic_cast<QKeyEvent*>(ev);
      auto key = event->key();
      switch (key) {
         case Qt::Key_0:
            m_renderer->resetLastStage();
            break;
         case Qt::Key_1:
            m_renderer->setLastStage(1);
            break;
         case Qt::Key_2:
            m_renderer->setLastStage(2);
            break;
         case Qt::Key_3:
            m_renderer->setLastStage(3);
            break;
         case Qt::Key_4:
            m_renderer->setLastStage(4);
            break;
         case Qt::Key_5:
            m_renderer->setLastStage(5);
            break;
         case Qt::Key_6:
            m_renderer->setLastStage(6);
            break;
         case Qt::Key_7:
            m_renderer->setLastStage(7);
            break;
         case Qt::Key_8:
            m_renderer->setLastStage(8);
            break;
         case Qt::Key_9:
            m_renderer->setLastStage(9);
            break;

         case Qt::Key_E:
            enableEditorCam(true);
            break;
         case Qt::Key_R:
            enableEditorCam(false);
            break;
      }
   }

   if (editorEnabled()) {
      auto button = m_inspectionCamera ? Qt::MouseButton::LeftButton : Qt::MouseButton::RightButton;
      // handle camera rotation
      if (ev->type() == QEvent::MouseButtonPress) {
         auto event = dynamic_cast<QMouseEvent*>(ev);
         if (event->button() == button) {
            m_editorCamRotating = true;
            m_lastMousePos = event->pos();
            setCursor(Qt::BlankCursor);
            update();
         }
      } else if (ev->type() == QEvent::MouseButtonRelease) {
         auto event = dynamic_cast<QMouseEvent*>(ev);
         if (event->button() == button) {
            m_editorCamRotating = false;
            unsetCursor();
            update();
         }
      } else if (ev->type() == QEvent::MouseMove && m_editorCamRotating) {
         auto event = dynamic_cast<QMouseEvent*>(ev);
         if (m_editorCamRotating) {
            auto delta = event->pos() - m_lastMousePos;
            m_rotation += QVector2D(-delta.y(), -delta.x()) * m_mouseSpeed;

            if (!m_inspectionCamera) {
               auto rotationX = QQuaternion::fromEulerAngles(QVector3D(m_rotation.x(), 0, 0));
               auto rotationY = QQuaternion::fromEulerAngles(QVector3D(0, m_rotation.y(), 0));
               m_editorTrans.rotation = rotationY * rotationX;
            } else {
               // rotate camera around (0,0,0) with constant distance of 5
               const auto center = QVector3D(0, 0, 0);

               auto rotationX = QQuaternion::fromEulerAngles(QVector3D(m_rotation.x(), 0, 0));
               auto rotationY = QQuaternion::fromEulerAngles(QVector3D(0, m_rotation.y(), 0));

               auto centerRotation = rotationY * rotationX;

               const auto globalForward = QVector3D(0, 0, -1);
               const auto direction = centerRotation.rotatedVector(globalForward).normalized();
               const auto cameraPosition = center + direction * -5;

               m_editorTrans.position = cameraPosition;
               m_editorTrans.rotation = centerRotation;
            }

            m_renderer->setEditorTrans(m_editorTrans);
            QCursor::setPos(mapToGlobal(m_lastMousePos));
            update();
         }
      }

      // handle wasd
      if (!m_inspectionCamera) {
         if (ev->type() == QEvent::KeyPress) {
            auto event = dynamic_cast<QKeyEvent*>(ev);
            auto key = event->key();
            if (key == Qt::Key_W) {
               m_movement += QVector3D(0, 0, -1);
            } else if (key == Qt::Key_S) {
               m_movement += QVector3D(0, 0, 1);
            } else if (key == Qt::Key_A) {
               m_movement += QVector3D(-1, 0, 0);
            } else if (key == Qt::Key_D) {
               m_movement += QVector3D(1, 0, 0);
            } else if (key == Qt::Key_E) {
               m_movement += QVector3D(0, 1, 0);
            } else if (key == Qt::Key_Q) {
               m_movement += QVector3D(0, -1, 0);
            }

            if (key == Qt::Key_Shift) {
               m_speed = 1.f;
            }
            if (key == Qt::Key_Alt) {
               m_speed = .01f;
            }
         } else if (ev->type() == QEvent::KeyRelease) {
            auto event = dynamic_cast<QKeyEvent*>(ev);
            auto key = event->key();
            if (key == Qt::Key_W) {
               m_movement -= QVector3D(0, 0, -1);
            } else if (key == Qt::Key_S) {
               m_movement -= QVector3D(0, 0, 1);
            } else if (key == Qt::Key_A) {
               m_movement -= QVector3D(-1, 0, 0);
            } else if (key == Qt::Key_D) {
               m_movement -= QVector3D(1, 0, 0);
            } else if (key == Qt::Key_E) {
               m_movement -= QVector3D(0, 1, 0);
            } else if (key == Qt::Key_Q) {
               m_movement -= QVector3D(0, -1, 0);
            }

            if (key == Qt::Key_Shift || key == Qt::Key_Alt) {
               m_speed = .2f;
            }

            // if something is stuck, reset with esc
            if (key == Qt::Key_Escape) {
               m_speed = .2f;
               m_movement = {};
            }
         }
      }
   }

   return false;
}

void OpenGLView::enableEditorCam(bool enable) {
   if (!m_renderer) return;
   if (m_inspectionCamera) {
      enable = true;
      m_editorCam.wireframe = true;
   }
   m_renderer->setEditorCam(enable ? std::optional(m_editorCam) : std::nullopt);
   m_renderer->setEditorTrans(enable ? std::optional(m_editorTrans) : std::nullopt);
}

bool OpenGLView::editorEnabled() const {
   return m_renderer && m_renderer->editorCam() && m_renderer->editorTrans();
}

void OpenGLView::disableLiveUpdates() {
   m_live = false;
}

void OpenGLView::enableInspectionCamera() {
   m_inspectionCamera = true;
   m_editorTrans.position = QVector3D(0, 0, 5);
   enableEditorCam(true);
}
