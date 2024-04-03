#include "OpenGLView.h"
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
   m_movementTimer.setInterval(16);
   m_movementTimer.start();
   connect(&m_movementTimer, &QTimer::timeout, this, [this] {
      if (editorEnabled()) {
         m_editorTrans.position += QVector3D(m_movement.x(), 0, m_movement.y());
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

   QOpenGLWidget::initializeGL();
}

void OpenGLView::resizeGL(int w, int h) {
   auto scalingFactor = devicePixelRatio();
   m_renderer->resize(w * scalingFactor, h * scalingFactor);
   QOpenGLWidget::resizeGL(w, h);
}

void OpenGLView::paintGL() {
   m_renderer->render();

   const auto now = std::chrono::high_resolution_clock::now();
   const auto durationMS = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastRenderTime).count() / 1000.0f;
   m_lastRenderTime = now;

   if (now - m_lastTimeNotify > std::chrono::milliseconds(500)) {
      m_lastTimeNotify = now;
      emit timeChanged(durationMS);
   }

   // schedule next update immediately
   update();
}

void OpenGLView::setScene(Scene* scene) {
   m_scene = std::move(scene);
   if (m_renderer) {
      m_renderer->setScene(m_scene);
   }
}
bool OpenGLView::eventFilter(QObject* watched, QEvent* ev) {

   if (ev->type() == QEvent::KeyPress) {
      auto event = static_cast<QKeyEvent*>(ev);
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
      repaint();
   }

   if (editorEnabled()) {
      // handle camera rotation
      if (ev->type() == QEvent::MouseButtonPress) {
         auto event = dynamic_cast<QMouseEvent*>(ev);
         if (event->button() == Qt::MouseButton::RightButton) {
            m_editorCamRotating = true;
            m_lastMousePos = event->pos();
         }
      } else if (ev->type() == QEvent::MouseButtonRelease) {
         auto event = dynamic_cast<QMouseEvent*>(ev);
         if (event->button() == Qt::MouseButton::RightButton) {
            m_editorCamRotating = false;
         }
      } else if (ev->type() == QEvent::MouseMove && m_editorCamRotating) {
         auto event = dynamic_cast<QMouseEvent*>(ev);
         if (m_editorCamRotating) {
            auto delta = event->pos() - m_lastMousePos;
            auto euler = m_editorTrans.rotation.toEulerAngles();
            euler += QVector3D(delta.y(), delta.x(), 0);
            m_editorTrans.rotation = QQuaternion::fromEulerAngles(euler);
            m_lastMousePos = event->pos();
            m_renderer->setEditorTrans(m_editorTrans);
         }
      }

      // handle wasd
      auto speed = 0.1f;
      if (ev->type() == QEvent::KeyPress) {
         auto event = dynamic_cast<QKeyEvent*>(ev);
         auto key = event->key();
         if (key == Qt::Key_W) {
            m_movement += QVector2D(0, -speed);
         } else if (key == Qt::Key_S) {
            m_movement += QVector2D(0, speed);
         } else if (key == Qt::Key_A) {
            m_movement += QVector2D(-speed, 0);
         } else if (key == Qt::Key_D) {
            m_movement += QVector2D(speed, 0);
         }
      } else if (ev->type() == QEvent::KeyRelease) {
         auto event = dynamic_cast<QKeyEvent*>(ev);
         auto key = event->key();
         if (key == Qt::Key_W) {
            m_movement -= QVector2D(0, -speed);
         } else if (key == Qt::Key_S) {
            m_movement -= QVector2D(0, speed);
         } else if (key == Qt::Key_A) {
            m_movement -= QVector2D(-speed, 0);
         } else if (key == Qt::Key_D) {
            m_movement -= QVector2D(speed, 0);
         }
      }
   }

   return false;
}

void OpenGLView::enableEditorCam(bool enable) {
   if (!m_renderer) return;
   m_renderer->setEditorCam(enable ? std::optional(m_editorCam) : std::nullopt);
   m_renderer->setEditorTrans(enable ? std::optional(m_editorTrans) : std::nullopt);
}

bool OpenGLView::editorEnabled() const {
   return m_renderer && m_renderer->editorCam() && m_renderer->editorTrans();
}
