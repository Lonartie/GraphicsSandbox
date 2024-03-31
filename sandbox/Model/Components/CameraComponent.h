#pragma once
#include "Component.h"
#include <QRectF>
#include <QColor>

struct CameraComponent : Component {
   static inline QString Name = "Camera";
   using Component::Component;

   QRectF viewport = QRectF(0,0,1,1); // shall be between 0 and 1
   float fov = 45.0f;
   float nearClip = 0.1f;
   float farClip = 1000.0f;
   QColor backgroundColor = QColor(0,0,0);

   QJsonObject toJson() const override {
      QJsonObject json;
      json["viewport"] = QJsonArray{viewport.x(), viewport.y(), viewport.width(), viewport.height()};
      json["fov"] = fov;
      json["nearClip"] = nearClip;
      json["farClip"] = farClip;
      json["backgroundColor"] = backgroundColor.name(QColor::NameFormat::HexRgb);
      return json;
   }

   void fromJson(const QJsonObject& json) override {
      auto vp = json["viewport"].toArray();
      viewport = QRectF(vp[0].toDouble(), vp[1].toDouble(), vp[2].toDouble(), vp[3].toDouble());
      fov = json["fov"].toDouble();
      nearClip = json["nearClip"].toDouble();
      farClip = json["farClip"].toDouble();
      backgroundColor = QColor(json["backgroundColor"].toString());
   }
};