#pragma once
#include "Common.h"
#include <QJsonObject>
#include <QUuid>
#include <QHash>
#include <memory>
#include <set>
#include <QOpenGLTexture>

class AssetProvider {
public:
   static AssetProvider& instance();

public:
   uint64_t add(QVariant asset);
   template<typename T>
   uint64_t add(T asset);

   const QVariant& get(uint64_t id);
   template<typename T> const T& get(uint64_t id);

   template <typename T> void prepare(uint64_t id);
   template <typename T> void bind(uint64_t id, int unit = 0);
   template <typename T> void unbind(uint64_t id);

   bool has(uint64_t id) const;

   QJsonObject toJson() const;
   void fromJson(const QJsonObject& json);

private:
   AssetProvider() = default;

private:
   struct QVariantComparator {
      bool operator()(const QVariant& lhs, const QVariant& rhs) const;
   };

   std::map<QVariant, uint64_t, QVariantComparator> m_assets;
   std::map<uint64_t, sptr<void>> m_buffers;
};

template<typename T>
uint64_t AssetProvider::add(T asset) {
   return add(QVariant::fromValue(std::move(asset)));
}

template<typename T>
const T& AssetProvider::get(uint64_t id) {
   auto& variant = get(id);
   auto& asset = *get_if<T>(&variant);
   return asset;
}

template<>
inline void AssetProvider::prepare<QImage>(uint64_t id) {
   if (m_buffers.contains(id)) return;
   const auto& image = get<QImage>(id);
   auto buffer = std::make_shared<QOpenGLTexture>(image.mirrored());
   buffer->setMinificationFilter(QOpenGLTexture::Linear);
   buffer->setMagnificationFilter(QOpenGLTexture::Linear);
   buffer->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::ClampToEdge);
   buffer->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::ClampToEdge);
   m_buffers[id] = std::move(buffer);
}

template<>
inline void AssetProvider::bind<QImage>(uint64_t id, int unit) {
   if (m_buffers.contains(id)) {
      const auto buffer = std::static_pointer_cast<QOpenGLTexture>(m_buffers[id]);
      buffer->bind(unit);
   }
}

template<>
inline void AssetProvider::unbind<QImage>(uint64_t id) {
   if (m_buffers.contains(id)) {
      const auto buffer = std::static_pointer_cast<QOpenGLTexture>(m_buffers[id]);
      buffer->release();
   }
}
