#include "scene/game_object.h"

#include "core/engine.h"
#include "core/io/file_system.h"
#include "graphics/texture_2d.h"
#include "physics/collision.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/model.h"
#include "scene/components/animation_component.h"
#include "scene/components/static_mesh_component.h"
#include "scene/scene.h"

namespace golias {

    GameObject* GameObject::Load(CString modelPath, Scene* scene, CString name) {
        if (!scene) {
            return nullptr;
        }

        const Ref<Model> model = Model::Load(modelPath);
        if (!model || (model->GetPrimitives().empty() && model->GetNodes().empty())) {
            return nullptr;
        }

        GameObject* root = scene->CreateGameObject(name.empty() ? Path(modelPath).stem().string() : name);
        if (!root) {
            return nullptr;
        }

        if (model->HasAnimations()) {
            auto* animation = new AnimationComponent();
            root->AddComponent(animation);
            for (const Ref<AnimationClip>& clip : model->GetAnimations()) {
                animation->RegisterClip(clip->Name, clip);
            }
        }

        auto add_primitive = [&](GameObject* parent, const ModelPrimitive& primitive) {
            Ref<Mesh> mesh         = Mesh::Create(*model, primitive);
            Ref<Material> material = Engine::GetInstance().GetAssetManager().LoadDefaultMaterial();
            if (!mesh || !material) {
                return;
            }

            if (primitive.materialIndex >= 0 && primitive.materialIndex < static_cast<int>(model->GetMaterials().size())) {
                const ModelMaterial& definition = model->GetMaterials()[primitive.materialIndex];
                material->SetParameter("_BaseColor", definition.baseColor);
                if (!definition.baseColorTexture.empty()) {
                    Ref<Texture2D> texture = Engine::GetInstance().GetAssetManager().Load<Texture2D>(definition.baseColorTexture);

                    if (texture) {
                        material->SetParameter("_MainTexture", texture);
                    } else {
                        GOLIAS_LOG_WARN("Failed to load texture: %s", definition.baseColorTexture.c_str());
                        material->SetParameter("_MainTexture", Engine::GetInstance().GetAssetManager().AcquireErrorTexture());
                    }
                }
            }

            GameObject* child = scene->CreateGameObject(primitive.name.empty() ? parent->GetName() : primitive.name, parent);
            if (child) {
                child->AddComponent(new StaticMeshComponent(mesh, material));
            }
        };

        if (model->GetNodes().empty()) {

            for (const ModelPrimitive& primitive : model->GetPrimitives()) {
                add_primitive(root, primitive);
            }

            return root;
        }

        const auto& nodes                                    = model->GetNodes();
        std::function<void(size_t, GameObject*)> create_node = [&](size_t nodeIndex, GameObject* parent) {
            if (nodeIndex >= nodes.size()) {
                return;
            }

            const ModelNode& node = nodes[nodeIndex];
            GameObject* object    = scene->CreateGameObject(node.name.empty() ? "Node" : node.name, parent);

            if (!object) {
                return;
            }

            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 position;
            glm::vec3 skew;
            glm::vec4 perspective;

            if (glm::decompose(node.localTransform, scale, rotation, position, skew, perspective)) {
                object->SetPosition(position);
                object->SetRotation(rotation);
                object->SetScale(scale);
            }

            for (size_t primitiveIndex : node.primitives) {
                if (primitiveIndex < model->GetPrimitives().size()) {
                    add_primitive(object, model->GetPrimitives()[primitiveIndex]);
                }
            }

            for (size_t childIndex : node.children) {
                create_node(childIndex, object);
            }
        };

        for (size_t rootIndex : model->GetSceneRoots()) {
            create_node(rootIndex, root);
        }

        return root;
    }

    void GameObject::Start() {
    }

    bool GameObject::LoadProperties(const Json& properties) {

        return true;
    }

    void GameObject::Update(float deltaTime) {

        if (!mIsActive) {
            return;
        }

        for (const auto& component : mComponents) {
            component->Update(deltaTime);
        }

        for (auto it = mChildren.begin(); it != mChildren.end();) {
            GameObject* child = it->get();

            if (child->IsAlive() && child->IsActive()) {

                child->Update(deltaTime);
                ++it;

            } else {
                it = mChildren.erase(it);
            }
        }
    }

    void GameObject::OnCollisionEnter(const Collision& collision) {
        UNUSED_PARAMETER(collision);
    }

