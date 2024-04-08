#include "Scene.h"
#include "Object.h"
#include "Model/Components/ComponentsRegistry.h"
#include <QJsonArray>
#include <ranges>
#include <unordered_set>

uptr<Scene> Scene::createEmpty() {
   return uptr<Scene>(new Scene());
}

uptr<Scene> Scene::createFromJson(const QJsonObject& json) {
   uptr<Scene> scene(new Scene());
   std::function objectGetter = [scene = scene.get()](QUuid id) -> Object* {
      return scene->findObject(id).value();
   };

   for (const auto& obj: json["objects"].toArray()) {
      scene->addObject(Object::createFromJson(obj.toObject(), *scene));
   }
   auto regSetter = [scene = scene.get()](QString name, sptr<void> o) {
      scene->m_componentsRegistrar[name] = std::move(o);
   };
   auto childrenAssoc = json["children"].toObject();
   for (const auto& parent: childrenAssoc.keys()) {
      auto parentID = QUuid::fromString(parent);
      auto children = childrenAssoc[parent].toArray();
      for (const auto& child: children) {
         auto childID = QUuid::fromString(child.toString());
         scene->m_children[parentID].push_back(childID);
      }
   }

   GlobalComponentsRegistry::FromJson(regSetter, json["components"].toObject(), objectGetter);

   return scene;
}

QJsonObject Scene::toJson() const {
   auto getter = [this](QString name) {
      return m_componentsRegistrar.at(name);
   };

   QJsonObject json;
   QJsonArray objects;
   for (const auto& obj: m_objects) {
      objects.append(obj->toJson());
   }
   json["objects"] = objects;
   json["components"] = GlobalComponentsRegistry::ToJson(getter);
   QJsonObject childrenObject;
   for (const auto& [parent, children]: m_children) {
      QJsonArray childrenArray;
      for (const auto& child: children) {
         childrenArray.append(child.toString());
      }
      childrenObject[parent.toString()] = childrenArray;
   }
   json["children"] = childrenObject;
   return json;
}

void Scene::addObject(uptr<Object> obj) {
   m_objects.push_back(std::move(obj));
   m_objects.back()->m_parent = this;
}

