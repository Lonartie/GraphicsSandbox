#include "Object.h"
#include "Scene.h"
#include "Model/Components/TransformComponent.h"
#include "Model/Components/ComponentsRegistry.h"

uptr<Object> Object::create(Scene& scene) {
   auto obj = uptr<Object>(new Object());
   obj->m_parent = &scene;
   obj->addComponent<TransformComponent>();
   return obj;
}

uptr<Object> Object::createFromJson(const QJsonObject& json, Scene& scene) {
   auto obj = uptr<Object>(new Object());
   // components will be deserialized later
   obj->m_parent = &scene;
   obj->m_id = QUuid::fromString(json["id"].toString());
   obj->setName(json["name"].toString());
   obj->enable(json["enabled"].toBool(true));
   obj->setOrder(json["order"].toVariant().toULongLong());
   return obj;
}

QJsonObject Object::toJson() const {
   QJsonObject json;
   json["id"] = m_id.toString();
   json["name"] = m_name;
   json["enabled"] = m_enabled;
   json["order"] = QVariant::fromValue(m_order).toJsonObject();
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

void Object::setParent(Object& parent) {
   m_parent->addChild(parent, *this);
}

void Object::unsetParent() {
   if (auto parent = m_parent->parentOf(*this)) {
      m_parent->removeChild(**parent, *this);
   }
}

std::optional<Object*> Object::parent() const {
   return m_parent->parentOf(*this);
}

std::vector<Object*> Object::children() const {
   return m_parent->childrenOf(*this);
}

Scene* Object::scene() const {
   return m_parent;
}

uint64_t Object::order() const {
   return m_order;
}

void Object::setOrder(uint64_t order) {
   m_order = order;
}

Object::~Object() {
   // remove parent child associations
   unsetParent();

   // remove all components associated with this object
   if (m_parent) {
      m_parent->unregister(this);
   }
}

bool Object::enabled() const {
   auto pt = parent();
   return m_enabled && (!pt.has_value() || (*pt)->enabled());
}

void Object::enable(bool enable) {
   m_enabled = enable;
   for (auto* child : children()) {
      child->enable(enable);
   }
}