    void GameObject::OnCollisionExit(const Collision& collision) {
        UNUSED_PARAMETER(collision);
    }

    GameObject* GameObject::FindChildByName(CString name) const {

        if (mName == name) {
            return const_cast<GameObject*>(this);
        }

        for (const auto& child : mChildren) {
            GameObject* found = child->FindChildByName(name);
            if (found) {
                return found;
            }
        }


        return nullptr;
    }

    bool GameObject::IsActive() const {
        return mIsActive;
    }

    void GameObject::SetActive(bool active) {
        if (mIsActive == active) {
            return;
        }

        mIsActive = active;

        for (const auto& component : mComponents) {
            if (active) {
                component->OnEnable();
            } else {
                component->OnDisable();
            }
        }

        for (const auto& child : mChildren) {
            child->SetActive(active);
        }
    }

    void GameObject::SetName(CString name) {
        mName = name;
    }

    Scene* GameObject::GetCurrentScene() const {
        return mScene;
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
            component->Start();
        }
    }

    bool GameObject::IsAlive() const {
        return mIsAlive;
    }

    const std::vector<std::unique_ptr<GameObject>>& GameObject::GetChildren() const {
        return mChildren;
    }

    glm::vec3 GameObject::GetPosition() const {
        return mPosition;
    }

    void GameObject::SetPosition(const glm::vec3& position) {
        mPosition = position;
    }

    void GameObject::SetWorldPosition(const glm::vec3& position) {
        if (mParent) {
            glm::mat4 parentWorldTransform = mParent->GetWorldTransform();
            glm::mat4 parentInverse        = glm::inverse(parentWorldTransform);
            glm::vec4 localPos             = parentInverse * glm::vec4(position, 1.0f);
            mPosition                      = glm::vec3(localPos) / localPos.w;
        } else {
            mPosition = position;
        }
    }


    glm::vec3 GameObject::GetWorldPosition() const {
        glm::vec4 hom = GetWorldTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return glm::vec3(hom) / hom.w;
    }


    glm::quat GameObject::GetRotation() const {
        return mRotation;
    }

    glm::quat GameObject::GetWorldRotation() const {
        if (mParent) {
            return mParent->GetWorldRotation() * mRotation;
        } else {
            return mRotation;
        }
    }

    void GameObject::SetWorldRotation(const glm::quat& rotation) {
        if (mParent) {
            glm::quat parentWorldRotation = mParent->GetWorldRotation();
            mRotation                     = glm::inverse(parentWorldRotation) * rotation;
        } else {
            mRotation = rotation;
        }
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


    GameObject* ObjectRegistry::CreateObject(CString pName) const {
        auto it = creators.find(pName.data());
        if (it != creators.end()) {
            return it->second->Create();
        }

        return nullptr;
    }

    glm::vec2 GameObject::GetPosition2D() const {
        return glm::vec2(mPosition.x, mPosition.y);
    }

    void GameObject::SetPosition2D(const glm::vec2& position) {
        mPosition = glm::vec3(position.x, position.y, 0.0f);
    }

    glm::vec2 GameObject::GetWorldPosition2D() const {
        const glm::vec4 hom = GetWorldTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return glm::vec2(glm::vec3(hom) / hom.w);
    }

    float GameObject::GetRotation2D() const {
        return glm::eulerAngles(mRotation).z;
    }

    void GameObject::SetRotation2D(float angle) {
        mRotation = glm::angleAxis(glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    glm::vec2 GameObject::GetScale2D() const {
        return glm::vec2(mScale.x, mScale.y);
    }

    void GameObject::SetScale2D(const glm::vec2& scale) {
        mScale = glm::vec3(scale.x, scale.y, 1.0f);
    }

    glm::mat4 GameObject::GetLocalTransform2D() const {
        glm::mat4 mat = glm::mat4(1.0f);

        const float rotation = GetRotation2D();

        float c = glm::cos(rotation);
        float s = glm::sin(rotation);

        mat[0][0] = c;
        mat[0][1] = -s;
        mat[1][0] = s;
        mat[1][1] = c;
        mat[3][0] = mPosition.x;
        mat[3][1] = mPosition.y;
        mat[0][0] *= mScale.x;
        mat[1][1] *= mScale.y;

        return mat;
    }

    glm::mat4 GameObject::GetWorldTransform2D() const {

        if (mParent) {
            return mParent->GetWorldTransform2D() * GetLocalTransform2D();
        } else {
            return GetLocalTransform2D();
        }
    }
} // namespace golias
