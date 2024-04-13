#pragma once

#include <memory>
#include <QBuffer>
#include <QHash>
#include <QString>
#include <QVariant>
#include <QImage>

using namespace Qt::Literals::StringLiterals;

template <typename T>
using sptr = std::shared_ptr<T>;

template <typename T>
using uptr = std::unique_ptr<T>;

template <typename T>
using wptr = std::weak_ptr<T>;

template <typename T>
struct QtHasher {
   std::size_t operator()(const T& id) const {
      return qHash(id);
   }
};

template<>
struct QtHasher<QVariant> {
   std::size_t operator()(const QVariant& id) const {
      QByteArray arr;
      QBuffer buffer(&arr);
      buffer.open(QIODevice::WriteOnly);
      QDataStream stream(&buffer);
      stream << id;
      return qHash(arr);
   }
};

template<typename T>
struct is_tuple_t : std::false_type {};

template<typename... Ts>
struct is_tuple_t<std::tuple<Ts...>> : std::true_type {};

template<typename T>
concept is_tuple = is_tuple_t<T>::value;

#define REG_ASSERT(X) if (!(X)) { std::exit(666); }
