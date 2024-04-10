#pragma once
#include "Common/Common.h"
#include "Model/Hierarchy/Object.h"
#include <QJsonObject>
#include <QOpenGLShaderProgram>
#include <QJsonArray>

struct Component {
   explicit Component(Object* parent) noexcept : m_parent(parent) {}
   Component(const Component& other) noexcept = default;
   Component(Component&& other) noexcept = default;
   Component& operator=(const Component& other) noexcept = default;
   Component& operator=(Component&& other) noexcept = default;
   virtual ~Component() = default;

   const Object& parent() const { return *m_parent; }
   Object& parent() { return *m_parent; }
   bool hasParent() const { return m_parent != nullptr; }

   virtual QJsonObject toJson() const = 0;
   virtual void fromJson(const QJsonObject& json) = 0;
   virtual void prepare(QOpenGLShaderProgram*) {}
   virtual void bind(QOpenGLShaderProgram*) {}
   virtual void release(QOpenGLShaderProgram*) {}

   void dirty() { m_dirty = true; }
   bool isDirty() const { return m_dirty; }

protected:
   void clean() { m_dirty = false; }

private:
   Object* m_parent = nullptr;
   bool m_dirty = false;
};
