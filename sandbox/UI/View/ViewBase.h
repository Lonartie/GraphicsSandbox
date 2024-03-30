#pragma once

#include <QWidget>
#include <functional>

class ViewBase {
public:
   virtual ~ViewBase() = default;

   using ViewCreator = std::function<ViewBase*(QWidget*)>;

   static std::map<QString, ViewCreator>& getCreators() {
      static std::map<QString, ViewCreator> creators;
      return creators;
   }

   static std::vector<QString> getViewNames() {
      std::vector<QString> names;
      for (const auto& [name, _] : getCreators()) {
         names.push_back(name);
      }
      return names;
   }

   [[nodiscard]] virtual QWidget* asWidget() = 0;
};

template<typename T>
class AutoRegisterView {
public:
   static volatile inline bool registered = []() {
      ViewBase::ViewCreator creator = [](QWidget* parent){ return new T(parent); };
      QString name = T::Name;
      ViewBase::getCreators().emplace(name, std::move(creator));
      return true;
   }();
};