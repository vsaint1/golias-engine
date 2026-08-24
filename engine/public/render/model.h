#pragma once

#include "graphics/vertex_layout.h"
#include "stdafx.h"

namespace golias {

    /// @brief  Represents a 3D model that can be loaded from various file formats (e.g., glTF, OBJ, FBX). 
    class Model {
    public:
        virtual ~Model() = default;

        static Ref<Model> Load(CString path);

        const VertexLayout& GetVertexLayout() const;

        const std::vector<float>& GetVertices() const;

        const std::vector<uint32_t>& GetIndices() const;

    protected:
        void AppendVertex(const float* position, const float* color, const float* texCoord, const float* normal);

        VertexLayout mVertexLayout;
        std::vector<float> mVertices;
        std::vector<uint32_t> mIndices;
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
