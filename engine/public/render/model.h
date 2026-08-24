#pragma once

#include "graphics/vertex_layout.h"
#include "scene/components/animation_component.h"
#include "stdafx.h"

namespace golias {

    struct ModelPrimitive {
        size_t vertexOffset = 0;
        size_t vertexCount  = 0;
        size_t indexOffset  = 0;
        size_t indexCount   = 0;
        int materialIndex   = -1;
        String name;
    };

    struct ModelNode {
        String name;
        glm::mat4 localTransform = glm::mat4(1.0f);
        std::vector<size_t> children;
        std::vector<size_t> primitives;
    };

    struct ModelMaterial {
        String name;
        glm::vec4 baseColor = glm::vec4(1.0f);
        String baseColorTexture;
    };

    /// @brief  Represents a 3D model that can be loaded from various file formats (e.g., glTF, OBJ, FBX).
    class Model {
    public:
        virtual ~Model() = default;

        static Ref<Model> Load(CString path);

        const VertexLayout& GetVertexLayout() const;

        const std::vector<float>& GetVertices() const;

        const std::vector<uint32_t>& GetIndices() const;

        const std::vector<ModelPrimitive>& GetPrimitives() const;

        const std::vector<ModelMaterial>& GetMaterials() const;

        const std::vector<ModelNode>& GetNodes() const;

        const std::vector<size_t>& GetSceneRoots() const;

        bool HasAnimations() const;

        const std::vector<Ref<AnimationClip>>& GetAnimations() const;

    protected:
        void AppendVertex(const float* position, const float* color, const float* texCoord, const float* normal);

        VertexLayout mVertexLayout;
        std::vector<float> mVertices;
        std::vector<uint32_t> mIndices;
        std::vector<ModelPrimitive> mPrimitives;
        std::vector<ModelMaterial> mMaterials;
        std::vector<ModelNode> mNodes;
        std::vector<size_t> mSceneRoots;
        std::vector<Ref<AnimationClip>> mAnimations;
    };

    /// @brief  Loads a glTF model based on 2.0 specification.
    class GltfModel final : public Model {
    public:
        GltfModel(const std::vector<char>& data, CString path);
    };

    /// @brief  Loads a Wavefront OBJ model.
    class ObjModel final : public Model {
    public:
        ObjModel(const std::vector<char>& data, CString path);
    };

    // TODO: Implement FBX model loading
    class FbxModel final : public Model {
    public:
        FbxModel() = default;
    };


} // namespace golias
