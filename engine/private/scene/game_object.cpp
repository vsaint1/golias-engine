#include "scene/game_object.h"

namespace golias {

    void GameObject::Update(float deltaTime) {

        for (const auto& component : mComponents) {
            component->Update(deltaTime);
        }

        for (auto it = mChildren.begin(); it != mChildren.end();) {
            GameObject* child = it->get();

            if (child->IsAlive()) {

                child->Update(deltaTime);
                ++it;

            } else {
                it = mChildren.erase(it);
            }
        }
    }

    void GameObject::SetName(CString name) {
        mName = name;
    }

    String GameObject::GetName() const {
        return mName;
    }

    void GameObject::SetParent(GameObject* parent) {
        mParent = parent;
    }

    GameObject* GameObject::GetParent() const {
        return mParent;
    }

    void GameObject::Destroy() {
        mIsAlive = false;
    }

    void GameObject::AddComponent(Component* component) {

        if (component) {
            component->mOwner = this;
            mComponents.emplace_back(component);
        }
    }

    bool GameObject::IsAlive() const {
        return mIsAlive;
    }

    glm::vec3 GameObject::GetPosition() const {
        return mPosition;
    }

    void GameObject::SetPosition(const glm::vec3& position) {
        mPosition = position;
    }

    glm::vec3 GameObject::GetWorldPosition() const {
        glm::vec4 hom = GetWorldTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return glm::vec3(hom) / hom.w;
    }

    glm::quat GameObject::GetRotation() const {
        return mRotation;
    }

    void GameObject::SetRotation(const glm::quat& rotation) {
        mRotation = rotation;
    }

    void GameObject::SetRotation(const glm::vec3& eulerAngles) {
        mRotation = glm::quat(eulerAngles);
    }

    glm::vec3 GameObject::GetScale() const {
        return mScale;
    }

    void GameObject::SetScale(const glm::vec3& scale) {
        mScale = scale;
    }

    void GameObject::RotateLocal(const glm::vec3& axis, float angle) {
        glm::quat rotation = glm::angleAxis(angle, axis);
        mRotation          = rotation * mRotation;
    }

    glm::mat4 GameObject::GetWorldTransform() const {

        if (mParent) {
            return mParent->GetWorldTransform() * GetLocalTransform();
        } else {
            return GetLocalTransform();
        }
    }

    glm::mat4 GameObject::GetLocalTransform() const {
        glm::mat4 mat = glm::mat4(1.0f);

        mat = glm::translate(mat, mPosition);
        mat = mat * glm::mat4_cast(mRotation);
        mat = glm::scale(mat, mScale);

        return mat;
    }


    glm::vec3 GameObject::GetForward() const {
        return mRotation * glm::vec3(0.0f, 0.0f, 1.0f);
    }

} // namespace golias
