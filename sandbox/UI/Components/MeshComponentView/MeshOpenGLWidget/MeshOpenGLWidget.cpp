#include "MeshOpenGLWidget.h"
#include <QOpenGLFunctions>
#include <QEvent>
#include <QWheelEvent>
#include <QMouseEvent>

MeshOpenGLWidget::MeshOpenGLWidget(QWidget* parent)
    : QOpenGLWidget(parent) {
   QSurfaceFormat fmt;
   fmt.setRenderableType(QSurfaceFormat::OpenGL);
   fmt.setVersion(4, 0);
   fmt.setProfile(QSurfaceFormat::CoreProfile);
   fmt.setSamples(4);
   setFormat(fmt);

   setMouseTracking(true);
   installEventFilter(this);
}

MeshOpenGLWidget::~MeshOpenGLWidget() {
   makeCurrent();
   doneCurrent();
}

void MeshOpenGLWidget::initializeGL() {
   initializeOpenGLFunctions();

   if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, "Shaders/Default/default.vert")) {
      qWarning() << "Failed to load vertex shader:\n"
                 << m_program.log();
   }
   if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, "Shaders/Default/default.frag")) {
      qWarning() << "Failed to load fragment shader:\n"
                 << m_program.log();
   }
   if (!m_program.link()) {
      qWarning() << "Failed to link program:\n"
                 << m_program.log();
   }

   m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
   m_vbo->create();

   m_vib = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
   m_vib->create();


   m_vao.create();
}

void MeshOpenGLWidget::resizeGL(int w, int h) {
   QOpenGLWidget::resizeGL(w, h);
}

void MeshOpenGLWidget::paintGL() {

   // 1. Clear screen black
   glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());

   glEnable(GL_DEPTH_TEST); // enables depth testing
   glEnable(GL_CULL_FACE); // enables face culling to only drawObject front faces
   glCullFace(GL_FRONT); // this will only drawObject the front faces
   glFrontFace(GL_CW); // this will make the front faces be the ones that are clockwise

   if (m_mesh == nullptr) return;

   // 2. Draw mesh as wireframe
   m_vbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
   m_vbo->bind();
   m_vbo->allocate(m_mesh->vertices.data(), m_mesh->vertices.size() * sizeof(VertexData));
   m_vib->setUsagePattern(QOpenGLBuffer::DynamicDraw);
   m_vib->bind();
   std::vector<GLushort> indices;
   for (auto index: m_mesh->indices) {
      indices.push_back(index);
   }
   m_vib->allocate(indices.data(), m_mesh->indices.size() * sizeof(GLushort));
   m_vao.bind();

   auto posLocation = m_program.attributeLocation("worldPos");
   auto uvLocation = m_program.attributeLocation("worldUV");
   m_program.enableAttributeArray(posLocation);
   m_program.enableAttributeArray(uvLocation);
   m_program.setAttributeBuffer(posLocation, GL_FLOAT, 0, 3, sizeof(VertexData));
   m_program.setAttributeBuffer(uvLocation, GL_FLOAT, sizeof(QVector3D), 2, sizeof(VertexData));

   m_vao.release();
   m_vbo->release();
   m_vib->release();

   m_program.setUniformValue("projection", projection());
   m_program.setUniformValue("model", QMatrix4x4());
   m_program.setUniformValue("view", QMatrix4x4());
   m_program.setUniformValue("solidColor", QColor(Qt::white));

   m_program.bind();
   m_vao.bind();
   m_vbo->bind();
   m_vib->bind();
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glDrawElements(GL_TRIANGLES, m_mesh->indices.size(), GL_UNSIGNED_SHORT, nullptr);
   //   m_vao.release();
   //   m_vbo->release();
   //   m_vib->release();
}

void MeshOpenGLWidget::setMesh(MeshComponent* mesh) {
   m_mesh = mesh;
   update();
}

QMatrix4x4 MeshOpenGLWidget::projection() const {
   QMatrix4x4 projection;
   projection.setToIdentity();
   projection.perspective(75.0f, float(width()) / float(height()), 0.1f, 100.0f);

   QMatrix4x4 camera;
//   camera.translate(0,0,0);
//   camera.rotate(QQuaternion::fromEulerAngles(m_rotation));
   camera.translate(0, 0, -5 * (1 / m_zoom));

   QMatrix4x4 mesh;
   mesh.translate(0,0,0);
   mesh.rotate(m_rotation);

   return projection * camera * mesh;
}

bool MeshOpenGLWidget::eventFilter(QObject* watched, QEvent* event) {
   // check scroll event for zoom
   if (event->type() == QEvent::Wheel) {
      auto wheelEvent = static_cast<QWheelEvent*>(event);
      auto factor = wheelEvent->angleDelta().y() > 0 ? 1.1f : 0.9f;
      m_zoom *= factor;
      update();
   }

   // check mouse down, mouse move, mouse up for rotation
   if (event->type() == QEvent::MouseButtonPress) {
      auto mouseEvent = static_cast<QMouseEvent*>(event);
      m_lastMousePos = mouseEvent->pos();
      m_rotating = true;
   }

   if (event->type() == QEvent::MouseMove && m_rotating) {
      auto mouseEvent = static_cast<QMouseEvent*>(event);
      auto diff = mouseEvent->pos() - m_lastMousePos;
      m_rotation = QQuaternion::fromEulerAngles(QVector3D(diff.y(), diff.x(), 0)) * m_rotation;
      m_lastMousePos = mouseEvent->pos();
      update();
   }

   if (event->type() == QEvent::MouseButtonRelease) {
      m_rotating = false;
   }

   return false;
}
