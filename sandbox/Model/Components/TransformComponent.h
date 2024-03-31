#pragma once
#include "Component.h"
#include "Model/Hierarchy/Object.h"
#include <QQuaternion>
#include <QVector3D>

struct TransformComponent : Component {
   static inline QString Name = "Transform";
   using Component::Component;

   QVector3D position;
   QQuaternion rotation;

   QJsonObject toJson() const override {
      QJsonObject json;
      json["position"] = QJsonArray{position.x(), position.y(), position.z()};
      json["rotation"] = QJsonArray{rotation.scalar(), rotation.x(), rotation.y(), rotation.z()};
      return json;
   }

   void fromJson(const QJsonObject& json) override {
      auto pos = json["position"].toArray();
      position = QVector3D(pos[0].toDouble(), pos[1].toDouble(), pos[2].toDouble());
      auto rot = json["rotation"].toArray();
      rotation = QQuaternion(rot[0].toDouble(), rot[1].toDouble(), rot[2].toDouble(), rot[3].toDouble());
   }
};
