#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <json.hpp>

namespace golias {

    class GameObject;

    class Component {
    public:
        virtual ~Component()             = default;
        virtual size_t GetTypeId() const = 0;

        GameObject* GetOwner() const;
        void SetOwner(GameObject* pOwner);

        virtual void Start()                 = 0;
        virtual void Update(float deltaTime) = 0;

        template <typename T>
        static size_t StaticTypeId() {
            static size_t type_id = next_type_id++;
            return type_id;
        }

        virtual void LoadProperties(const nlohmann::json& json) = 0;
    protected:
        GameObject* owner = nullptr;
        friend class GameObject;

    private:
        static size_t next_type_id;
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
        static ComponentRegistry& GetInstance() {
            static ComponentRegistry instance;
            return instance;
        }

        template <typename T>
        void RegisterComponent(const std::string_view pName) {
            creators.emplace(pName.data(), std::make_unique<ComponentFactory<T>>());
        }

        Component* CreateComponent(const std::string_view pName) const {
            auto it = creators.find(pName.data());
            if (it != creators.end()) {
                return it->second->Create();
            }

            return nullptr;
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<ComponentFactoryBase>> creators;
    };

#define COMPONENT(Clazz)                                                  \
public:                                                                   \
    static size_t TypeId() {                                              \
        return Component::StaticTypeId<Clazz>();                          \
    }                                                                     \
    size_t GetTypeId() const override {                                   \
        return TypeId();                                                  \
    }                                                                     \
    static void Register() {                                              \
        ComponentRegistry::GetInstance().RegisterComponent<Clazz>(#Clazz); \
    }

}; // namespace golias
