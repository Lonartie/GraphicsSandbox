#pragma once
#include "Component.h"
#include <QColor>

struct MaterialComponent : Component {
   static inline auto Name = u"Material"_s;

   using Component::Component;

   QColor solidColor;

   QJsonObject toJson() const override {
      QJsonObject json;
      json["solidColor"] = solidColor.name();
      return json;
   }

   void fromJson(const QJsonObject& json) override {
      solidColor = QColor(json["solidColor"].toString());
   }
};