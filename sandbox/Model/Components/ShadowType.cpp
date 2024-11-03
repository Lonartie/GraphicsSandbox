#include "ShadowType.h"

const ShadowType ShadowType::None = ShadowType(ShadowType::NoShadows, .0f);
const ShadowType ShadowType::Hard = ShadowType(ShadowType::HardShadows, .0f);
const ShadowType ShadowType::SoftNormal = ShadowType(ShadowType::SoftShadows, 1.f);

ShadowType::ShadowType() {}

ShadowType::ShadowType(Type type, float softness) {
   this->type = type;
   this->softness = softness;
}

ShadowType::ShadowType(const ShadowType& other) {
   this->type = other.type;
   this->softness = other.softness;
}

ShadowType& ShadowType::operator=(const ShadowType& other) {
   this->type = other.type;
   this->softness = other.softness;
   return *this;
}