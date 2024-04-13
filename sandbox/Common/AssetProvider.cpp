#include "AssetProvider.h"

namespace {
   QtHasher<QVariant> hasher = {};
   uint64_t ID = 1;

   QByteArray variantToByteArray(const QVariant& variant) {
      QByteArray arr;
      QBuffer buffer(&arr);
      buffer.open(QIODevice::WriteOnly);
      QDataStream stream(&buffer);
      stream << variant;
      return arr;
   }

   QVariant byteArrayToVariant(const QByteArray& arr) {
      QVariant variant;
      QBuffer buffer(const_cast<QByteArray*>(&arr));
      buffer.open(QIODevice::ReadOnly);
      QDataStream stream(&buffer);
      stream >> variant;
      return variant;
   }

   QString variantToString(const QVariant& variant) {
      auto str = QString::fromUtf8(variantToByteArray(variant).toBase64());
      return str;
   }

   QVariant stringToVariant(const QString& json) {
      return byteArrayToVariant(QByteArray::fromBase64(json.toUtf8()));
   }
}

AssetProvider& AssetProvider::instance() {
   static AssetProvider inst;
   return inst;
}

uint64_t AssetProvider::add(QVariant asset) {
   if (const auto it = m_assets.find(asset); it != m_assets.end()) {
      return it->second;
   }
   return m_assets.emplace(std::move(asset), ID++).first->second;
}

const QVariant& AssetProvider::get(uint64_t id) {
   const auto it = std::ranges::find_if(m_assets, [id](const auto& pair) {
      return pair.second == id;
   });
   if (it == m_assets.end()) {
      static QVariant empty;
      return empty;
   }
   return it->first;
}

bool AssetProvider::has(uint64_t id) const {
   return std::ranges::find_if(m_assets, [id](const auto& pair) {
      return pair.second == id;
   }) != m_assets.end();
}

QJsonObject AssetProvider::toJson() const {
   QJsonObject result;
   for (const auto& [asset, id]: m_assets) {
      result[QString::number(id)] = variantToString(asset);
   }
   return result;
}

void AssetProvider::fromJson(const QJsonObject& json) {
   for (const auto& key: json.keys()) {
      auto id = key.toULongLong();
      if (id >= ID) ID = id + 1;
      m_assets.emplace(stringToVariant(json[key].toString()), id);
   }
}

bool AssetProvider::QVariantComparator::operator()(const QVariant& lhs, const QVariant& rhs) const {
   if (lhs.metaType().id() != rhs.metaType().id()) {
      return lhs.metaType().id() < rhs.metaType().id();
   }

   // QImage optimization
   if (lhs.metaType().id() == QMetaType::fromType<QImage>().id()) {
      auto* lhsImage = get_if<QImage>(&lhs);
      auto* rhsImage = get_if<QImage>(&rhs);
      if (lhsImage && rhsImage) {
         return lhsImage->cacheKey() < rhsImage->cacheKey();
      }
   }

   // general case
   return variantToByteArray(lhs) < variantToByteArray(rhs);
}
