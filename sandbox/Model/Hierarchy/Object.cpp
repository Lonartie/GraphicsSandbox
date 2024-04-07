#include "Object.h"
#include "Scene.h"
#include "Model/Components/TransformComponent.h"

uptr<Object> Object::create() {
   auto obj = uptr<Object>(new Object());
//   obj->addComponent<TransformComponent>();
   return obj;
}

uptr<Object> Object::create(Scene* scene) {
   auto obj = uptr<Object>(new Object());
   obj->m_parent = scene;
   obj->addComponent<TransformComponent>();
   return obj;
}

uptr<Object> Object::createFromJson(const QJsonObject& json) {
   auto obj = uptr<Object>(new Object());
   // components will be deserialized later
   obj->m_id = QUuid::fromString(json["id"].toString());
   obj->setName(json["name"].toString());
   return obj;
}

QJsonObject Object::toJson() const {
   QJsonObject json;
   json["id"] = m_id.toString();
   json["name"] = m_name;
   return json;
}

const QUuid& Object::id() const {
   return m_id;
}

void Object::setName(const QString& name) {
   if (m_name == name) return;
   m_name = name;
}

const QString& Object::name() const {
   return m_name;
}

Object::~Object() {
   // remove all components associated with this object
   if (m_parent) {
      m_parent->unregister(this);
   }
}
