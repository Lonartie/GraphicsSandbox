#pragma once

#include "Common/Common.h"
#include "Component.h"
#include "ComponentsRegistry.h"
#include <QVector3D>
#include <tuple>
#include <unordered_map>
#include <QOpenGLBuffer>

struct MeshComponent : Component {
   static inline auto Name = u"Mesh"_s;

   using Component::Component;

   // we use structure of arrays instead of array of structures

   std::vector<QVector3D> vertices;
   std::vector<QVector2D> uvs;
   std::vector<QVector3D> normals;
   std::vector<uint16_t> indices;

   QJsonObject toJson() const override {
      REG_ASSERT(ComponentsRegistry<MeshComponent>::ComponentTypeRegistered);
      QJsonObject json;

      // vertices
      QJsonArray verticesArray;
      for (auto& vertex: vertices) {
         QJsonObject vertexObj;
         vertexObj["position"] = QJsonArray{vertex.x(), vertex.y(), vertex.z()};
         verticesArray.append(vertexObj);
      }
      json["vertices"] = verticesArray;

      // uvs
      QJsonArray uvsArray;
      for (auto& uv: uvs) { uvsArray.append(QJsonArray{uv.x(), uv.y()}); }
      json["uvs"] = uvsArray;

      // normals
      QJsonArray normalsArray;
      for (auto& normal: normals) {
         normalsArray.append(QJsonArray{normal.x(), normal.y(), normal.z()});
      }
      json["normals"] = normalsArray;

      // indices
      QJsonArray indicesArray;
      for (auto& index: indices) { indicesArray.append(index); }
      json["indices"] = indicesArray;

      return json;
   }

   void fromJson(const QJsonObject& json) override {
      auto verticesArray = json["vertices"].toArray();
      for (auto vertex: verticesArray) {
         auto vertexObj = vertex.toObject();
         auto position = vertexObj["position"].toArray();
         vertices.push_back(QVector3D(position[0].toDouble(), position[1].toDouble(),
                                      position[2].toDouble()));
      }

      auto uvsArray = json["uvs"].toArray();
      for (auto uv: uvsArray) {
         auto uvArray = uv.toArray();
         uvs.push_back(QVector2D(uvArray[0].toDouble(), uvArray[1].toDouble()));
      }

      auto normalsArray = json["normals"].toArray();
      for (auto normal: normalsArray) {
         auto normalArray = normal.toArray();
         normals.push_back(QVector3D(normalArray[0].toDouble(), normalArray[1].toDouble(),
                                     normalArray[2].toDouble()));
      }

      auto indicesArray = json["indices"].toArray();
      for (auto index: indicesArray) { indices.push_back(index.toInt()); }
   }

   void prepare(QOpenGLShaderProgram* program) override {
      if (m_vertexBuffer == nullptr || isDirty()) {
         delete m_vertexBuffer;
         if (!vertices.empty()) {
            m_vertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
            m_vertexBuffer->create();
            m_vertexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
            m_vertexBuffer->bind();
            m_vertexBuffer->allocate(vertices.data(), vertices.size() * sizeof(QVector3D));
         }
      }

      if (m_uvBuffer == nullptr || isDirty()) {
         delete m_uvBuffer;
         if (!uvs.empty()) {
            m_uvBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
            m_uvBuffer->create();
            m_uvBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
            m_uvBuffer->bind();
            m_uvBuffer->allocate(uvs.data(), uvs.size() * sizeof(QVector2D));
         }
      }

      if (m_normalBuffer == nullptr || isDirty()) {
         delete m_normalBuffer;
         if (!normals.empty()) {
            m_normalBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
            m_normalBuffer->create();
            m_normalBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
            m_normalBuffer->bind();
            m_normalBuffer->allocate(normals.data(), normals.size() * sizeof(QVector3D));
         }
      }

      if (m_indexBuffer == nullptr || isDirty()) {
         delete m_indexBuffer;
         if (!indices.empty()) {
            m_indexBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
            m_indexBuffer->create();
            m_indexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
            m_indexBuffer->bind();
            m_indexBuffer->allocate(indices.data(), indices.size() * sizeof(uint16_t));
         }
      }

      clean();
   }

   void bind(QOpenGLShaderProgram* program) override {
      if (m_vertexBuffer) {
         m_vertexBuffer->bind();
         program->enableAttributeArray("worldPos");
         program->setAttributeBuffer("worldPos", GL_FLOAT, 0, 3, sizeof(QVector3D));
      } else { program->disableAttributeArray("worldPos"); }

      if (m_uvBuffer) {
         m_uvBuffer->bind();
         program->enableAttributeArray("worldUV");
         program->setAttributeBuffer("worldUV", GL_FLOAT, 0, 2, sizeof(QVector2D));
      } else { program->disableAttributeArray("worldUV"); }

      if (m_normalBuffer) {
         m_normalBuffer->bind();
         program->enableAttributeArray("worldNormal");
         program->setAttributeBuffer("worldNormal", GL_FLOAT, 0, 3, sizeof(QVector3D));
      } else { program->disableAttributeArray("worldNormal"); }

      if (m_indexBuffer) { m_indexBuffer->bind(); }
   }

