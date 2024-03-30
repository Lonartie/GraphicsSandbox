#include "Scene.h"
#include <QJsonArray>

uptr<Scene> Scene::createEmpty() {
   return uptr<Scene>(new Scene());
}

uptr<Scene> Scene::createFromJson(const QJsonObject& json) {
   uptr<Scene> scene(new Scene());
   for (const auto& obj: json["objects"].toArray()) {
      scene->addObject(Object::createFromJson(obj.toObject()));
   }
   return scene;
}

QJsonObject Scene::toJson() const {
   QJsonObject json;
   QJsonArray objects;
   for (const auto& obj: m_objects) {
      objects.append(obj->toJson());
   }
   json["objects"] = objects;
   return json;
}

void Scene::addObject(uptr<Object> obj) {
   m_objects.push_back(std::move(obj));
}

void Scene::removeObject(Object& obj) {
   auto it = std::find_if(m_objects.begin(), m_objects.end(), [&obj](const uptr<Object>& o) {
      return o.get() == &obj;
   });

   if (it != m_objects.end()) {
      m_objects.erase(it);
   }
}

std::optional<const Object*> Scene::findObject(const QString& name) const {
   auto it = std::find_if(m_objects.begin(), m_objects.end(), [&name](const uptr<Object>& obj) {
      return obj->name() == name;
   });

   if (it != m_objects.end()) {
      return it->get();
   }

   return std::nullopt;
}

std::optional<Object*> Scene::findObject(const QString& name) {
   auto it = std::find_if(m_objects.begin(), m_objects.end(), [&name](const uptr<Object>& obj) {
      return obj->name() == name;
   });

   if (it != m_objects.end()) {
      return it->get();
   }

   return std::nullopt;
}
