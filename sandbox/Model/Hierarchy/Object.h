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
   friend class Scene;

   virtual ~Object() override;

   static uptr<Object> create();
   static uptr<Object> create(Scene* parent);
   static uptr<Object> createFromJson(const QJsonObject& json);
   QJsonObject toJson() const;

   const QUuid& id() const;

   void setName(const QString& name);
   const QString& name() const;

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
   Scene* m_parent = nullptr;
};
