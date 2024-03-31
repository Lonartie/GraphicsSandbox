#pragma once
#include "Component.h"
#include <QRectF>

struct CameraComponent : Component {
   static inline QString Name = "Camera";
   using Component::Component;

   QRectF m_viewport; // shall be between 0 and 1
   float m_fov = 45.0f;
   float m_nearClip = 0.1f;
   float m_farClip = 1000.0f;

   QJsonObject toJson() const override {
      QJsonObject json;
      json["viewport"] = QJsonArray{m_viewport.x(), m_viewport.y(), m_viewport.width(), m_viewport.height()};
      json["fov"] = m_fov;
      json["nearClip"] = m_nearClip;
      json["farClip"] = m_farClip;
      return json;
   }

   void fromJson(const QJsonObject& json) override {
      auto vp = json["viewport"].toArray();
      m_viewport = QRectF(vp[0].toDouble(), vp[1].toDouble(), vp[2].toDouble(), vp[3].toDouble());
      m_fov = json["fov"].toDouble();
      m_nearClip = json["nearClip"].toDouble();
      m_farClip = json["farClip"].toDouble();
   }
};