void Scene::removeObject(Object& obj) {
   for (auto* child: std::vector(obj.children())) {
      this->removeObject(*child);
   }
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

std::optional<const Object*> Scene::findObject(QUuid id) const {
   auto it = std::find_if(m_objects.begin(), m_objects.end(), [&id](const uptr<Object>& obj) {
      return obj->id() == id;
   });

   if (it != m_objects.end()) {
      return it->get();
   }

   return std::nullopt;
}

std::optional<Object*> Scene::findObject(QUuid id) {
   auto it = std::ranges::find_if(m_objects, [&id](const uptr<Object>& obj) {
      return obj && obj->id() == id;
   });

   if (it != m_objects.end()) {
      return it->get();
   }

   return std::nullopt;
}

std::vector<const Object*> Scene::objects() const {
   std::vector<const Object*> objs;
   for (const auto& obj: m_objects) {
      objs.push_back(obj.get());
   }
   return objs;
}

std::vector<Object*> Scene::objects() {
   std::vector<Object*> objs;
   for (const auto& obj: m_objects) {
      objs.push_back(obj.get());
   }
   return objs;
}

void Scene::unregister(Object* obj) {
   auto getter = [this](QString name) {
      if (m_componentsRegistrar.find(name) == m_componentsRegistrar.end()) {
         return sptr<void>();
      }
      return m_componentsRegistrar.at(name);
   };
   for (auto& deletor: GlobalComponentsRegistry::Deletors()) {
      deletor(getter, obj->id());
   }
}

Scene::~Scene() {
   // first clear objects !!!
   m_objects.clear();

   // only afterwards clear registry!!!
   m_componentsRegistrar.clear();
}

Object& Scene::copyObject(const Object& obj) {
   // intentionally without this parameter to avoid creating a transform component
   // since we manually copy each component later
   auto newObj = Object::create(*this);
   newObj->m_id = QUuid::createUuid();
   newObj->setName(obj.name() + " (copy)");
   newObj->enable(obj.enabled());
   auto id = newObj->m_id;

   for (auto& [name, copier]: GlobalComponentsRegistry::Copiers()) {
      if (!m_componentsRegistrar.contains(name)) continue;

      auto compReg = m_componentsRegistrar.at(name);
      copier(&obj, newObj.get(), compReg);
   }

   addObject(std::move(newObj));
   return *findObject(id).value();
}

void Scene::addChild(Object& parent, Object& child) {
   if (auto oldParent = parentOf(child))
      removeChild(**oldParent, child);

   m_children[parent.id()].push_back(child.id());
}

void Scene::removeChild(Object& parent, Object& child) {
   auto iter = std::ranges::find(m_children[parent.id()], child.id());
   if (iter != m_children[parent.id()].end()) {
      m_children[parent.id()].erase(iter);
   }
}

std::optional<Object*> Scene::parentOf(const Object& child) {
   auto it = std::ranges::find_if(m_children, [&child](const auto& pair) {
      return std::ranges::find(pair.second, child.id()) != pair.second.end();
   });
   if (it == m_children.end()) return std::nullopt;
   auto id = it->first;
   return findObject(id);
}

std::vector<Object*> Scene::childrenOf(const Object& parent) {
   std::vector<Object*> children;
   if (m_children.find(parent.id()) == m_children.end()) return children;
   for (const auto& id: m_children[parent.id()]) {
      if (auto child = findObject(id)) {
         children.push_back(child.value());
      }
   }
   return children;
}

std::vector<Object*> Scene::allChildrenOf(const Object& parent) {
   std::vector<Object*> children = childrenOf(parent);
   std::vector<Object*> newChildren = children;
   while (!newChildren.empty()) {
      std::vector<Object*> newChildlrenCopy = newChildren;
      newChildren.clear();

      for (const auto& child: newChildlrenCopy) {
         auto newChildrenOfChild = childrenOf(*child);
         children.insert(children.end(), newChildrenOfChild.begin(), newChildrenOfChild.end());
         newChildren.insert(newChildren.end(), newChildrenOfChild.begin(), newChildrenOfChild.end());
      }
   }

   // remove duplicates
   std::unordered_set uniqueChildren(children.begin(), children.end());
   children.assign(uniqueChildren.begin(), uniqueChildren.end());
   return children;
}

TransformComponent Scene::toRelativeTransformOf(Object& obj) {
   TransformComponent transform(&obj); // <- this is an unregistered component
   transform.fromJson(obj.getComponent<TransformComponent>().toJson());
   Object* current = &obj;
   while (auto parent = current->parent()) {
      auto& parentTransform = (*parent)->getComponent<TransformComponent>();
      auto& childTransform = transform;
      childTransform.position = childTransform.position - parentTransform.position;
      // childTransform.rotation = childTransform.rotation - parentTransform.rotation;
      // childTransform.scale = childTransform.scale / parentTransform.scale;
      current = *parent;
   }

   return transform;
}

TransformComponent Scene::toGlobalTransformOf(Object& obj) {
   TransformComponent transform(&obj); // <- this is an unregistered component
   transform.fromJson(obj.getComponent<TransformComponent>().toJson());
   Object* current = &obj;
   while (auto parent = current->parent()) {
      auto& parentTransform = (*parent)->getComponent<TransformComponent>();
      auto& childTransform = transform;
      childTransform.position = childTransform.position + parentTransform.position;
      // childTransform.rotation = childTransform.rotation + parentTransform.rotation;
      // childTransform.scale = childTransform.scale * parentTransform.scale;
      current = *parent;
   }

   return transform;
}

void Scene::updateRelativeTransformOf(Object& obj, const TransformComponent& newRelative) {
   auto oldRelative = toRelativeTransformOf(obj);
   auto positionDiff = newRelative.position - oldRelative.position;
   auto rotationDiff = newRelative.rotation - oldRelative.rotation;
   auto scaleDiff = newRelative.scale - oldRelative.scale;

   auto& transform = obj.getComponent<TransformComponent>();
   transform.position = transform.position + positionDiff;
   transform.rotation = transform.rotation + rotationDiff;
   transform.scale = transform.scale + scaleDiff;

   for (auto* child: obj.children()) {
      TransformComponent oldRelativeChild = toRelativeTransformOf(*child);
      TransformComponent newRelativeChild = oldRelativeChild;
      newRelativeChild.position = oldRelativeChild.position + positionDiff;
      // newRelativeChild.rotation = oldRelativeChild.rotation + rotationDiff;
      // newRelativeChild.scale = oldRelativeChild.scale + scaleDiff;
      updateRelativeTransformOf(*child, newRelativeChild);
   }
}