   void release(QOpenGLShaderProgram* program) override {
      if (m_vertexBuffer) m_vertexBuffer->release();
      if (m_uvBuffer) m_uvBuffer->release();
      if (m_normalBuffer) m_normalBuffer->release();
      if (m_indexBuffer) m_indexBuffer->release();
   }

   auto asTuple() { return std::tie(vertices, uvs, normals, indices); }

   auto asTuple() const { return std::tie(vertices, uvs, normals, indices); }

   ~MeshComponent() override {
      delete m_vertexBuffer;
      delete m_uvBuffer;
      delete m_normalBuffer;
      delete m_indexBuffer;
   }

private:
   QOpenGLBuffer* m_vertexBuffer = nullptr;
   QOpenGLBuffer* m_uvBuffer = nullptr;
   QOpenGLBuffer* m_normalBuffer = nullptr;
   QOpenGLBuffer* m_indexBuffer = nullptr;
};

using primitive_t = std::tuple<
   std::vector<QVector3D>,
   std::vector<QVector2D>,
   std::vector<uint16_t> >;

using primitive_normals_t = std::tuple<
   std::vector<QVector3D>,
   std::vector<QVector2D>,
   std::vector<QVector3D>,
   std::vector<uint16_t> >;

static primitive_t cube_primitive_data = {
      {
            // positions
            {-1, -1, -1},
            {-1, 1, -1},
            {1, 1, -1},
            {1, -1, -1},// front
            {1, -1, 1},
            {1, 1, 1},
            {-1, 1, 1},
            {-1, -1, 1},// back
            {-1, -1, 1},
            {-1, 1, 1},
            {-1, 1, -1},
            {-1, -1, -1},// left
            {1, -1, -1},
            {1, 1, -1},
            {1, 1, 1},
            {1, -1, 1},// right
            {-1, 1, -1},
            {-1, 1, 1},
            {1, 1, 1},
            {1, 1, -1},// top
            {-1, -1, 1},
            {-1, -1, -1},
            {1, -1, -1},
            {1, -1, 1},
      },// bottom
      {
            // uvs
            {0, 0},
            {0, 1},
            {1, 1},
            {1, 0},// front
            {0, 0},
            {0, 1},
            {1, 1},
            {1, 0},// back
            {0, 1},
            {1, 1},
            {1, 0},
            {0, 0},// left
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1},// right
            {0, 0},
            {0, 1},
            {1, 1},
            {1, 0},// top
            {1, 1},
            {1, 0},
            {0, 0},
            {0, 1}// bottom
      },
      {
            // indices
            0, 1, 2, 2, 3, 0,// front
            4, 5, 6, 6, 7, 4,// back
            8, 9, 10, 10, 11, 8,// left
            12, 13, 14, 14, 15, 12,// right
            16, 17, 18, 18, 19, 16,// top
            20, 21, 22, 22, 23, 20,// bottom
      }};

static primitive_t pyramid_primitive_data = {
      {
            // position
            // bottom
            {1, -1, -1},
            {1, -1, 1},
            {-1, -1, 1},
            {-1, -1, -1},

            // front
            {-1, -1, -1},
            {1, -1, -1},
            {0, 1, 0},

            // right
            {1, -1, -1},
            {1, -1, 1},
            {0, 1, 0},

            // back
            {1, -1, 1},
            {-1, -1, 1},
            {0, 1, 0},

            // left
            {-1, -1, 1},
            {-1, -1, -1},
            {0, 1, 0},
      },
      {
            // uvs
            {0, 0},
            {0, 1},
            {1, 1},
            {1, 0},
            {0, 0},
            {0, 1},
            {.5f, 1},
            {0, 0},
            {0, 1},
            {.5f, 1},
            {0, 0},
            {0, 1},
            {.5f, 1},
            {0, 0},
            {0, 1},
            {.5f, 1},
      },
      {
            // indices
            0, 1, 2, 2, 3, 0,// bottom
            6, 5, 4,// front
            9, 8, 7,// right
            12, 11, 10,// back
            15, 14, 13,// left
      }};

static inline primitive_normals_t to_normals(primitive_t data) {
   primitive_normals_t result;
   // copy vertices and uvs
   std::get<0>(result) = std::move(std::get<0>(data));
   std::get<1>(result) = std::move(std::get<1>(data));
   std::get<3>(result) = std::move(std::get<2>(data));

   // generate normals
   std::vector<QVector3D> normals;
   for (size_t i = 0; i < std::get<0>(data).size(); i += 3) {
      auto v1 = std::get<0>(data)[i + 1] - std::get<0>(data)[i];
      auto v2 = std::get<0>(data)[i + 2] - std::get<0>(data)[i];
      auto normal = QVector3D::crossProduct(v1, v2).normalized();
      normals.push_back(normal);
      normals.push_back(normal);
      normals.push_back(normal);
   }
   std::get<2>(result) = std::move(normals);

   return result;
}

static std::unordered_map<QString, primitive_normals_t, QtHasher<QString> > primitives = {
      {"cube", to_normals(cube_primitive_data)},
      {"pyramid", to_normals(pyramid_primitive_data)}};