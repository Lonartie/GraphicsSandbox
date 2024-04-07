#pragma once

#include "Common/Common.h"
#include "Model/Components/TransformComponent.h"
#include <QJsonObject>
#include <QObject>
#include <memory>
#include <vector>
#include "Model/Components/ComponentsRegistry.h"

class Object;

class Scene : public QObject, public std::enable_shared_from_this<Scene> {
   Q_OBJECT

public:
   ~Scene();

   static uptr<Scene> createEmpty();
   static uptr<Scene> createFromJson(const QJsonObject& json);
   QJsonObject toJson() const;

   void addObject(uptr<Object> obj);
   void removeObject(Object& obj);
   void copyObject(const Object& obj);

   std::optional<const Object*> findObject(const QString& name) const;
   std::optional<Object*> findObject(const QString& name);
   std::optional<const Object*> findObject(QUuid id) const;
   std::optional<Object*> findObject(QUuid id);

   std::vector<const Object*> objects() const;
   std::vector<Object*> objects();

   template <typename T> T& getComponent(Object* obj);
   template <typename T> const T& getComponent(const Object* obj);
   template <typename T> T& addComponent(Object* obj);
   template <typename T> void removeComponent(Object* obj);
   template <typename T> bool hasComponent(const Object* obj);

   template <typename T> std::unordered_map<QUuid, T, QtHasher<QUuid>>& components();

   void unregister(Object* obj);

signals:
   void objectAdded(const Object& obj);
   void objectRemoved(const Object& obj);
   void objectsChanged();

private:
   Scene() = default;

private:
   std::vector<uptr<Object>> m_objects;
   std::unordered_map<QString, sptr<void>, QtHasher<QString>> m_componentsRegistrar;
};

template<typename T>
T& Object::getComponent() {
   return m_parent->getComponent<T>(this);
}

template<typename T>
const T& Object::getComponent() const {
   return m_parent->getComponent<T>(this);
}

template<typename T>
T& Object::addComponent() {
   return m_parent->addComponent<T>(this);
}

template<typename T>
void Object::removeComponent() {
   m_parent->removeComponent<T>(this);
}

template<typename T>
bool Object::hasComponent() const {
   return m_parent->hasComponent<T>(this);
}

template<typename T>
T& Scene::getComponent(Object* obj) {
   auto name = T::Name;
   if (!m_componentsRegistrar.contains(name)) {
      m_componentsRegistrar[name] = std::make_shared<ComponentsRegistry<T>>();
   }

   auto registry = std::static_pointer_cast<ComponentsRegistry<T>>(m_componentsRegistrar.at(name));
   return registry->components().at(obj->id());
}

template<typename T>
const T& Scene::getComponent(const Object* obj) {
   auto name = T::Name;
   if (!m_componentsRegistrar.contains(name)) {
      m_componentsRegistrar[name] = std::make_shared<ComponentsRegistry<T>>();
   }

   auto registry = std::static_pointer_cast<ComponentsRegistry<T>>(m_componentsRegistrar.at(name));
   return registry->components().at(obj->id());
}

template<typename T>
T& Scene::addComponent(Object* obj) {
   auto name = T::Name;
   if (!m_componentsRegistrar.contains(name)) {
      m_componentsRegistrar[name] = std::make_shared<ComponentsRegistry<T>>();
   }

   auto registry = std::static_pointer_cast<ComponentsRegistry<T>>(m_componentsRegistrar.at(name));
   return registry->components().emplace(obj->id(), obj).first->second;
}

template<typename T>
void Scene::removeComponent(Object* obj) {
   auto name = T::Name;
   if (!m_componentsRegistrar.contains(name)) {
      return;
   }

   auto registry = std::static_pointer_cast<ComponentsRegistry<T>>(m_componentsRegistrar.at(name));
   registry->components().erase(obj->id());
}

template<typename T>
bool Scene::hasComponent(const Object* obj) {
   auto name = T::Name;
   if (!m_componentsRegistrar.contains(name)) {
      return false;
   }

   auto registry = std::static_pointer_cast<ComponentsRegistry<T>>(m_componentsRegistrar.at(name));
   return registry->components().contains(obj->id());
}

template <typename T>
std::unordered_map<QUuid, T, QtHasher<QUuid>>& Scene::components() {
   auto name = T::Name;
   if (!m_componentsRegistrar.contains(name)) {
      m_componentsRegistrar[name] = std::make_shared<ComponentsRegistry<T>>();
   }

   auto registry = std::static_pointer_cast<ComponentsRegistry<T>>(m_componentsRegistrar.at(name));
   return registry->components();
}

