#pragma once

#include <memory>
#include <QHash>

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
