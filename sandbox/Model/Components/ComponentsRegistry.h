#pragma once
#include "Common/Common.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <map>

class Object;

struct GlobalComponentsRegistry {
   using deletor_fn = std::function<void(QUuid)>;
   using serialize_fn = std::function<QJsonArray()>;
   using object_getter_fn = std::function<Object*(QUuid)>;
   using deserialize_fn = std::function<void(QJsonArray, object_getter_fn)>;
   using serialize_map = std::unordered_map<QString, serialize_fn, QtHasher<QString>>;
   using deserialize_map = std::unordered_map<QString, deserialize_fn, QtHasher<QString>>;

   static std::vector<deletor_fn>& Deletors() {
      static std::vector<deletor_fn> s_deletors;
      return s_deletors;
   }

   static serialize_map& Serializers() {
      static serialize_map s_serializers;
      return s_serializers;
   }

   static deserialize_map& Deserializers() {
      static deserialize_map s_deserializers;
      return s_deserializers;
   }

   static QJsonObject ToJson() {
      QJsonObject obj;
      for (const auto& [name, fn]: Serializers()) {
         obj[name] = fn();
      }
      return obj;
   }

   static void FromJson(QJsonObject obj, const object_getter_fn& getter) {
      for (const auto& [name, fn]: Deserializers()) {
         fn(obj[name].toArray(), getter);
      }
   }
};

template<typename T>
struct ComponentsRegistry {
   static std::unordered_map<QUuid, T, QtHasher<QUuid>>& Components();

   static inline bool ComponentTypeRegistered = []() {
      GlobalComponentsRegistry::Deletors().push_back([](QUuid id) {
         Components().erase(id);
      });
      GlobalComponentsRegistry::Serializers()[T::Name] = []() {
         QJsonArray arr;
         for (const auto& [id, component]: Components()) {
            QJsonObject obj;
            obj["id"] = id.toString();
            obj["data"] = component.toJson();
            arr.append(obj);
         }
         return arr;
      };
      GlobalComponentsRegistry::Deserializers()[T::Name] = [](QJsonArray arr, const GlobalComponentsRegistry::object_getter_fn& getter) {
         for (const auto& obj: arr) {
            auto id = QUuid::fromString(obj.toObject()["id"].toString());
            auto data = obj.toObject()["data"].toObject();
            Components().emplace(id, getter(id));
            Components().at(id).fromJson(data);
         }
      };
      return true;
   }();
};

template<typename T>
std::unordered_map<QUuid, T, QtHasher<QUuid>>& ComponentsRegistry<T>::Components() {
   static std::unordered_map<QUuid, T, QtHasher<QUuid>> s_components;
   Q_ASSERT(ComponentTypeRegistered);
   return s_components;
}
