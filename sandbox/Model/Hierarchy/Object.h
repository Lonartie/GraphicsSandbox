#pragma once

#include "Common/Common.h"
#include <QObject>
#include <QJsonObject>
#include <QUuid>

class Object : public QObject, std::enable_shared_from_this<Object> {
   Q_OBJECT

public:
   static uptr<Object> create(const QString& name = "<unnamed>");
   static uptr<Object> createFromJson(const QJsonObject& json);
   QJsonObject toJson() const;

   const QUuid& id() const;

   void setName(const QString& name);
   const QString& name() const;

signals:
   void nameChanged(const QString& name);

private:
   Object() = default;

private:
   QUuid m_id = QUuid::createUuid();
   QString m_name;
};
