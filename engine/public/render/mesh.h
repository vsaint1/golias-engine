#pragma once
#include "graphics/vertex_layout.h"
#include "stdafx.h"

namespace golias {

    class Model;
    struct ModelPrimitive;

    class Mesh {
    public:
        Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices);
        Mesh(const VertexLayout& layout, const std::vector<float>& vertices);

        ~Mesh();

        static Ref<Mesh> Load(CString path);

        static Ref<Mesh> Create(const Model& model, const ModelPrimitive& primitive);

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

        void Unbind() const;
        
        void Draw() const;

    private:
        Mesh(const Mesh&)            = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&)                 = delete;
        Mesh& operator=(Mesh&&)      = delete;

    private:
        VertexLayout mVertexLayout;
        size_t mVertexCount = 0;
        size_t mIndexCount  = 0;

        GLuint mVAO = 0;
        GLuint mVBO = 0;
        GLuint mEBO = 0;
    };

} // namespace golias
