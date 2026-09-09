#pragma once

#include "physics/soft_body.h"
#include "scene/components/component.h"

namespace golias {

    class Mesh;

    class SoftBodyComponent : public Component {
        COMPONENT(SoftBodyComponent)

    public:
        SoftBodyComponent() = default;
        SoftBodyComponent(const Ref<Mesh>& mesh, const Ref<SoftBody>& softBody);
        ~SoftBodyComponent() override;

        bool LoadProperties(const Json& properties);

        void Start() override;

        void Update(float deltaTime) override;

        void SetMesh(const Ref<Mesh>& mesh);

        void SetSoftBody(const Ref<SoftBody>& softBody);

    private:
        void UpdateMesh();

        Ref<Mesh> mMesh         = nullptr;
        Ref<SoftBody> mSoftBody = nullptr;

        float mWidth  = 1.0f;
        float mHeight = 1.0f;

        uint32_t mSubdivisionsX = 1;
        uint32_t mSubdivisionsY = 1;


        std::vector<uint32_t> mIndices;
        std::vector<float> mVertices;
    };
} // namespace golias
