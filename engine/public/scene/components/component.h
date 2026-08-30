#pragma once

#include "stdafx.h"

namespace golias {

    class GameObject;

    class Component {

    public:
        virtual ~Component() = default;

        virtual size_t GetTypeId() const        = 0;
        virtual const char* GetTypeName() const = 0;

        virtual bool LoadProperties(const Json& properties);

        virtual void Start();

        virtual void Update(float deltaTime) = 0;

        virtual void OnEnable();

        virtual void OnDisable();

        GameObject* GetOwner() const;

        template <typename T>
        static size_t StaticTypeId() {
            static size_t typeId = sNextComponentID++;
            return typeId;
        }

    protected:
        Component() = default;

        friend class GameObject;

    private:
        static size_t sNextComponentID;

        GameObject* mOwner = nullptr;
    };

    class ComponentFactoryBase {
    public:
        virtual ~ComponentFactoryBase() = default;

        virtual Component* Create() const = 0;
    };

    template <typename T>
    class ComponentFactory : public ComponentFactoryBase {
    public:
        virtual Component* Create() const override {
            return new T();
        }
    };


    class ComponentRegistry {
    public:
        static ComponentRegistry& GetInstance();

        template <typename T>
        void RegisterComponent(CString name) {
            mCreators.emplace(name.data(), std::make_unique<ComponentFactory<T>>());
            mParents[T::TypeId()].push_back(Component::StaticTypeId<Component>());
        }

        template <typename T, typename ParentType>
        void RegisterComponent(CString name) {
            mCreators.emplace(name.data(), std::make_unique<ComponentFactory<T>>());
            mParents[T::TypeId()].push_back(Component::StaticTypeId<ParentType>());
        }

        Component* CreateComponent(CString name) const;

        bool HasParent(size_t objTypeId, size_t parentTypeId) const;

    private:
        std::unordered_map<std::string, std::unique_ptr<ComponentFactoryBase>> mCreators;
        std::unordered_map<size_t, std::vector<size_t>> mParents;
    };


#define COMPONENT(ClazzName)                                                       \
public:                                                                            \
    static size_t TypeId() {                                                       \
        return Component::StaticTypeId<ClazzName>();                               \
    }                                                                              \
    size_t GetTypeId() const override {                                            \
        return TypeId();                                                           \
    }                                                                              \
    const char* GetTypeName() const override {                                     \
        return #ClazzName;                                                         \
    }                                                                              \
    static void Register() {                                                       \
        ComponentRegistry::GetInstance().RegisterComponent<ClazzName>(#ClazzName); \
    }

#define COMPONENT_DERIVED(Clazz, ParentType)                                           \
public:                                                                                \
    static size_t TypeId() {                                                           \
        return Component::StaticTypeId<Clazz>();                                       \
    }                                                                                  \
    size_t GetTypeId() const override {                                                \
        return TypeId();                                                               \
    }                                                                                  \
    const char* GetTypeName() const override {                                         \
        return #Clazz;                                                                 \
    }                                                                                  \
    static void Register() {                                                           \
        ComponentRegistry::GetInstance().RegisterComponent<Clazz, ParentType>(#Clazz); \
    }

} // namespace golias
