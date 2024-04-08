#pragma once
#include "Common/Common.h"
#include <QJsonObject>
#include <QObject>
#include <QUuid>

class Scene;

class Object : public QObject, std::enable_shared_from_this<Object> {
   Q_OBJECT

public:
   friend class Scene;

   virtual ~Object() override;

   static uptr<Object> create(Scene& parent);
   static uptr<Object> createFromJson(const QJsonObject& json, Scene& scene);
   QJsonObject toJson() const;

   const QUuid& id() const;

   void setName(const QString& name);
   const QString& name() const;

   void setParent(Object& parent);
   void unsetParent();
   std::optional<Object*> parent() const;
   std::vector<Object*> children() const;
   Scene* scene() const;

   uint64_t order() const;
   void setOrder(uint64_t order);

   bool enabled() const;
   void enable(bool enable);

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
   bool m_enabled = true;
   uint64_t m_order = 0;
   Scene* m_parent = nullptr;
};
