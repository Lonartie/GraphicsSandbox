#pragma once
#include "Component.h"
#include "ComponentsRegistry.h"
#include "ShadowType.h"

#include <QJsonObject>
#include <QColor>

struct DirectionalLightSourceComponent : Component {
   static inline auto Name = u"Directional light source"_s;
   using Component::Component;

   ~DirectionalLightSourceComponent() override = default;

   QJsonObject toJson() const override;

   void fromJson(const QJsonObject& json) override;

   QColor color = Qt::white;
   double intensity = 1.0;
   ShadowType shadowType = ShadowType::None;
};

inline QJsonObject DirectionalLightSourceComponent::toJson() const {
   REG_ASSERT(ComponentsRegistry<DirectionalLightSourceComponent>::ComponentTypeRegistered);
   QJsonObject object;
   object["color"] = QJsonArray(
         {color.red(), color.green(), color.blue(), color.alpha()}
         );
   object["intensity"] = intensity;
   object["shadowType"] = QJsonValue::fromVariant(QVariant::fromValue(shadowType));
   return object;
}

inline void DirectionalLightSourceComponent::fromJson(const QJsonObject& json) {
   REG_ASSERT(ComponentsRegistry<DirectionalLightSourceComponent>::ComponentTypeRegistered);
   QJsonArray colorArray = json["color"].toArray();
   color = QColor(colorArray[0].toInt(), colorArray[1].toInt(), colorArray[2].toInt(),
                  colorArray[3].toInt());
   intensity = json["intensity"].toDouble(1.0f);
   shadowType = json["shadowType"].toVariant().value<ShadowType>();
}