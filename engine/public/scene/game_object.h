#pragma once
#include "stdafx.h"

namespace golias {


    class GameObject {

    public:
        virtual ~GameObject() = default;

        virtual void Update(float deltaTime);

        void SetName(CString name);

        String GetName() const;

        void SetParent(GameObject* parent);

        GameObject* GetParent() const;

        void Destroy();

        bool IsAlive() const;

    protected:
        GameObject() = default;

        friend class Scene;

    private:
        String mName;

        GameObject* mParent = nullptr;

        std::vector<std::unique_ptr<GameObject>> mChildren = {};

        bool mIsAlive = true;
    };
} // namespace golias
