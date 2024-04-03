#pragma once
#include "Common/Common.h"
#include "Model/Hierarchy/Object.h"
#include <QJsonObject>

struct Component {
   explicit Component(Object* parent) noexcept : m_parent(parent) {}
   Component(const Component& other) noexcept = default;
   Component(Component&& other) noexcept = default;
   Component& operator=(const Component& other) noexcept = default;
   Component& operator=(Component&& other) noexcept = default;

   bool standalone() const { return m_parent == nullptr; }
   const Object& parent() const { return *m_parent; }
   Object& parent() { return *m_parent; }

   virtual QJsonObject toJson() const = 0;
   virtual void fromJson(const QJsonObject& json) = 0;

private:
   Object* m_parent = nullptr;
};
