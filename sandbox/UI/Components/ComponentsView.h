#pragma once
#include "Model/Model.h"
#include "UI/ObjectEditor/ObjectEditor.h"
#include <QWidget>
#include <concepts>

class ComponentsViewBase {
public:
   using creator_fn = std::function<ComponentsViewBase*(ObjectEditor*, Object*)>;
   using add_component_fn = std::function<void(Object*)>;
   using remove_component_fn = std::function<void(Object*)>;
   using order_fn = std::function<int()>;

   static std::unordered_map<QString, creator_fn, QtHasher<QString>>& Views() {
      static std::unordered_map<QString, creator_fn, QtHasher<QString>> sViews;
      return sViews;
   }

   static std::unordered_map<QString, order_fn, QtHasher<QString>>& Order() {
      static std::unordered_map<QString, order_fn, QtHasher<QString>> sOrder;
      return sOrder;
   }

   static std::vector<QString> SortedNames() {
      std::vector<QString> names;
      for (const auto& [name, _] : Order()) {
         names.push_back(name);
      }
      std::sort(names.begin(), names.end(), [](const QString& a, const QString& b) {
         return Order()[a]() < Order()[b]();
      });
      return names;
   }

   static std::unordered_map<QString, add_component_fn, QtHasher<QString>>& AddComponent() {
      static std::unordered_map<QString, add_component_fn, QtHasher<QString>> sAddComponent;
      return sAddComponent;
   }

   static std::unordered_map<QString, remove_component_fn, QtHasher<QString>>& RemoveComponent() {
      static std::unordered_map<QString, remove_component_fn, QtHasher<QString>> sRemoveComponent;
      return sRemoveComponent;
   }

   virtual ~ComponentsViewBase() = default;
   virtual QWidget* asWidget() = 0;
   virtual void init() = 0;

   void setObject(Object* obj) { m_obj = obj; }

protected:
   Object* m_obj = nullptr;
};

template <typename T>
concept has_order_field = requires  { {T::Order} -> std::convertible_to<int>; };

template<typename V, typename M>
struct ComponentsViewRegistrar {
   static bool registerView() {
      ComponentsViewBase::creator_fn creator = [](ObjectEditor* parent, Object* obj) -> ComponentsViewBase* {
         if (!obj->hasComponent<M>()) return nullptr;
         auto view = new V(nullptr);
         view->setObject(obj);
         view->init();
         view->connect(view, &V::objectChanged, parent, &ObjectEditor::objectChanged);
         return view;
      };
      ComponentsViewBase::add_component_fn addComponent = [](Object* obj) {
         obj->addComponent<M>();
      };
      ComponentsViewBase::remove_component_fn removeComponent = [](Object* obj) {
         obj->removeComponent<M>();
      };
      ComponentsViewBase::order_fn order = []() -> int {
         if constexpr (has_order_field<V>) {
            return V::Order;
         } else {
            return std::numeric_limits<int>::max();
         }
      };
      QString name = M::Name;
      ComponentsViewBase::Views().emplace(name, std::move(creator));
      ComponentsViewBase::AddComponent().emplace(name, std::move(addComponent));
      ComponentsViewBase::Order().emplace(name, std::move(order));
      ComponentsViewBase::RemoveComponent().emplace(name, std::move(removeComponent));
      return true;
   }
};

template<typename V, typename M>
struct ComponentsView: public ComponentsViewBase {
   static bool Registered;
};

template <typename V, typename M>
bool ComponentsView<V, M>::Registered = ComponentsViewRegistrar<V, M>::registerView();
