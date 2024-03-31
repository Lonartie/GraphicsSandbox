#pragma once
#include "Model/Model.h"
#include "UI/ObjectEditor/ObjectEditor.h"
#include <QWidget>

class ComponentsViewBase {
public:
   using creator_fn = std::function<ComponentsViewBase*(ObjectEditor*, Object*)>;
   using add_component_fn = std::function<void(Object*)>;

   static std::unordered_map<QString, creator_fn, QtHasher<QString>>& Views() {
      static std::unordered_map<QString, creator_fn, QtHasher<QString>> sViews;
      return sViews;
   }

   static std::unordered_map<QString, add_component_fn, QtHasher<QString>>& AddComponent() {
      static std::unordered_map<QString, add_component_fn, QtHasher<QString>> sAddComponent;
      return sAddComponent;
   }

   virtual ~ComponentsViewBase() = default;
   virtual QWidget* asWidget() = 0;
   virtual void init() = 0;

   void setObject(Object* obj) { m_obj = obj; }

protected:
   Object* m_obj = nullptr;
};

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
      QString name = M::Name;
      ComponentsViewBase::Views().emplace(name, std::move(creator));
      ComponentsViewBase::AddComponent().emplace(name, std::move(addComponent));
      return true;
   }
};

template<typename V, typename M>
class ComponentsView: public ComponentsViewBase {
   static inline bool Registered = ComponentsViewRegistrar<V, M>::registerView();
};
