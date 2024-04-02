#pragma once
#include "Component.h"

struct MaterialComponent : Component {
   static inline auto Name = u"Material"_s;

   using Component::Component;
};