#include "scene/game_object.h"

#include "core/engine.h"
#include "core/model.h"
#include <scene/3d/mesh_component.h>

namespace golias {


    void GameObject::AddComponent(Component* pComponent) {
        if (!pComponent) {
            return;
        }

        components.emplace_back(pComponent);
        pComponent->SetOwner(this);
        pComponent->Start();
        spdlog::info("GameObject::AddComponent added Component: {} to GameObject: {}", typeid(*pComponent).name(), typeid(*this).name());
    }

    void GameObject::LoadProperties(const nlohmann::json& json) {
        // Override in derived classes
    }

    void GameObject::Start() {
        // Override in derived classes
    }

    void GameObject::Update(float deltaTime) {
        if (!IsActive()) {
            return;
        }

        for (auto& component : components) {
            component->Update(deltaTime);
        }

        for (auto it = children.begin(); it != children.end();) {
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

    void GameObject::Destroy() {
        isAlive = false;
    }

    bool GameObject::IsAlive() const {
        return isAlive;
    }


    bool GameObject::IsActive() const {
        return isActive;
    }

    void GameObject::SetActive(bool active) {
        isActive = active;
    }


    bool GameObject::SetParent(GameObject* pParent) {
        if (!scene) {
            return false;
        }
        return scene->SetParent(this, pParent);
    }

    Scene* GameObject::GetScene() const {
        return scene;
    }

    GameObject* GameObject::GetParent() const {
        return parent;
    }

    const std::vector<std::unique_ptr<GameObject>>& GameObject::GetChildren() const {
        return children;
    }

    GameObject* GameObject::FindChildByName(const std::string_view pName) const {
        if (name == pName) {
            return const_cast<GameObject*>(this);
        }

        for (const auto& child : children) {
            if (child->GetName() == pName) {
                return child.get();
            }
            GameObject* found = child->FindChildByName(pName);
            if (found) {
                return found;
            }
        }

        return nullptr;
    }


    glm::vec3 GameObject::GetPosition() const {
        return position;
    }

    void GameObject::SetPosition(const glm::vec3& pos) {
        position = pos;
    }

    glm::vec3 GameObject::GetWorldPosition() const {
        glm::vec4 hom = GetWorldTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return glm::vec3(hom) / hom.w;
    }

    void GameObject::SetWorldPosition(const glm::vec3& pos) {
        if (parent) {
            glm::mat4 parentWorldTransform = parent->GetWorldTransform();
            glm::mat4 inverseParentWorld   = glm::inverse(parentWorldTransform);
            glm::vec4 localPos             = inverseParentWorld * glm::vec4(pos, 1.0f);
            SetPosition(glm::vec3(localPos) / localPos.w);
        } else {
            SetPosition(pos);
        }
    }


    glm::quat GameObject::GetRotation() const {
        return rotation;
    }

    void GameObject::SetRotation(const glm::quat& rot) {
        rotation = rot;
    }

    glm::quat GameObject::GetWorldRotation() const {
        if (parent) {
            return parent->GetWorldRotation() * rotation;
        } else {
            return rotation;
        }
    }

    void GameObject::SetWorldRotation(const glm::quat& rot) {
        if (parent) {
            glm::quat parentWorldRotation = parent->GetWorldRotation();
            SetRotation(glm::inverse(parentWorldRotation) * rot);
        } else {
            SetRotation(rot);
        }
    }


    glm::vec3 GameObject::GetScale() const {
        return scale;
    }

    void GameObject::SetScale(const glm::vec3& value) {
        scale = value;
    }


    glm::mat4 GameObject::GetLocalTransform() const {
        glm::mat4 mat = glm::mat4(1.0f);
        mat           = glm::translate(mat, position);
        mat           = mat * glm::mat4_cast(rotation);
        mat           = glm::scale(mat, scale);
        return mat;
    }

    glm::mat4 GameObject::GetWorldTransform() const {
        if (parent) {
            return parent->GetWorldTransform() * GetLocalTransform();
        } else {
            return GetLocalTransform();
        }
    }


    glm::vec2 GameObject::GetPosition2D() const {
        return glm::vec2(position.x, position.y);
    }

    void GameObject::SetPosition2D(const glm::vec2& pos) {
        position = glm::vec3(pos, 0.0f);
    }

    glm::vec2 GameObject::GetWorldPosition2D() const {
        glm::vec4 hom = GetWorldTransform2D() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return glm::vec2(hom) / hom.w;
    }


    float GameObject::GetRotation2D() const {
        return glm::angle(rotation);
    }

    void GameObject::SetRotation2D(float degrees) {
        rotation = glm::angleAxis(glm::radians(degrees), glm::vec3(0.0f, 0.0f, 1.0f));
    }


    glm::vec2 GameObject::GetScale2D() const {
        return glm::vec2(scale.x, scale.y);
    }

    void GameObject::SetScale2D(const glm::vec2& value) {
        scale = glm::vec3(value, 1.0f);
    }


    glm::mat4 GameObject::GetLocalTransform2D() const {
        glm::mat4 mat = glm::mat4(1.0f);
        mat           = glm::translate(mat, glm::vec3(position.x, position.y, 0.0f));
        mat           = glm::rotate(mat, glm::radians(GetRotation2D()), glm::vec3(0.0f, 0.0f, 1.0f));
        mat           = glm::scale(mat, glm::vec3(scale.x, scale.y, 1.0f));
        return mat;
    }

    glm::mat4 GameObject::GetWorldTransform2D() const {
        if (parent) {
            return parent->GetWorldTransform2D() * GetLocalTransform2D();
        }
        return GetLocalTransform2D();
    }

    void GameObject::SetUseIBL(bool value) {

        if (auto meshComp = GetComponent<golias::MeshComponent>()) {
            if (auto material = meshComp->GetMaterial()) {
                material->SetImageBasedLighting(value);
            }
        }

        for (const auto& child : GetChildren()) {
            child->SetUseIBL(value);
        }
    }

    ObjectRegistry& ObjectRegistry::GetInstance() {
        static ObjectRegistry instance;
        return instance;
    }

    GameObject* ObjectRegistry::CreateObject(const std::string_view pName) const {
        auto it = creators.find(pName.data());
        if (it != creators.end()) {
            return it->second->Create();
        }

        return nullptr;
    }
} // namespace golias
