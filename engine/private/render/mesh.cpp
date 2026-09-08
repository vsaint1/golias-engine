#include "render/mesh.h"

#include "core/engine.h"
#include "core/io/asset_manager.h"
#include "graphics/buffer.h"
#include "graphics/vertex_array.h"
#include "math/tangent.h"
#include "render/model.h"
#include "render/render_stats.h"

namespace golias {

    namespace {

        void generate_mesh_tangents(std::vector<float>& vertices, const std::vector<uint32_t>& indices) {

            TangentLayout layout;
            layout.Stride    = kVertexFloatCount;
            layout.Position  = offsetof(Vertex, position) / sizeof(float);
            layout.TexCoord  = offsetof(Vertex, texcoord) / sizeof(float);
            layout.Normal    = offsetof(Vertex, normal) / sizeof(float);
            layout.Tangent   = offsetof(Vertex, tangent) / sizeof(float);
            layout.Bitangent = offsetof(Vertex, bitangent) / sizeof(float);
            GenerateTangents(vertices, indices, layout);
        }

        // Flattens Vertex data (tangents/bitangents may be zeroed) into the interleaved float buffer.
        std::vector<float> flatten(const std::vector<Vertex>& vertices) {
            std::vector<float> out;
            out.reserve(vertices.size() * kVertexFloatCount);

            // clang-format off
            for (const Vertex& v : vertices) {
                out.insert(out.end(),
                           {v.position.x,    v.position.y,    v.position.z,
                            v.color.x,       v.color.y,       v.color.z,
                            v.texcoord.x,    v.texcoord.y,
                            v.normal.x,      v.normal.y,      v.normal.z,
                            v.tangent.x,     v.tangent.y,     v.tangent.z,
                            v.bitangent.x,   v.bitangent.y,   v.bitangent.z});
            }
            // clang-format on

            return out;
        }

    } // namespace

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices) {
        mVertexLayout = layout;
        mVertexCount  = vertices.size() / (layout.Stride / sizeof(float));
        mIndexCount   = indices.size();

        for (const auto& element : layout.Elements) {
            if (element.Index != 0) {
                continue;
            }

            const size_t stride = layout.Stride / sizeof(float);
            const size_t offset = element.Offset / sizeof(float);

            for (size_t vertex = 0; vertex < mVertexCount && offset + 2 < stride; ++vertex) {
                const size_t position = vertex * stride + offset;

                if (position + 2 < vertices.size()) {
                    mAABB.Expand(glm::vec3(vertices[position], vertices[position + 1], vertices[position + 2]));
                }
            }

            break;
        }

        ConfigureBuffers(vertices.data(), vertices.size() * sizeof(float), indices);
    }

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& vertices) : Mesh(layout, vertices, {}) {
    }

    Mesh::Mesh(const VertexLayout& layout,
               const void* vertexData,
               size_t vertexDataBytes,
               size_t vertexCount,
               const std::vector<uint32_t>& indices) {
        mVertexLayout = layout;
        mVertexCount  = vertexCount;
        mIndexCount   = indices.size();

        const uint8_t* base = static_cast<const uint8_t*>(vertexData);
        for (const auto& element : layout.Elements) {
            if (element.Index != 0 || element.Format != VertexFormat::Float3) {
                continue;
            }

            for (size_t vertex = 0; vertex < mVertexCount && element.Offset + 12 <= layout.Stride; ++vertex) {
                const uint8_t* position = base + vertex * layout.Stride + element.Offset;
                mAABB.Expand(glm::vec3(*reinterpret_cast<const float*>(position),
                                       *reinterpret_cast<const float*>(position + sizeof(float)),
                                       *reinterpret_cast<const float*>(position + 2 * sizeof(float))));
            }

            break;
        }

        ConfigureBuffers(vertexData, vertexDataBytes, indices);
    }

    void Mesh::ConfigureBuffers(const void* vertexData, size_t vertexDataBytes, const std::vector<uint32_t>& indices) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        // clang-format off
        BufferDesc desc = {
            .Target = BufferTarget::Vertex,
            .Usage = BufferUsage::Static, 
            .Size = vertexDataBytes
        };
        // clang-format on

        mVBO = device.CreateBuffer(desc);
        mVBO->Update(vertexData, vertexDataBytes);

        if (!indices.empty()) {

            // clang-format off
            BufferDesc desc = {
                .Target = BufferTarget::Index,
                .Usage = BufferUsage::Static, 
                .Size = indices.size() * sizeof(uint32_t)
            };
            // clang-format on

            mEBO = device.CreateBuffer(desc);
            mEBO->Update(indices.data(), indices.size() * sizeof(uint32_t));
        }

        mVAO = new VertexArray();
        mVAO->SetVertexBuffer(mVBO, mVertexLayout);

        if (mEBO) {
            mVAO->SetIndexBuffer(mEBO);
        }
    }

    Ref<Mesh> Mesh::Load(CString path) {
        const Ref<Model> model = Model::Load(path);

        if (!model || model->GetVertices().empty() || model->GetIndices().empty()) {
            GOLIAS_LOG_ERROR("Failed to load mesh from model: %s", path.data());
            return nullptr;
        }

        Ref<Mesh> mesh = std::make_shared<Mesh>(model->GetVertexLayout(), model->GetVertices(), model->GetIndices());

        return mesh;
    }

    Ref<Mesh> Mesh::Create(const Model& model, const ModelPrimitive& primitive) {
        return Create(model, std::vector<const ModelPrimitive*>{&primitive});
    }

    Ref<Mesh> Mesh::Create(const Model& model, const std::vector<const ModelPrimitive*>& primitives) {
        if (primitives.empty()) {
            return nullptr;
        }

        if (primitives.front()->Skinned) {
            constexpr uint32_t kModelStride = VertexAttributeOffsets::kSkinnedVertexFloatCount;
            const auto& vertices            = model.GetSkinnedVertices();
            const auto& indices             = model.GetSkinnedIndices();

            std::vector<uint8_t> packed;
            std::vector<uint32_t> mergedIndices;
            uint32_t vertexBase = 0;

            for (const ModelPrimitive* primitive : primitives) {
                if (!primitive || primitive->vertexOffset + primitive->vertexCount > vertices.size() / kModelStride
                    || primitive->indexOffset + primitive->indexCount > indices.size()) {
                    return nullptr;
                }

                const size_t vertexStart = primitive->vertexOffset * kModelStride;
                for (size_t v = 0; v < primitive->vertexCount; ++v) {
                    const size_t source = vertexStart + v * kModelStride;

                    for (uint32_t i = 0; i < 17; ++i) {
                        const float value    = vertices[source + i];
                        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
                        packed.insert(packed.end(), bytes, bytes + sizeof(float));
                    }

                    for (uint32_t i = 0; i < 4; ++i) {
                        const float jointValue = vertices[source + VertexAttributeOffsets::Joints + i];
                        const uint16_t joint   = jointValue <= 0.0f ? 0u : static_cast<uint16_t>(glm::clamp(jointValue, 0.0f, 65535.0f));
                        const uint8_t* bytes   = reinterpret_cast<const uint8_t*>(&joint);
                        packed.insert(packed.end(), bytes, bytes + sizeof(uint16_t));
                    }

                    for (uint32_t i = 0; i < 4; ++i) {
                        const float value    = vertices[source + VertexAttributeOffsets::Weights + i];
                        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
                        packed.insert(packed.end(), bytes, bytes + sizeof(float));
                    }
                }

                for (size_t i = 0; i < primitive->indexCount; ++i) {
                    mergedIndices.push_back(indices[primitive->indexOffset + i] - static_cast<uint32_t>(primitive->vertexOffset)
                                            + vertexBase);
                }

                vertexBase += static_cast<uint32_t>(primitive->vertexCount);
            }

            return std::make_shared<Mesh>(SkinnedVertexLayout(), packed.data(), packed.size(), vertexBase, mergedIndices);
        }

        const size_t stride  = model.GetVertexLayout().Stride / sizeof(float);
        const auto& vertices = model.GetVertices();
        const auto& indices  = model.GetIndices();

        std::vector<float> mergedVertices;
        std::vector<uint32_t> mergedIndices;
        uint32_t vertexBase = 0;

        std::vector<float> primitiveVertices;
        for (const ModelPrimitive* primitive : primitives) {
            if (!primitive || primitive->vertexOffset + primitive->vertexCount > vertices.size() / stride
                || primitive->indexOffset + primitive->indexCount > indices.size()) {
                return nullptr;
            }

            primitiveVertices.assign(vertices.begin() + primitive->vertexOffset * stride,
                                     vertices.begin() + (primitive->vertexOffset + primitive->vertexCount) * stride);
            mergedVertices.insert(mergedVertices.end(), primitiveVertices.begin(), primitiveVertices.end());

            for (size_t i = 0; i < primitive->indexCount; ++i) {
                mergedIndices.push_back(indices[primitive->indexOffset + i] - static_cast<uint32_t>(primitive->vertexOffset) + vertexBase);
            }

            vertexBase += static_cast<uint32_t>(primitive->vertexCount);
        }

        return std::make_shared<Mesh>(model.GetVertexLayout(), mergedVertices, mergedIndices);
    }

    void Mesh::Bind() const {
        mVAO->Bind();
    }

    void Mesh::Update(const std::vector<float>& vertices, const std::vector<uint32_t>& indices) {
        mVertexCount = vertices.size() / (mVertexLayout.Stride / sizeof(float));
        mIndexCount  = indices.size();

        mVAO->Bind();
        mVBO->Update(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)));

        if (mEBO && mIndexCount > 0) {
            mEBO->Update(indices.data(), static_cast<uint32_t>(indices.size() * sizeof(uint32_t)));
            mEBO->Bind();
        }

        mVAO->Unbind();
    }

    void Mesh::Unbind() const {
        mVAO->Unbind();
    }

    void Mesh::Draw() const {
        FrameStats::RecordDrawCall(static_cast<uint32_t>(mVertexCount), static_cast<uint32_t>(mIndexCount));

        mVAO->Draw(static_cast<uint32_t>(mVertexCount), static_cast<uint32_t>(mIndexCount));
    }

    void Mesh::DrawIndexed(uint32_t start, uint32_t count) const {
        FrameStats::RecordDrawCall(count, count);

        mVAO->DrawIndexed(start, count);
    }

    void Mesh::DrawInstanced(const Ref<Buffer>& instanceBuffer, uint32_t instanceCount) const {
        if (instanceCount == 0) {
            return;
        }

        // Instanced model matrices stream through vertex attributes at locations 8-11, immediately
        // followed by the per-instance base color (location 12). The color attribute is always enabled
        // even for shaders that read only the matrix (e.g. shadow pass); an unused attribute is harmless.
        constexpr uint32_t kMatrixColumns = 4;

        const uint32_t matrixBytes = kMatrixColumns * sizeof(glm::vec4);
        const uint32_t stride      = matrixBytes + sizeof(glm::vec4);

        std::vector<VertexArray::InstanceAttribute> attributes;
        attributes.reserve(kMatrixColumns + 1);
        for (uint32_t column = 0; column < kMatrixColumns; ++column) {
            attributes.emplace_back(VertexArray::InstanceAttribute{VertexAttributeBinding::InstanceMatrix + column, 4, column * sizeof(glm::vec4)});
        }
        attributes.emplace_back(VertexArray::InstanceAttribute{VertexAttributeBinding::InstanceColor, 4, matrixBytes});

        FrameStats::RecordDrawCall(static_cast<uint32_t>(mVertexCount) * instanceCount, static_cast<uint32_t>(mIndexCount) * instanceCount);

        mVAO->SetInstanceBuffer(instanceBuffer, stride, attributes);
        mVAO->Bind();
        mVAO->DrawInstanced(instanceCount, static_cast<uint32_t>(mVertexCount), static_cast<uint32_t>(mIndexCount));
        mVAO->Unbind();
    }

    const AABB& Mesh::GetAABB() const {
        return mAABB;
    }

    Ref<Mesh> Mesh::CreateCube(const glm::vec3& size, uint32_t segments) {
        const ProceduralMeshKey key{ProceduralMeshKind::Cube, {size.x, size.y, size.z, static_cast<float>(segments)}};

        return Engine::GetInstance().GetAssetManager().AcquireProceduralMesh(key, [size, segments]() {

        const glm::vec3 halfSize = size * 0.5f;

        const float sx = size.x;
        const float sy = size.y;
        const float sz = size.z;

        // clang-format off
        std::vector<Vertex> vertices = {
            // Front (+Z)
            {{ halfSize.x,  halfSize.y,  halfSize.z},  {1.0f, 1.0f, 1.0f},  {sx,   sy },  {0.0f, 0.0f, 1.0f}},
            {{-halfSize.x,  halfSize.y,  halfSize.z},  {1.0f, 1.0f, 1.0f},  {0.0f, sy },  {0.0f, 0.0f, 1.0f}},
            {{-halfSize.x, -halfSize.y,  halfSize.z},  {1.0f, 1.0f, 1.0f},  {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{ halfSize.x, -halfSize.y,  halfSize.z},  {1.0f, 1.0f, 1.0f},  {sx,   0.0f}, {0.0f, 0.0f, 1.0f}},
            // Back (-Z)
            {{-halfSize.x,  halfSize.y, -halfSize.z},  {1.0f, 1.0f, 1.0f},  {0.0f, sy },  {0.0f, 0.0f, -1.0f}},
            {{ halfSize.x,  halfSize.y, -halfSize.z},  {1.0f, 1.0f, 1.0f},  {sx,   sy },  {0.0f, 0.0f, -1.0f}},
            {{ halfSize.x, -halfSize.y, -halfSize.z},  {1.0f, 1.0f, 1.0f},  {sx,   0.0f}, {0.0f, 0.0f, -1.0f}},
            {{-halfSize.x, -halfSize.y, -halfSize.z},  {1.0f, 1.0f, 1.0f},  {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
            // Top (+Y)
            {{-halfSize.x,  halfSize.y,  halfSize.z},  {1.0f, 1.0f, 1.0f},  {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{ halfSize.x,  halfSize.y,  halfSize.z},  {1.0f, 1.0f, 1.0f},  {sx,   0.0f}, {0.0f, 1.0f, 0.0f}},
            {{ halfSize.x,  halfSize.y, -halfSize.z},  {1.0f, 1.0f, 1.0f},  {sx,   sz },  {0.0f, 1.0f, 0.0f}},
            {{-halfSize.x,  halfSize.y, -halfSize.z},  {1.0f, 1.0f, 1.0f},  {0.0f, sz },  {0.0f, 1.0f, 0.0f}},
            // Bottom (-Y)
            {{-halfSize.x, -halfSize.y, -halfSize.z},  {1.0f, 1.0f, 1.0f},  {0.0f, sz },  {0.0f, -1.0f, 0.0f}},
            {{ halfSize.x, -halfSize.y, -halfSize.z},  {1.0f, 1.0f, 1.0f},  {sx,   sz },  {0.0f, -1.0f, 0.0f}},
            {{ halfSize.x, -halfSize.y,  halfSize.z},  {1.0f, 1.0f, 1.0f},  {sx,   0.0f}, {0.0f, -1.0f, 0.0f}},
            {{-halfSize.x, -halfSize.y,  halfSize.z},  {1.0f, 1.0f, 1.0f},  {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
            // Right (+X)
            {{ halfSize.x,  halfSize.y, -halfSize.z},  {1.0f, 1.0f, 1.0f},  {0.0f, sy },  {1.0f, 0.0f, 0.0f}},
            {{ halfSize.x,  halfSize.y,  halfSize.z},  {1.0f, 1.0f, 1.0f},  {sz,   sy },  {1.0f, 0.0f, 0.0f}},
            {{ halfSize.x, -halfSize.y,  halfSize.z},  {1.0f, 1.0f, 1.0f},  {sz,   0.0f}, {1.0f, 0.0f, 0.0f}},
            {{ halfSize.x, -halfSize.y, -halfSize.z},  {1.0f, 1.0f, 1.0f},  {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            // Left (-X)
            {{-halfSize.x,  halfSize.y,  halfSize.z},  {1.0f, 1.0f, 1.0f},  {0.0f, sy },  {-1.0f, 0.0f, 0.0f}},
            {{-halfSize.x,  halfSize.y, -halfSize.z},  {1.0f, 1.0f, 1.0f},  {sz,   sy },  {-1.0f, 0.0f, 0.0f}},
            {{-halfSize.x, -halfSize.y, -halfSize.z},  {1.0f, 1.0f, 1.0f},  {sz,   0.0f}, {-1.0f, 0.0f, 0.0f}},
            {{-halfSize.x, -halfSize.y,  halfSize.z},  {1.0f, 1.0f, 1.0f},  {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}}
        };
        // clang-format on


        std::vector<uint32_t> indices = {
            0,  1,  2,  0,  2,  3, // Front
            4,  5,  6,  4,  6,  7, // Back
            8,  9,  10, 8,  10, 11, // Top
            12, 13, 14, 12, 14, 15, // Bottom
            16, 17, 18, 16, 18, 19, // Right
            20, 21, 22, 20, 22, 23 // Left
        };
        //  clang-format on

        std::vector<float> fullVertices = flatten(vertices);
        generate_mesh_tangents(fullVertices, indices);

        const VertexLayout layout = StandardVertexLayout();

        Ref<Mesh> mesh = std::make_shared<Mesh>(layout, fullVertices, indices);

        return mesh;
        });
    }


    Ref<Mesh> Mesh::CreateQuad(const glm::vec2& size) {
        const ProceduralMeshKey key{ProceduralMeshKind::Quad, {size.x, size.y}};

        return Engine::GetInstance().GetAssetManager().AcquireProceduralMesh(key, [size]() {
        const glm::vec2 half = size * 0.5f;

        // clang-format off
        std::vector<Vertex> vertices = {
            {{ half.x,  half.y, 0.0f},  {1,1,1},  {1.0f, 1.0f},  {0,0,1}},
            {{-half.x,  half.y, 0.0f},  {1,1,1},  {0.0f, 1.0f},  {0,0,1}},
            {{-half.x, -half.y, 0.0f},  {1,1,1},  {0.0f, 0.0f},  {0,0,1}},
            {{ half.x, -half.y, 0.0f},  {1,1,1},  {1.0f, 0.0f},  {0,0,1}}
        };
        // clang-format on

        std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

        std::vector<float> fullVertices = flatten(vertices);
        generate_mesh_tangents(fullVertices, indices);

        const VertexLayout layout = StandardVertexLayout();

        Ref<Mesh> mesh = std::make_shared<Mesh>(layout, fullVertices, indices);

        return mesh;
        });
    }


    Ref<Mesh> Mesh::CreateSphere(float radius, uint32_t sectorCount, uint32_t stackCount) {
        const ProceduralMeshKey key{ProceduralMeshKind::Sphere, {radius, static_cast<float>(sectorCount), static_cast<float>(stackCount)}};

        return Engine::GetInstance().GetAssetManager().AcquireProceduralMesh(key, [radius, sectorCount, stackCount]() {

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t i = 0; i <= stackCount; ++i) {
            float stackAngle = glm::half_pi<float>() - (float) i * (glm::pi<float>() / stackCount); // +pi/2 -> -pi/2
            float xy         = radius * cosf(stackAngle);
            float z          = radius * sinf(stackAngle);

            for (uint32_t j = 0; j <= sectorCount; ++j) {
                float sectorAngle = (float) j * (glm::two_pi<float>() / sectorCount);

                float x = xy * cosf(sectorAngle);
                float y = xy * sinf(sectorAngle);

                glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
                float u          = (float) j / sectorCount;
                float v          = (float) i / stackCount;

                vertices.push_back({
                    {x, y, z},
                    {1.0f, 1.0f, 1.0f},
                    {u, v},
                    normal, {0.0f, 0.0f, 0.0f},
                    {0.0f, 0.0f, 0.0f}
                });
            }
        }

        for (uint32_t i = 0; i < stackCount; ++i) {
            uint32_t k1 = i * (sectorCount + 1);
            uint32_t k2 = k1 + sectorCount + 1;

            for (uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }
                if (i != (stackCount - 1)) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        std::vector<float> fullVertices = flatten(vertices);
        generate_mesh_tangents(fullVertices, indices);

        const VertexLayout layout = StandardVertexLayout();

        Ref<Mesh> mesh = std::make_shared<Mesh>(layout, fullVertices, indices);

        return mesh;
        });
    }


    Ref<Mesh> Mesh::CreateTorus(float majorRadius, float minorRadius, uint32_t majorSegments, uint32_t minorSegments) {
        const ProceduralMeshKey key{ProceduralMeshKind::Torus,
                                    {majorRadius, minorRadius, static_cast<float>(majorSegments), static_cast<float>(minorSegments)}};

        return Engine::GetInstance().GetAssetManager().AcquireProceduralMesh(
            key, [majorRadius, minorRadius, majorSegments, minorSegments]() {

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t i = 0; i <= majorSegments; ++i) {
            float theta = (float) i * (glm::two_pi<float>() / majorSegments);
            float cosT = cosf(theta), sinT = sinf(theta);

            for (uint32_t j = 0; j <= minorSegments; ++j) {
                float phi  = (float) j * (glm::two_pi<float>() / minorSegments);
                float cosP = cosf(phi), sinP = sinf(phi);

                float x = (majorRadius + minorRadius * cosP) * cosT;
                float z = (majorRadius + minorRadius * cosP) * sinT;
                float y = minorRadius * sinP;

                glm::vec3 normal = glm::normalize(glm::vec3(cosP * cosT, sinP, cosP * sinT));
                float u          = (float) i / majorSegments;
                float v          = (float) j / minorSegments;

                vertices.push_back({
                    {x, y, z},
                    {1.0f, 1.0f, 1.0f},
                    {u, v},
                    normal, {0.0f, 0.0f, 0.0f},
                    {0.0f, 0.0f, 0.0f}
                });
            }
        }

        for (uint32_t i = 0; i < majorSegments; ++i) {
            uint32_t k1 = i * (minorSegments + 1);
            uint32_t k2 = k1 + minorSegments + 1;

            for (uint32_t j = 0; j < minorSegments; ++j, ++k1, ++k2) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);

                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }

        std::vector<float> fullVertices = flatten(vertices);
        generate_mesh_tangents(fullVertices, indices);

        const VertexLayout layout = StandardVertexLayout();

        Ref<Mesh> mesh = std::make_shared<Mesh>(layout, fullVertices, indices);

        return mesh;
        });
    }


    Ref<Mesh> Mesh::CreateCylinder(float radiusTop, float radiusBottom, float height, uint32_t sectorCount) {
        const ProceduralMeshKey key{ProceduralMeshKind::Cylinder,
                                    {radiusTop, radiusBottom, height, static_cast<float>(sectorCount)}};

        return Engine::GetInstance().GetAssetManager().AcquireProceduralMesh(
            key, [radiusTop, radiusBottom, height, sectorCount]() {

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        float halfH = height * 0.5f;

        // Side surface
        for (uint32_t i = 0; i <= 1; ++i) {
            float y = (i == 0) ? halfH : -halfH;
            float r = (i == 0) ? radiusTop : radiusBottom;

            for (uint32_t j = 0; j <= sectorCount; ++j) {
                float angle = (float) j * (glm::two_pi<float>() / sectorCount);
                float x     = r * cosf(angle);
                float z     = r * sinf(angle);

                glm::vec3 normal = glm::normalize(glm::vec3(x, (radiusBottom - radiusTop) / height, z));
                float u          = (float) j / sectorCount;
                float v          = (float) i;

                vertices.push_back({
                    {x, y, z},
                    {1.0f, 1.0f, 1.0f},
                    {u, v},
                    normal, {0.0f, 0.0f, 0.0f},
                    {0.0f, 0.0f, 0.0f}
                });
            }
        }

        for (uint32_t j = 0; j < sectorCount; ++j) {
            uint32_t k1 = j;
            uint32_t k2 = k1 + sectorCount + 1;

            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k1 + 1);

            indices.push_back(k1 + 1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);
        }

        // Caps
        auto add_cap = [&](float y, float r, bool top) {
            uint32_t centerIdx = (uint32_t) vertices.size();
            glm::vec3 normal(0.0f, top ? 1.0f : -1.0f, 0.0f);

            vertices.push_back({
                {0.0f, y, 0.0f},
                {1, 1, 1},
                {0.5f, 0.5f},
                normal, {0, 0, 0},
                {0, 0, 0}
            });

            uint32_t startIdx = (uint32_t) vertices.size();
            for (uint32_t j = 0; j <= sectorCount; ++j) {
                float angle = (float) j * (glm::two_pi<float>() / sectorCount);
                float x     = r * cosf(angle);
                float z     = r * sinf(angle);
                float u     = 0.5f + 0.5f * cosf(angle);
                float v     = 0.5f + 0.5f * sinf(angle);

                vertices.push_back({
                    {x, y, z},
                    {1, 1, 1},
                    {u, v},
                    normal, {0, 0, 0},
                    {0, 0, 0}
                });
            }

            for (uint32_t j = 0; j < sectorCount; ++j) {
                if (top) {
                    indices.push_back(centerIdx);
                    indices.push_back(startIdx + j);
                    indices.push_back(startIdx + j + 1);
                } else {
                    indices.push_back(centerIdx);
                    indices.push_back(startIdx + j + 1);
                    indices.push_back(startIdx + j);
                }
            }
        };

        if (radiusTop > 0.0f) {
            add_cap(halfH, radiusTop, true);
        }
        if (radiusBottom > 0.0f) {
            add_cap(-halfH, radiusBottom, false);
        }

        std::vector<float> fullVertices = flatten(vertices);
        generate_mesh_tangents(fullVertices, indices);

        const VertexLayout layout = StandardVertexLayout();

        Ref<Mesh> mesh = std::make_shared<Mesh>(layout, fullVertices, indices);

        return mesh;
        });
    }

    Ref<Mesh> Mesh::CreateCone(float radius, float height, uint32_t sectorCount) {
        return CreateCylinder(0.0f, radius, height, sectorCount);
    }


    Ref<Mesh> Mesh::CreateCapsule(float radius, float cylinderHeight, uint32_t sectorCount, uint32_t hemisphereRings) {
        const ProceduralMeshKey key{ProceduralMeshKind::Capsule,
                                    {radius, cylinderHeight, static_cast<float>(sectorCount), static_cast<float>(hemisphereRings)}};

        return Engine::GetInstance().GetAssetManager().AcquireProceduralMesh(
            key, [radius, cylinderHeight, sectorCount, hemisphereRings]() {

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        float halfH           = cylinderHeight * 0.5f;
        uint32_t ringsPerHemi = hemisphereRings;

        // Top hemisphere
        for (uint32_t i = 0; i <= ringsPerHemi; ++i) {
            float stackAngle = glm::half_pi<float>() * (1.0f - (float) i / ringsPerHemi); // pi/2 -> 0
            float ringRadius = radius * cosf(stackAngle);
            float y          = halfH + radius * sinf(stackAngle);

            for (uint32_t j = 0; j <= sectorCount; ++j) {
                float angle      = (float) j * (glm::two_pi<float>() / sectorCount);
                float x          = ringRadius * cosf(angle);
                float z          = ringRadius * sinf(angle);
                glm::vec3 normal = glm::normalize(glm::vec3(x, radius * sinf(stackAngle), z));
                float u          = (float) j / sectorCount;
                float v          = (float) i / (ringsPerHemi * 2.0f + 1.0f);
                vertices.push_back({
                    {x, y, z},
                    {1, 1, 1},
                    {u, v},
                    normal, {0, 0, 0},
                    {0, 0, 0}
                });
            }
        }

        //  Bottom ring of cylinder
        for (uint32_t j = 0; j <= sectorCount; ++j) {
            float angle      = (float) j * (glm::two_pi<float>() / sectorCount);
            float x          = radius * cosf(angle);
            float z          = radius * sinf(angle);
            glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
            float u          = (float) j / sectorCount;
            vertices.push_back({
                {x, -halfH, z},
                {1, 1, 1},
                {u, 0.5f},
                normal, {0, 0, 0},
                {0, 0, 0}
            });
        }

        //  Bottom hemisphere
        for (uint32_t i = 0; i <= ringsPerHemi; ++i) {
            float stackAngle = glm::half_pi<float>() * ((float) i / ringsPerHemi); // 0 -> pi/2
            float ringRadius = radius * cosf(stackAngle);
            float y          = -halfH - radius * sinf(stackAngle);

            for (uint32_t j = 0; j <= sectorCount; ++j) {
                float angle      = (float) j * (glm::two_pi<float>() / sectorCount);
                float x          = ringRadius * cosf(angle);
                float z          = ringRadius * sinf(angle);
                glm::vec3 normal = glm::normalize(glm::vec3(x, -radius * sinf(stackAngle), z));
                float u          = (float) j / sectorCount;
                float v          = 0.5f + (float) (i + 1) / (ringsPerHemi * 2.0f + 2.0f);
                vertices.push_back({
                    {x, y, z},
                    {1, 1, 1},
                    {u, v},
                    normal, {0, 0, 0},
                    {0, 0, 0}
                });
            }
        }

        uint32_t ringStride = sectorCount + 1;
        uint32_t ringCount  = (ringsPerHemi + 1) + 1 + (ringsPerHemi + 1); // top hemi rings + cyl bottom ring + bottom hemi rings

        for (uint32_t i = 0; i < ringCount - 1; ++i) {
            uint32_t k1 = i * ringStride;
            uint32_t k2 = k1 + ringStride;

            for (uint32_t j = 0; j < sectorCount; ++j) {
                indices.push_back(k1 + j);
                indices.push_back(k2 + j);
                indices.push_back(k1 + j + 1);

                indices.push_back(k1 + j + 1);
                indices.push_back(k2 + j);
                indices.push_back(k2 + j + 1);
            }
        }

        std::vector<float> fullVertices = flatten(vertices);
        generate_mesh_tangents(fullVertices, indices);

        const VertexLayout layout = StandardVertexLayout();

        Ref<Mesh> mesh = std::make_shared<Mesh>(layout, fullVertices, indices);

        return mesh;
        });
    }

    Mesh::~Mesh() {
        delete mVAO;
    }

} // namespace golias
