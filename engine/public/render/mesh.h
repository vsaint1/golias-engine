#pragma once
#include "graphics/vertex_layout.h"
#include "math/aabb.h"
#include "stdafx.h"

namespace golias {

    class Model;
    class Buffer;

    struct ModelPrimitive;

    class Mesh {
    public:
        Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices);
        Mesh(const VertexLayout& layout, const std::vector<float>& vertices);

        /// @brief  Constructs a mesh from a raw interleaved vertex stream
        Mesh(const VertexLayout& layout,
             const void* vertexData,
             size_t vertexDataBytes,
             size_t vertexCount,
             const std::vector<uint32_t>& indices);

        ~Mesh();

        static Ref<Mesh> Load(CString path);

        static Ref<Mesh> Create(const Model& model, const ModelPrimitive& primitive);

        /// @brief  Merges multiple primitives sharing the same vertex layout into a single mesh (single draw call).
        static Ref<Mesh> Create(const Model& model, const std::vector<const ModelPrimitive*>& primitives);

        static Ref<Mesh> CreateCube(const glm::vec3& size = glm::vec3(1.0f), uint32_t segments = 1);

        static Ref<Mesh> CreateQuad(const glm::vec2& size = {1.0f, 1.0f});

        static Ref<Mesh> CreateSphere(float radius = 0.5f, uint32_t sectorCount = 32, uint32_t stackCount = 16);

        static Ref<Mesh>
            CreateTorus(float majorRadius = 0.5f, float minorRadius = 0.2f, uint32_t majorSegments = 32, uint32_t minorSegments = 16);

        static Ref<Mesh> CreateCylinder(float radiusTop = 0.5f, float radiusBottom = 0.5f, float height = 1.0f, uint32_t sectorCount = 32);

        static Ref<Mesh> CreateCone(float radius = 0.5f, float height = 1.0f, uint32_t sectorCount = 32);

        static Ref<Mesh>
            CreateCapsule(float radius = 0.5f, float cylinderHeight = 1.0f, uint32_t sectorCount = 16, uint32_t hemisphereRings = 8);


        void Bind() const;

        void Update(const std::vector<float>& vertices, const std::vector<uint32_t>& indices = {});

        void Unbind() const;

        void Draw() const;

        void DrawIndexed(uint32_t start, uint32_t count) const;

        /// @brief  Renders the mesh multiple times in a single draw call.
        void DrawInstanced(const Buffer& instanceBuffer, uint32_t instanceCount) const;

        const AABB& GetAABB() const;

    private:
        Mesh(const Mesh&)            = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&)                 = delete;
        Mesh& operator=(Mesh&&)      = delete;

        /// @brief  Allocates the vertex/index buffers and configures the VAO for the current layout.
        void ConfigureBuffers(const void* vertexData, size_t vertexDataBytes, const std::vector<uint32_t>& indices);

    private:
        VertexLayout mVertexLayout;
        size_t mVertexCount = 0;
        size_t mIndexCount  = 0;
        AABB mAABB;

        GLuint mVAO      = 0;
        Ref<Buffer> mEBO = nullptr;
        Ref<Buffer> mVBO = nullptr;
    };

} // namespace golias
