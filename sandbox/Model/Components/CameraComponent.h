#pragma once
#include "Component.h"
#include "TransformComponent.h"
#include "ComponentsRegistry.h"
#include <QColor>
#include <QRectF>
#include <array>

struct CameraComponent : Component {
   static inline auto Name = u"Camera"_s;

   using Component::Component;

   QRectF viewport = QRectF(0, 0, 1, 1);// shall be between 0 and 1
   float fov = 45.0f;
   float nearClip = 0.1f;
   float farClip = 1000.0f;
   bool wireframe = false;
   QColor backgroundColor = QColor(0, 0, 0);

   QJsonObject toJson() const override {
      REG_ASSERT(ComponentsRegistry<CameraComponent>::ComponentTypeRegistered);
      QJsonObject json;
      json["viewport"] = QJsonArray{viewport.x(), viewport.y(), viewport.width(),
                                    viewport.height()};
      json["fov"] = fov;
      json["nearClip"] = nearClip;
      json["farClip"] = farClip;
      json["backgroundColor"] = backgroundColor.name(QColor::NameFormat::HexRgb);
      json["wireframe"] = wireframe;
      return json;
   }

   void fromJson(const QJsonObject& json) override {
      auto vp = json["viewport"].toArray();
      viewport = QRectF(vp[0].toDouble(), vp[1].toDouble(), vp[2].toDouble(), vp[3].toDouble());
      fov = json["fov"].toDouble();
      nearClip = json["nearClip"].toDouble();
      farClip = json["farClip"].toDouble();
      backgroundColor = QColor(json["backgroundColor"].toString());
      wireframe = json["wireframe"].toBool();
   }

   QMatrix4x4 projectionMatrix(float aspectRatio) const {
      QMatrix4x4 projection;
      projection.perspective(fov, aspectRatio, nearClip, farClip);
      return projection;
   }

   QMatrix4x4 viewMatrix(const TransformComponent& transform) const {
      QMatrix4x4 view;
      view.translate(0, 0, 0);
      view.rotate(transform.rotation);
      view.translate(transform.position);
      return view;
   }
};