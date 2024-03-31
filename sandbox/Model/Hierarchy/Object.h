#pragma once
#include "Common/Common.h"
#include "Model/Components/ComponentsRegistry.h"
#include <QJsonObject>
#include <QObject>
#include <QUuid>

class Scene;

class Object : public QObject, std::enable_shared_from_this<Object> {
   Q_OBJECT

public:
   virtual ~Object() override;

   static uptr<Object> create();
   static uptr<Object> createFromJson(const QJsonObject& json);
   QJsonObject toJson() const;

   const QUuid& id() const;

   void setName(const QString& name);
   const QString& name() const;

   template <typename T> T& getComponent();
   template <typename T> const T& getComponent() const;
   template <typename T> T& addComponent();
   template <typename T> void removeComponent();
   template <typename T> bool hasComponent() const;

protected:
   Object() = default;

private:
   QUuid m_id = QUuid::createUuid();
   QString m_name = "<unnamed>";
};

template<typename T>
T& Object::getComponent() {
   return ComponentsRegistry<T>::Components().at(m_id);
}

template<typename T>
const T& Object::getComponent() const {
   return ComponentsRegistry<T>::Components().at(m_id);
}

template<typename T>
T& Object::addComponent() {
   return ComponentsRegistry<T>::Components().emplace(m_id, this).first->second;
}

template<typename T>
void Object::removeComponent() {
   ComponentsRegistry<T>::Components().erase(m_id);
}

template<typename T>
bool Object::hasComponent() const {
   return ComponentsRegistry<T>::Components().find(m_id) != ComponentsRegistry<T>::Components().end();
}
