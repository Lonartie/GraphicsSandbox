#pragma once
#include <compare>
#include <QMetaType>
#include <QMetaEnum>

struct ShadowType : QObject {

   Q_OBJECT
   Q_PROPERTY(Type type MEMBER type)
   Q_PROPERTY(float softness MEMBER softness)

public:
   enum Type {
      NoShadows,
      HardShadows,
      SoftShadows
   };

   Q_ENUM(Type)

   Type type = NoShadows;
   float softness = .0f;

   ShadowType();

   ShadowType(Type type, float softness);

   ShadowType(const ShadowType& other);

   ShadowType& operator=(const ShadowType& other);

   static const ShadowType None;
   static const ShadowType Hard;
   static const ShadowType SoftNormal;
};

Q_DECLARE_METATYPE(ShadowType::Type);

Q_DECLARE_METATYPE(ShadowType);