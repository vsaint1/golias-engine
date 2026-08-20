#pragma once


namespace golias {

    class GameObject;

    class Component {


    public:
        virtual ~Component() = default;

        virtual void Update(float deltaTime) = 0;

        GameObject* GetOwner() const;

    protected:
        Component() = default;

        friend class GameObject;

    private:
        GameObject* mOwner = nullptr;
    };
} // namespace golias
