#include "Scene.h"
#include "Object.h"
#include "Model/Components/ComponentsRegistry.h"
#include <QJsonArray>
#include <ranges>
#include <unordered_set>

#include "Common/AssetProvider.h"

namespace {
   QMatrix4x4 toMatrix(QVector3D pos, QQuaternion rot, QVector3D scale) {
      QMatrix4x4 matrix;
      matrix.translate(pos);
      matrix.rotate(rot);
      matrix.scale(scale);
      return matrix;
   }

   QMatrix4x4 toMatrix(const TransformComponent& transform) {
      return toMatrix(transform.position, transform.rotation, transform.scale);
   }

   std::tuple<QVector3D, QQuaternion, QVector3D> fromMatrix(const QMatrix4x4& matrix) {
      QVector3D pos, scale;
      QQuaternion rot;

      pos = matrix.column(3).toVector3D();
      scale.setX(matrix.column(0).toVector3D().length());
      scale.setY(matrix.column(1).toVector3D().length());
      scale.setZ(matrix.column(2).toVector3D().length());
      QMatrix3x3 rotMat;
      rotMat(0, 0) = matrix(0, 0) / scale.x();
      rotMat(0, 1) = matrix(0, 1) / scale.y();
      rotMat(0, 2) = matrix(0, 2) / scale.z();
      rotMat(1, 0) = matrix(1, 0) / scale.x();
      rotMat(1, 1) = matrix(1, 1) / scale.y();
      rotMat(1, 2) = matrix(1, 2) / scale.z();
      rotMat(2, 0) = matrix(2, 0) / scale.x();
      rotMat(2, 1) = matrix(2, 1) / scale.y();
      rotMat(2, 2) = matrix(2, 2) / scale.z();
      rot = QQuaternion::fromRotationMatrix(rotMat);
      return {pos, rot, scale};
   }
}

uptr<Scene> Scene::createEmpty() { return uptr<Scene>(new Scene()); }

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

   qDebug() << "There are " << GlobalComponentsRegistry::Deserializers().size() << " deserializers";
   GlobalComponentsRegistry::FromJson(regSetter, json["components"].toObject(), objectGetter);
   AssetProvider::instance().fromJson(json["assets"].toObject());

   return scene;
}

QJsonObject Scene::toJson() const {
   auto getter = [this](QString name) { return m_componentsRegistrar.at(name); };

   QJsonObject json;
   QJsonArray objects;
   for (const auto& obj: m_objects) { objects.append(obj->toJson()); }
   json["objects"] = objects;
   json["components"] = GlobalComponentsRegistry::ToJson(getter);
   QJsonObject childrenObject;
   for (const auto& [parent, children]: m_children) {
      QJsonArray childrenArray;
      for (const auto& child: children) { childrenArray.append(child.toString()); }
      childrenObject[parent.toString()] = childrenArray;
   }
   json["children"] = childrenObject;
   json["assets"] = AssetProvider::instance().toJson();
   return json;
}

void Scene::addObject(uptr<Object> obj) {
   m_objects.push_back(std::move(obj));
   m_objects.back()->m_parent = this;
}

void Scene::removeObject(Object& obj) {
   for (auto* child: std::vector(obj.children())) { this->removeObject(*child); }
   auto it = std::find_if(m_objects.begin(), m_objects.end(), [&obj](const uptr<Object>& o) {
      return o.get() == &obj;
   });

   if (it != m_objects.end()) { m_objects.erase(it); }
}

std::optional<const Object*> Scene::findObject(const QString& name) const {
   auto it = std::find_if(m_objects.begin(), m_objects.end(), [&name](const uptr<Object>& obj) {
      return obj->name() == name;
   });

   if (it != m_objects.end()) { return it->get(); }

   return std::nullopt;
}

std::optional<Object*> Scene::findObject(const QString& name) {
   auto it = std::find_if(m_objects.begin(), m_objects.end(), [&name](const uptr<Object>& obj) {
      return obj->name() == name;
   });

   if (it != m_objects.end()) { return it->get(); }

   return std::nullopt;
}

std::optional<const Object*> Scene::findObject(QUuid id) const {
   auto it = std::find_if(m_objects.begin(), m_objects.end(), [&id](const uptr<Object>& obj) {
      return obj->id() == id;
   });

   if (it != m_objects.end()) { return it->get(); }

   return std::nullopt;
}

std::optional<Object*> Scene::findObject(QUuid id) {
   auto it = std::ranges::find_if(m_objects, [&id](const uptr<Object>& obj) {
      return obj && obj->id() == id;
   });

   if (it != m_objects.end()) { return it->get(); }

   return std::nullopt;
}

std::vector<const Object*> Scene::objects() const {
   std::vector<const Object*> objs;
   for (const auto& obj: m_objects) { objs.push_back(obj.get()); }
   return objs;
}

std::vector<Object*> Scene::objects() {
   std::vector<Object*> objs;
   for (const auto& obj: m_objects) { objs.push_back(obj.get()); }
   return objs;
}

void Scene::unregister(Object* obj) {
   auto getter = [this](QString name) {
      if (m_componentsRegistrar.find(name) == m_componentsRegistrar.end()) { return sptr<void>(); }
      return m_componentsRegistrar.at(name);
   };
   for (auto& deletor: GlobalComponentsRegistry::Deletors()) { deletor(getter, obj->id()); }
}

Scene::~Scene() {
   // first clear objects !!!
   std::vector<uptr<Object> > tmp;
   m_objects.swap(tmp);
   tmp.clear();

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
   if (auto oldParent = parentOf(child)) removeChild(**oldParent, child);

   m_children[parent.id()].push_back(child.id());
}

void Scene::removeChild(Object& parent, Object& child) {
   auto iter = std::ranges::find(m_children[parent.id()], child.id());
   if (iter != m_children[parent.id()].end()) { m_children[parent.id()].erase(iter); }
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
      if (auto child = findObject(id)) { children.push_back(child.value()); }
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
         newChildren.insert(newChildren.end(), newChildrenOfChild.begin(),
                            newChildrenOfChild.end());
      }
   }

   // remove duplicates
   std::unordered_set uniqueChildren(children.begin(), children.end());
   children.assign(uniqueChildren.begin(), uniqueChildren.end());
   return children;
}
