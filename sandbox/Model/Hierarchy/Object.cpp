#include "Object.h"

uptr<Object> Object::create(const QString& name) {
   uptr<Object> obj(new Object());
   obj->setName(name);
   return obj;
}

uptr<Object> Object::createFromJson(const QJsonObject& json) {
   uptr<Object> obj(new Object());
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
   emit nameChanged(name);
}

const QString& Object::name() const {
   return m_name;
}