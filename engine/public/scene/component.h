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

        template <typename T>
        static size_t StaticTypeId() {
            static size_t type_id = next_type_id++;
            return type_id;
        }

        virtual void Start();
        virtual void Update(float deltaTime);
        virtual void LoadProperties(const nlohmann::json& json);

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
            parents[T::TypeId()].push_back(Component::StaticTypeId<Component>());
        }

        template <typename T, typename ParentType>
        void RegisterComponent(const std::string_view pName) {
            creators.emplace(pName.data(), std::make_unique<ComponentFactory<T>>());
            parents[T::TypeId()].push_back(Component::StaticTypeId<ParentType>());
        }

        Component* CreateComponent(const std::string_view pName) const {
            auto it = creators.find(pName.data());
            if (it != creators.end()) {
                return it->second->Create();
            }

            return nullptr;
        }

        bool HasParent(size_t objTypeId, size_t parentTypeId) const {
            
            auto record = parents.find(objTypeId);
            if (record == parents.end()) {
              
                return false;
            }

            const auto& parentList = record->second;

            if(std::find(parentList.begin(), parentList.end(), parentTypeId) != parentList.end()) {
                return true;
            }

            for (const auto& parent : parentList) {
                if(parent == objTypeId) {
                    continue;
                }
                
                if(HasParent(parent, parentTypeId)) {
                    return true;
                }
            }

            return false;
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<ComponentFactoryBase>> creators;
        std::unordered_map<size_t, std::vector<size_t>> parents;
    };

#define COMPONENT(Clazz)                                                   \
public:                                                                    \
    static size_t TypeId() {                                               \
        return Component::StaticTypeId<Clazz>();                           \
    }                                                                      \
    size_t GetTypeId() const override {                                    \
        return TypeId();                                                   \
    }                                                                      \
    static void Register() {                                               \
        ComponentRegistry::GetInstance().RegisterComponent<Clazz>(#Clazz); \
    }

#define COMPONENT_DERIVED(Clazz, ParentType)                                 \
public:                                                                    \
    static size_t TypeId() {                                               \
        return Component::StaticTypeId<Clazz>();                           \
    }                                                                      \
    size_t GetTypeId() const override {                                    \
        return TypeId();                                                   \
    }                                                                      \
    static void Register() {                                               \
        ComponentRegistry::GetInstance().RegisterComponent<Clazz, ParentType>(#Clazz); \
    }

}; // namespace golias
