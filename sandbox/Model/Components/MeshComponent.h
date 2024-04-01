#pragma once

#include "Common/Common.h"
#include "Component.h"
#include <QVector3D>
#include <tuple>
#include <unordered_map>

struct VertexData {
   QVector3D position;
   QVector2D uv;

   bool operator==(const VertexData& other) const {
      return std::tie(position, uv) == std::tie(other.position, other.uv);
   }
};

struct MeshComponent : Component {
   static inline auto Name = u"Mesh"_s;

   using Component::Component;

   std::vector<VertexData> vertices;
   std::vector<uint16_t> indices;

   QJsonObject toJson() const {
      QJsonObject json;
      QJsonArray verticesArray;
      for (const auto& vertex: vertices) {
         QJsonObject vertexObj;
         vertexObj["position"] = QJsonArray{vertex.position.x(), vertex.position.y(), vertex.position.z()};
         vertexObj["uv"] = QJsonArray{vertex.uv.x(), vertex.uv.y()};
         verticesArray.append(vertexObj);
      }
      json["vertices"] = verticesArray;
      QJsonArray indicesArray;
      for (const auto& index: indices) {
         indicesArray.append(index);
      }
      json["indices"] = indicesArray;
      return json;
   }

   void fromJson(const QJsonObject& json) {
      vertices.clear();
      auto verticesArray = json["vertices"].toArray();
      for (const auto& vertex: verticesArray) {
         auto vertexObj = vertex.toObject();
         VertexData vertexData;
         auto pos = vertexObj["position"].toArray();
         vertexData.position = QVector3D(pos[0].toDouble(), pos[1].toDouble(), pos[2].toDouble());
         auto uv = vertexObj["uv"].toArray();
         vertexData.uv = QVector2D(uv[0].toDouble(), uv[1].toDouble());
         vertices.push_back(vertexData);
      }
      indices.clear();
      auto indicesArray = json["indices"].toArray();
      for (const auto& index: indicesArray) {
         indices.push_back(index.toInt());
      }
   }
};

static std::pair<std::vector<VertexData>, std::vector<uint16_t>> cube_primitive_data = {
      {
            // Vertex data for face 0
            {QVector3D(-0.5, -0.5, 0.5), QVector2D(0.0f, 0.0f)},// v0
            {QVector3D(0.5, -0.5, 0.5), QVector2D(0.33f, 0.0f)},// v1
            {QVector3D(-0.5, 0.5, 0.5), QVector2D(0.0f, 0.5f)}, // v2
            {QVector3D(0.5, 0.5, 0.5), QVector2D(0.33f, 0.5f)}, // v3

            // Vertex 0.5ata for face 1
            {QVector3D(0.5, -0.5, 0.5), QVector2D(0.0f, 0.5f)},  // v4
            {QVector3D(0.5, -0.5, -0.5), QVector2D(0.33f, 0.5f)},// v5
            {QVector3D(0.5, 0.5, 0.5), QVector2D(0.0f, 1.0f)},   // v6
            {QVector3D(0.5, 0.5, -0.5), QVector2D(0.33f, 1.0f)}, // v7

            // Vertex 0.5ata for face 2
            {QVector3D(0.5, -0.5, -0.5), QVector2D(0.66f, 0.5f)},// v8
            {QVector3D(-0.5, -0.5, -0.5), QVector2D(1.0f, 0.5f)},// v9
            {QVector3D(0.5, 0.5, -0.5), QVector2D(0.66f, 1.0f)}, // v10
            {QVector3D(-0.5, 0.5, -0.5), QVector2D(1.0f, 1.0f)}, // v11

            // Vertex 0.5ata for face 3
            {QVector3D(-0.5, -0.5, -0.5), QVector2D(0.66f, 0.0f)},// v12
            {QVector3D(-0.5, -0.5, 0.5), QVector2D(1.0f, 0.0f)},  // v13
            {QVector3D(-0.5, 0.5, -0.5), QVector2D(0.66f, 0.5f)}, // v14
            {QVector3D(-0.5, 0.5, 0.5), QVector2D(1.0f, 0.5f)},   // v15

            // Vertex 0.5ata for face 4
            {QVector3D(-0.5, -0.5, -0.5), QVector2D(0.33f, 0.0f)},// v16
            {QVector3D(0.5, -0.5, -0.5), QVector2D(0.66f, 0.0f)}, // v17
            {QVector3D(-0.5, -0.5, 0.5), QVector2D(0.33f, 0.5f)}, // v18
            {QVector3D(0.5, -0.5, 0.5), QVector2D(0.66f, 0.5f)},  // v19

            // Vertex 0.5ata for face 5
            {QVector3D(-0.5, 0.5, 0.5), QVector2D(0.33f, 0.5f)}, // v20
            {QVector3D(0.5, 0.5, 0.5), QVector2D(0.66f, 0.5f)},  // v21
            {QVector3D(-0.5, 0.5, -0.5), QVector2D(0.33f, 1.0f)},// v22
            {QVector3D(0.5, 0.5, -0.5), QVector2D(0.66f, 1.0f)}  // v23
      },
      {
            0, 1, 2, 3, 3,         // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
            4, 4, 5, 6, 7, 7,      // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
            8, 8, 9, 10, 11, 11,   // Face 2 - triangle strip ( v8,  v9, v10, v11)
            12, 12, 13, 14, 15, 15,// Face 3 - triangle strip (v12, v13, v14, v15)
            16, 16, 17, 18, 19, 19,// Face 4 - triangle strip (v16, v17, v18, v19)
            20, 20, 21, 22, 23     // Face 5 - triangle strip (v20, v21, v22, v23)
      }};

static std::pair<std::vector<VertexData>, std::vector<uint16_t>> pyramid_primitive_data = {
      {
            // Vertex data for the base of the pyramid
            {QVector3D(-0.5f, -0.5f, -0.5f), QVector2D(0.0f, 0.0f)}, // v0
            {QVector3D(0.5f, -0.5f, -0.5f), QVector2D(1.0f, 0.0f)},  // v1
            {QVector3D(0.5f, -0.5f, 0.5f), QVector2D(1.0f, 1.0f)},   // v2
            {QVector3D(-0.5f, -0.5f, 0.5f), QVector2D(0.0f, 1.0f)},  // v3

            // Vertex data for the sides of the pyramid
            {QVector3D(0.0f, 0.5f, 0.0f), QVector2D(0.5f, 0.5f)}     // v4 (apex)
      },
      {
            // Base of the pyramid (triangle fan)
            0, 1, 2, 3,

            // Side triangles
            0, 4, 1,
            1, 4, 2,
            2, 4, 3,
            3, 4, 0
      }
};



static std::unordered_map<QString, std::pair<std::vector<VertexData>, std::vector<uint16_t>>, QtHasher<QString>> primitives = {
      {"cube", cube_primitive_data},
      {"pyramid", pyramid_primitive_data}
};