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
      {{{-1, -1, -1}, {0, 0}},
       {{-1, -1, 1}, {0, 1}},
       {{-1, 1, -1}, {1, 0}},
       {{-1, 1, 1}, {1, 1}},
       {{1, -1, -1}, {0, 0}},
       {{1, -1, 1}, {0, 1}},
       {{1, 1, -1}, {1, 0}},
       {{1, 1, 1}, {1, 1}}},
      {0, 1, 2, 3, 7, 5, 1, 3, 6, 7, 4, 5, 0, 2, 6, 4, 2, 0, 1, 5, 7, 6, 3, 4}};


static std::unordered_map<QString, std::pair<std::vector<VertexData>, std::vector<uint16_t>>, QtHasher<QString>> primitives = {
      {"cube", cube_primitive_data},
};