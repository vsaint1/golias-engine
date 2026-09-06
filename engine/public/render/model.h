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
        bool Skinned        = false;
        String name;
    };

    struct ModelNode {
        String name;
        glm::mat4 localTransform = glm::mat4(1.0f);
        int skinIndex            = -1;
        std::vector<size_t> children;
        std::vector<size_t> primitives;
    };

    struct ModelSkin {
        String name;
        std::vector<String> jointNames;
        std::vector<glm::mat4> inverseBindMatrices;
    };

    /// @brief  Decoded (RGBA/RGB) pixel data for a texture embedded in a model file (e.g. a GLB bufferView).
    struct ModelTextureData {
        std::vector<unsigned char> Pixels;
        int32_t Width      = 0;
        int32_t Height     = 0;
        int32_t Components = 0;
        String Path; // Optional path to the texture file (if not embedded in the model)

        bool IsValid() const {
            return !Pixels.empty() && Width > 0 && Height > 0 && Components > 0;
        }
    };

    // TODO: Add more material properties (metallic/roughness, emissive, etc.) and texture maps (normal, metallic, roughness, occlusion, emissive).
    struct ModelMaterial {
        String name;

        glm::vec4 baseColor = glm::vec4(1.0f);
        ModelTextureData baseColorData;

        ModelTextureData normalData;
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

        /// @brief  Flat skinned-vertex stream (stride: SkinnedVertexOffsets::kSkinnedVertexFloatCount).
        const std::vector<float>& GetSkinnedVertices() const;

        /// @brief  Index buffer for skinned primitives.
        const std::vector<uint32_t>& GetSkinnedIndices() const;

        /// @brief  Skins (skeletons) defined by the model.
        const std::vector<ModelSkin>& GetSkins() const;

        bool HasAnimations() const;

        const std::vector<Ref<AnimationClip>>& GetAnimations() const;

    protected:
        void AppendVertex(const float* position, const float* color, const float* texCoord, const float* normal);

        VertexLayout mVertexLayout;
        std::vector<float> mVertices;
        std::vector<uint32_t> mIndices;
        std::vector<float> mSkinnedVertices;
        std::vector<uint32_t> mSkinnedIndices;
        std::vector<ModelPrimitive> mPrimitives;
        std::vector<ModelMaterial> mMaterials;
        std::vector<ModelNode> mNodes;
        std::vector<size_t> mSceneRoots;
        std::vector<ModelSkin> mSkins;
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
