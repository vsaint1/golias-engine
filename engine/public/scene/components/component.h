#pragma once


namespace golias {

    class GameObject;

    class Component {

    public:
        virtual ~Component() = default;

        virtual size_t GetTypeId() const = 0;

        virtual void Update(float deltaTime) = 0;

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

#define COMPONENT(ClazzName)              \
    static size_t TypeId() {              \
        return Component::StaticTypeId<ClazzName>(); \
    }                                     \
    size_t GetTypeId() const override {   \
        return TypeId(); \
    }
} // namespace golias
