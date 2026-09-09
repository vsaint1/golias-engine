#include "scene/components/soft_body_component.h"

#include "core/engine.h"
#include "physics/physics_manager.h"
#include "render/mesh.h"
#include "scene/game_object.h"
#include <BulletSoftBody/btSoftBody.h>

namespace golias {

    SoftBodyComponent::SoftBodyComponent(const Ref<Mesh>& mesh, const Ref<SoftBody>& softBody) : mMesh(mesh), mSoftBody(softBody) {

        if (mSoftBody) {
            mWidth         = mSoftBody->GetWidth();
            mHeight        = mSoftBody->GetHeight();
            mSubdivisionsX = mSoftBody->GetSubdivisionsX();
            mSubdivisionsY = mSoftBody->GetSubdivisionsY();
        }
    }

    bool SoftBodyComponent::LoadProperties(const Json& properties){
        return true;
    }

    void SoftBodyComponent::Start() {

        if (!mMesh) {
            return;
        }

        if (!mSoftBody || !mSoftBody->GetBody()) {
            return;
        }

        mSoftBody->SetPosition(GetOwner()->GetWorldPosition());

        mSoftBody->SetGameObject(GetOwner());

        mIndices.clear();

        const uint32_t columns = mSubdivisionsX + 1;
        for (uint32_t y = 0; y < mSubdivisionsY; ++y) {
            for (uint32_t x = 0; x < mSubdivisionsX; ++x) {
                const uint32_t a = y * columns + x;
                const uint32_t b = a + 1;
                const uint32_t c = a + columns;
                const uint32_t d = c + 1;
                mIndices.insert(mIndices.end(), {a, b, c, b, d, c, a, c, b, b, c, d});
            }
        }

        UpdateMesh();
    }

    void SoftBodyComponent::Update(float deltaTime) {
        UpdateMesh();
    }

    void SoftBodyComponent::UpdateMesh() {
        if (!mSoftBody || !mSoftBody->GetBody() || !mMesh) {
            return;
        }

        const size_t nodeCount = mSoftBody->GetBody()->m_nodes.size();
        std::vector<glm::vec3> positions(nodeCount);

        for (size_t i = 0; i < nodeCount; ++i) {
            const btVector3& position = mSoftBody->GetBody()->m_nodes[i].m_x;
            positions[i]              = glm::vec3(position.x(), position.y(), position.z()) - mSoftBody->GetPosition();
        }

        std::vector<glm::vec3> normals(nodeCount, glm::vec3(0.0f));

        for (size_t i = 0; i < mIndices.size(); i += 12) {
            for (size_t triangle = 0; triangle < 6; triangle += 3) {
                const uint32_t a       = mIndices[i + triangle];
                const uint32_t b       = mIndices[i + triangle + 1];
                const uint32_t c       = mIndices[i + triangle + 2];
                const glm::vec3 normal = glm::cross(positions[b] - positions[a], positions[c] - positions[a]);
                normals[a] += normal;
                normals[b] += normal;
                normals[c] += normal;
            }
        }

        mVertices.clear();
        mVertices.reserve(nodeCount * kVertexFloatCount);

        for (uint32_t y = 0; y <= mSubdivisionsY; ++y) {
            for (uint32_t x = 0; x <= mSubdivisionsX; ++x) {
                const size_t index = y * (mSubdivisionsX + 1) + x;
              
                const glm::vec3 normal =
                    glm::length2(normals[index]) > 1e-8f ? glm::normalize(normals[index]) : glm::vec3(0.0f, 1.0f, 0.0f);
                    
                const glm::vec3 position = positions[index];
                const glm::vec2 uv(static_cast<float>(x) / mSubdivisionsX, static_cast<float>(y) / mSubdivisionsY);
                const glm::vec3 color    = glm::vec3(1.0f, 1.0f, 1.0f);
                const glm::vec3 tangent   = glm::vec3(1.0f, 0.0f, 0.0f);
                const glm::vec3 bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

                mVertices.insert(mVertices.end(),
                                 {position.x, position.y, position.z,
                                  color.x, color.y, color.z,
                                  uv.x, uv.y,
                                  normal.x, normal.y, normal.z,
                                  tangent.x, tangent.y, tangent.z,
                                  bitangent.x, bitangent.y, bitangent.z});

            }
        }

        mMesh->Update(mVertices, mIndices);
    }

    void SoftBodyComponent::SetMesh(const Ref<Mesh>& mesh){
        mMesh = mesh;
    }

    void SoftBodyComponent::SetSoftBody(const Ref<SoftBody>& softBody){
        mSoftBody = softBody;
    }

    SoftBodyComponent::~SoftBodyComponent() {
    }
} // namespace golias
