#include "scene/game_object.h"

#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_transform.hpp>

namespace golias {
    void GameObject::AddComponent(Component* pComponent) {
        components.emplace_back(pComponent);
        pComponent->SetOwner(this);

        spdlog::info("GameObject::AddComponent added Component: {} to GameObject: {}", typeid(*pComponent).name(), typeid(*this).name());
    }

    void GameObject::Update(float deltaTime) {

        for (auto& component : components) {
            component->Update(deltaTime);
        }


        for (auto it = children.begin(); it != children.end(); ++it) {
            if ((*it)->IsAlive()) {

                (*it)->Update(deltaTime);
                ++it;

            } else {
                it = children.erase(it);
            }
        }
    }

    const std::string& GameObject::GetName() const {
        return name;
    }

    void GameObject::SetName(const std::string& newName) {
        name = newName;
    }

    void GameObject::SetParent(GameObject* pParent) {
        parent = pParent;
    }

    GameObject* GameObject::GetParent() const {
        return parent;
    }


    const std::vector<std::unique_ptr<GameObject>>& GameObject::GetChildren() const {
        return children;
    }

    void GameObject::Destroy() {
        is_alive = false;
    }

    bool GameObject::IsAlive() const {
        return is_alive;
    }


    glm::vec3& GameObject::GetPosition() {
        return position;
    }

    void GameObject::SetPosition(const glm::vec3& newPosition) {
        position = newPosition;
    }

    glm::quat& GameObject::GetRotation() {
        return rotation;
    }

    void GameObject::SetRotation(const glm::quat& newRotation) {
        rotation = newRotation;
    }

    glm::vec3& GameObject::GetScale() {
        return scale;
    }

    void GameObject::SetScale(const glm::vec3& newScale) {
        scale = newScale;
    }

     glm::mat4 GameObject::GetLocalTransform() const {
        glm::mat4 mat = glm::mat4(1.0f);
        mat = glm::translate(mat, position);
        mat = mat * glm::mat4_cast(rotation);
        mat = glm::scale(mat, scale);

        return mat;
    }

     glm::mat4 GameObject::GetWorldTransform() const {
        if (parent) {
            return parent->GetWorldTransform() * GetLocalTransform();
        } else {
            return GetLocalTransform();
        }
    }
}; // namespace golias
