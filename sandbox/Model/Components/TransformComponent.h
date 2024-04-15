#pragma once
#include "Component.h"
#include "Model/Hierarchy/Object.h"
#include "ComponentsRegistry.h"
#include <QQuaternion>
#include <QVector3D>
#include <QDebug>
#include <QDataStream>

struct TransformComponent : Component {
   static inline QString Name = "Transform";
   using Component::Component;

   QVector3D position = QVector3D(0, 0, 0);
   QQuaternion rotation = QQuaternion::fromEulerAngles(0, 0, 0);
   QVector3D scale = QVector3D(1, 1, 1);

   QJsonObject toJson() const override {
      REG_ASSERT(ComponentsRegistry<TransformComponent>::ComponentTypeRegistered);
      QJsonObject json;
      json["position"] = QJsonArray{position.x(), position.y(), position.z()};
      json["rotation"] = QJsonArray{rotation.scalar(), rotation.x(), rotation.y(), rotation.z()};
      json["scale"] = QJsonArray{scale.x(), scale.y(), scale.z()};
      return json;
   }

   void fromJson(const QJsonObject& json) override {
      auto pos = json["position"].toArray();
      position = QVector3D(pos[0].toDouble(), pos[1].toDouble(), pos[2].toDouble());
      auto rot = json["rotation"].toArray();
      rotation = QQuaternion(rot[0].toDouble(), rot[1].toDouble(), rot[2].toDouble(),
                             rot[3].toDouble());
      auto sca = json["scale"].toArray();
      scale = QVector3D(sca[0].toDouble(), sca[1].toDouble(), sca[2].toDouble());
   }

   QMatrix4x4 modelMatrix() const {
      static QMatrix4x4 defaultMatrix;
      defaultMatrix.translate(QVector3D());
      defaultMatrix.rotate(QQuaternion());
      defaultMatrix.scale(QVector3D(1, 1, 1));

      QMatrix4x4 parentModel = hasParent() && parent().parent()
                                  ? (*parent().parent())->getComponent<TransformComponent>().
                                  modelMatrix()
                                  : defaultMatrix;

      QMatrix4x4 model;
      model.translate(position);
      model.rotate(rotation);
      model.scale(scale);

      return parentModel * model;
   }

   TransformComponent toGlobal() const { return FromMatrix(modelMatrix()); }

   TransformComponent fromGlobal(const TransformComponent& global) const {
      if (!hasParent() || !parent().parent()) return global;
      auto parentModel = (*parent().parent())->getComponent<TransformComponent>().modelMatrix();
      auto globalMatrix = global.modelMatrix();
      auto localMatrix = parentModel.inverted() * globalMatrix;
      return FromMatrix(localMatrix);
   }

   static TransformComponent FromMatrix(const QMatrix4x4& matrix) {
      // decompose global matrix
      QVector3D pos, scale;
      QQuaternion rot;
      pos = matrix.column(3).toVector3D();
      scale.setX(matrix.column(0).toVector3D().length());
      scale.setY(matrix.column(1).toVector3D().length());
      scale.setZ(matrix.column(2).toVector3D().length());
      QMatrix3x3 rotMat;
      rotMat(0, 0) = matrix(0, 0) / scale.x();
      rotMat(0, 1) = matrix(0, 1) / scale.y();
      rotMat(0, 2) = matrix(0, 2) / scale.z();
      rotMat(1, 0) = matrix(1, 0) / scale.x();
      rotMat(1, 1) = matrix(1, 1) / scale.y();
      rotMat(1, 2) = matrix(1, 2) / scale.z();
      rotMat(2, 0) = matrix(2, 0) / scale.x();
      rotMat(2, 1) = matrix(2, 1) / scale.y();
      rotMat(2, 2) = matrix(2, 2) / scale.z();
      rot = QQuaternion::fromRotationMatrix(rotMat);

      TransformComponent result(nullptr);
      result.position = pos;
      result.rotation = rot;
      result.scale = scale;

      return result;
   }
};

inline QDataStream& operator<<(QDataStream& stream, const TransformComponent& transform) {
   stream << transform.position << transform.rotation << transform.scale;
   return stream;
}

inline QDataStream& operator>>(QDataStream& stream, TransformComponent& transform) {
   stream >> transform.position >> transform.rotation >> transform.scale;
   return stream;
}

inline QDebug& operator<<(QDebug& dbg, const TransformComponent& transform) {
   dbg << "TransformComponent(" << transform.position << ", " << transform.rotation << ", " <<
         transform.scale << ")";
   return dbg;
}

Q_DECLARE_METATYPE(TransformComponent)