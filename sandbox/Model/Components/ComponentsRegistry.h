#pragma once
#include "Common/Common.h"
#include "Model/Hierarchy/Object.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <map>

struct GlobalComponentsRegistry {
   using deletor_fn = std::function<void(std::function<sptr<void>(QString)>, QUuid)>;
   using copier_fn = std::function<void(const Object*, Object*, sptr<void>)>;
   using serialize_fn = std::function<QJsonArray(sptr<void>)>;
   using object_getter_fn = std::function<Object*(QUuid)>;
   using deserialize_fn = std::function<void(std::function<void(QString, sptr<void>)>, QJsonArray,
                                             object_getter_fn)>;
   using serialize_map = std::unordered_map<QString, serialize_fn, QtHasher<QString> >;
   using deserialize_map = std::unordered_map<QString, deserialize_fn, QtHasher<QString> >;
   using copier_map = std::unordered_map<QString, copier_fn, QtHasher<QString> >;

   static std::vector<deletor_fn>& Deletors() {
      static std::vector<deletor_fn> s_deletors;
      return s_deletors;
   }

   static copier_map& Copiers() {
      static copier_map s_copiers;
      return s_copiers;
   }

   static serialize_map& Serializers() {
      static serialize_map s_serializers;
      return s_serializers;
   }

   static deserialize_map& Deserializers() {
      static deserialize_map s_deserializers;
      return s_deserializers;
   }

   static QJsonObject ToJson(std::function<sptr<void>(QString)> regGetter) {
      QJsonObject obj;
      for (const auto& [name, fn]: Serializers()) { obj[name] = fn(regGetter(name)); }
      return obj;
   }

   static void FromJson(std::function<void(QString, sptr<void>)> regSetter, QJsonObject obj,
                        const object_getter_fn& getter) {
      for (const auto& [name, fn]: Deserializers()) { fn(regSetter, obj[name].toArray(), getter); }
   }
};

template<typename T>
bool create_deletor();

template<typename T>
bool create_serializer();

template<typename T>
bool create_deserializer();

template<typename T>
bool create_copier();

template<typename T>
struct ComponentsRegistry {
   std::unordered_map<QUuid, T, QtHasher<QUuid> >& components() { return m_components; }

   static inline bool ComponentTypeRegistered =
         create_deletor<T>() &&
         create_serializer<T>() &&
         create_deserializer<T>() &&
         create_copier<T>();

private:
   std::unordered_map<QUuid, T, QtHasher<QUuid> > m_components;
};


template<typename T>
bool create_deletor() {
   GlobalComponentsRegistry::Deletors().push_back(
         [](std::function<sptr<void>(QString)> getter, QUuid id) {
            auto rawReg = getter(T::Name);
            if (!rawReg) return;
            auto reg = std::static_pointer_cast<ComponentsRegistry<T> >(rawReg);
            reg->components().erase(id);
         });
   return true;
}

template<typename T>
bool create_serializer() {
   GlobalComponentsRegistry::Serializers()[T::Name] = [](sptr<void> o) {
      auto reg = std::static_pointer_cast<ComponentsRegistry<T> >(o);
      QJsonArray arr;
      for (const auto& [id, component]: reg->components()) {
         QJsonObject obj;
         obj["id"] = id.toString();
         obj["data"] = component.toJson();
         arr.append(obj);
      }
      return arr;
   };
   return true;
}

template<typename T>
bool create_deserializer() {
   GlobalComponentsRegistry::Deserializers()[T::Name] = [
         ](std::function<void(QString, sptr<void>)> regSetter, QJsonArray arr,
           const GlobalComponentsRegistry::object_getter_fn& getter) {
            auto reg = std::make_shared<ComponentsRegistry<T> >();
            for (const auto& obj: arr) {
               auto id = QUuid::fromString(obj.toObject()["id"].toString());
               auto data = obj.toObject()["data"].toObject();
               reg->components().emplace(id, getter(id));
               reg->components().at(id).fromJson(data);
            }
            regSetter(T::Name, reg);
         };
   return true;
}

template<typename T>
bool create_copier() {
   GlobalComponentsRegistry::Copiers()[T::Name] = [](const Object* from, Object* to, sptr<void> o) {
      auto reg = std::static_pointer_cast<ComponentsRegistry<T> >(o);
      if (!reg->components().contains(from->id())) return;
      auto& component = reg->components().at(from->id());
      auto serialized = component.toJson();
      reg->components().emplace(to->id(), to);
      reg->components().at(to->id()).fromJson(serialized);
   };
   return true;
}
