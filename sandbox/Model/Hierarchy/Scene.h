#pragma once

#include "Common/Common.h"
#include <QObject>
#include <QJsonObject>
#include <vector>
#include <memory>
#include "Object.h"

class Scene : public QObject, public std::enable_shared_from_this<Scene> {
   Q_OBJECT

public:
   static uptr<Scene> createEmpty();
   static uptr<Scene> createFromJson(const QJsonObject& json);
   QJsonObject toJson() const;

   void addObject(uptr<Object> obj);
   void removeObject(Object& obj);

   std::optional<const Object*> findObject(const QString& name) const;
   std::optional<Object*> findObject(const QString& name);

signals:
   void objectAdded(const Object& obj);
   void objectRemoved(const Object& obj);
   void objectsChanged();

private:
   Scene() = default;

private:
   std::vector<uptr<Object>> m_objects;

};