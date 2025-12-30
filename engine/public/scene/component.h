#pragma once


namespace golias {

    class GameObject;

    class Component {
    public:
        virtual ~Component()       = default;
        virtual size_t GetTypeId() const = 0;

        GameObject* GetOwner() const;
        void SetOwner(GameObject* pOwner);

        virtual void Start() = 0;
        virtual void Update(float deltaTime) = 0;

        template <typename T>
        static size_t StaticTypeId() {
            static size_t type_id = next_type_id++;
            return type_id;
        }

    protected:
        GameObject* owner = nullptr;
        friend class GameObject;

    private:
        static size_t next_type_id;
    };


#define COMPONENT(Clazz)                         \
public:                                          \
    static size_t TypeId() {               \
        return Component::StaticTypeId<Clazz>(); \
    }                                            \
     size_t GetTypeId() const override {        \
        return TypeId();                  \
    }

}; // namespace golias
