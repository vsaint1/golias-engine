#include "render/mesh.h"

#include "core/engine.h"
#include "render/model.h"

namespace golias {

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices) {
        mVertexLayout = layout;
        mVertexCount  = vertices.size() / (layout.Stride / sizeof(float));
        mIndexCount   = indices.size();

        for (const auto& element : layout.Elements) {
            if (element.Index != 0 || element.Size < 3) {
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

        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        mVBO = device.CreateVertexBuffer(vertices);

        if (!indices.empty()) {
            mEBO = device.CreateIndexBuffer(indices);
        }

        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);

        for (const auto& element : layout.Elements) {
            glVertexAttribPointer(element.Index, element.Size, element.Type, GL_FALSE, layout.Stride, (void*) (uintptr_t) (element.Offset));
            glEnableVertexAttribArray(element.Index);
        }

        if (mEBO) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& vertices) : Mesh(layout, vertices, {}) {
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
        const size_t stride  = model.GetVertexLayout().Stride / sizeof(float);
        const auto& vertices = model.GetVertices();
        const auto& indices  = model.GetIndices();
        if (primitive.vertexOffset + primitive.vertexCount > vertices.size() / stride
            || primitive.indexOffset + primitive.indexCount > indices.size()) {
            return nullptr;
        }

        std::vector<float> primitiveVertices(vertices.begin() + primitive.vertexOffset * stride,
                                             vertices.begin() + (primitive.vertexOffset + primitive.vertexCount) * stride);

        std::vector<uint32_t> primitiveIndices;
        primitiveIndices.reserve(primitive.indexCount);

        for (size_t i = 0; i < primitive.indexCount; ++i) {
            primitiveIndices.push_back(indices[primitive.indexOffset + i] - static_cast<uint32_t>(primitive.vertexOffset));
        }

        return std::make_shared<Mesh>(model.GetVertexLayout(), primitiveVertices, primitiveIndices);
    }

    void Mesh::Bind() const {
        glBindVertexArray(mVAO);
    }

    void Mesh::Unbind() const {
        glBindVertexArray(0);
    }

    void Mesh::Draw() const {
        if (mIndexCount) {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndexCount), GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mVertexCount));
        }
    }

    const AABB& Mesh::GetAABB() const {
        return mAABB;
    }

    Ref<Mesh> Mesh::CreateCube(const glm::vec3& size, uint32_t segments) {
        const glm::vec3 halfSize = size * 0.5f;

        const float sx = size.x;
        const float sy = size.y;
        const float sz = size.z;

        // clang-format off
        std::vector<float> vertices = {
            // Front (+Z)
            halfSize.x,  halfSize.y,  halfSize.z,   1.0f, 1.0f, 1.0f,  sx,   sy,   0.0f, 0.0f, 1.0f,
            -halfSize.x,  halfSize.y,  halfSize.z,  1.0f, 1.0f, 1.0f,  0.0f, sy,   0.0f, 0.0f, 1.0f,
            -halfSize.x, -halfSize.y,  halfSize.z,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            halfSize.x, -halfSize.y,  halfSize.z,   1.0f, 1.0f, 1.0f,  sx,   0.0f, 0.0f, 0.0f, 1.0f,
            // Back (-Z)
            -halfSize.x,  halfSize.y, -halfSize.z,  1.0f, 1.0f, 1.0f,  0.0f, sy,   0.0f, 0.0f, -1.0f,
            halfSize.x,  halfSize.y, -halfSize.z,   1.0f, 1.0f, 1.0f,  sx,   sy,   0.0f, 0.0f, -1.0f,
            halfSize.x, -halfSize.y, -halfSize.z,   1.0f, 1.0f, 1.0f,  sx,   0.0f, 0.0f, 0.0f, -1.0f,
            -halfSize.x, -halfSize.y, -halfSize.z,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
            // Top (+Y)
            -halfSize.x,  halfSize.y,  halfSize.z,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            halfSize.x,  halfSize.y,  halfSize.z,   1.0f, 1.0f, 1.0f,  sx,   0.0f, 0.0f, 1.0f, 0.0f,
            halfSize.x,  halfSize.y, -halfSize.z,   1.0f, 1.0f, 1.0f,  sx,   sz,   0.0f, 1.0f, 0.0f,
            -halfSize.x,  halfSize.y, -halfSize.z,  1.0f, 1.0f, 1.0f,  0.0f, sz,   0.0f, 1.0f, 0.0f,
            // Bottom (-Y)
            -halfSize.x, -halfSize.y, -halfSize.z,  1.0f, 1.0f, 1.0f,  0.0f, sz,   0.0f, -1.0f, 0.0f,
            halfSize.x, -halfSize.y, -halfSize.z,   1.0f, 1.0f, 1.0f,  sx,   sz,   0.0f, -1.0f, 0.0f,
            halfSize.x, -halfSize.y,  halfSize.z,   1.0f, 1.0f, 1.0f,  sx,   0.0f, 0.0f, -1.0f, 0.0f,
            -halfSize.x, -halfSize.y,  halfSize.z,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
            // Right (+X)
            halfSize.x,  halfSize.y, -halfSize.z,   1.0f, 1.0f, 1.0f,  0.0f, sy,   1.0f, 0.0f, 0.0f,
            halfSize.x,  halfSize.y,  halfSize.z,   1.0f, 1.0f, 1.0f,  sz,   sy,   1.0f, 0.0f, 0.0f,
            halfSize.x, -halfSize.y,  halfSize.z,   1.0f, 1.0f, 1.0f,  sz,   0.0f, 1.0f, 0.0f, 0.0f,
            halfSize.x, -halfSize.y, -halfSize.z,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            // Left (-X)
            -halfSize.x,  halfSize.y,  halfSize.z,  1.0f, 1.0f, 1.0f,  0.0f, sy,   -1.0f, 0.0f, 0.0f,
            -halfSize.x,  halfSize.y, -halfSize.z,  1.0f, 1.0f, 1.0f,  sz,   sy,   -1.0f, 0.0f, 0.0f,
            -halfSize.x, -halfSize.y, -halfSize.z,  1.0f, 1.0f, 1.0f,  sz,   0.0f, -1.0f, 0.0f, 0.0f,
            -halfSize.x, -halfSize.y,  halfSize.z,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f
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

        VertexLayout layout;
        layout.Elements.push_back({0, 3, GL_FLOAT, 0}); // Position
        layout.Elements.push_back({1, 3, GL_FLOAT, 3 * sizeof(float)}); // Color
        layout.Elements.push_back({2, 2, GL_FLOAT, 6 * sizeof(float)}); // TexCoord
        layout.Elements.push_back({3, 3, GL_FLOAT, 8 * sizeof(float)}); // Normal
        layout.Stride = 11 * sizeof(float);

        Ref<Mesh> mesh = std::make_shared<Mesh>(layout, vertices, indices);

        return mesh;
    }


    Ref<Mesh> Mesh::CreateQuad(const glm::vec2& size) {
        const glm::vec2 half = size * 0.5f;

        // clang-format off
        std::vector<float> vertices = {
            half.x,  half.y, 0.0f,  1,1,1,  1.0f, 1.0f,  0,0,1,
            -half.x,  half.y, 0.0f,  1,1,1,  0.0f, 1.0f,  0,0,1,
            -half.x, -half.y, 0.0f,  1,1,1,  0.0f, 0.0f,  0,0,1,
            half.x, -half.y, 0.0f,  1,1,1,  1.0f, 0.0f,  0,0,1
        };
        // clang-format on

        std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

        VertexLayout layout;
        layout.Elements.push_back({0, 3, GL_FLOAT, 0});
        layout.Elements.push_back({1, 3, GL_FLOAT, 3 * sizeof(float)});
        layout.Elements.push_back({2, 2, GL_FLOAT, 6 * sizeof(float)});
        layout.Elements.push_back({3, 3, GL_FLOAT, 8 * sizeof(float)});
        layout.Stride = 11 * sizeof(float);

        return std::make_shared<Mesh>(layout, vertices, indices);
    }


    Ref<Mesh> Mesh::CreateSphere(float radius, uint32_t sectorCount, uint32_t stackCount) {
        std::vector<float> vertices;
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

                vertices.insert(vertices.end(), {x, y, z, 1.0f, 1.0f, 1.0f, u, v, normal.x, normal.y, normal.z});
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

        VertexLayout layout;
        layout.Elements.push_back({0, 3, GL_FLOAT, 0});
        layout.Elements.push_back({1, 3, GL_FLOAT, 3 * sizeof(float)});
        layout.Elements.push_back({2, 2, GL_FLOAT, 6 * sizeof(float)});
        layout.Elements.push_back({3, 3, GL_FLOAT, 8 * sizeof(float)});
        layout.Stride = 11 * sizeof(float);

        return std::make_shared<Mesh>(layout, vertices, indices);
    }


    Ref<Mesh> Mesh::CreateTorus(float majorRadius, float minorRadius, uint32_t majorSegments, uint32_t minorSegments) {
        std::vector<float> vertices;
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

                vertices.insert(vertices.end(), {x, y, z, 1.0f, 1.0f, 1.0f, u, v, normal.x, normal.y, normal.z});
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

        VertexLayout layout;
        layout.Elements.push_back({0, 3, GL_FLOAT, 0});
        layout.Elements.push_back({1, 3, GL_FLOAT, 3 * sizeof(float)});
        layout.Elements.push_back({2, 2, GL_FLOAT, 6 * sizeof(float)});
        layout.Elements.push_back({3, 3, GL_FLOAT, 8 * sizeof(float)});
        layout.Stride = 11 * sizeof(float);

        return std::make_shared<Mesh>(layout, vertices, indices);
    }


    Ref<Mesh> Mesh::CreateCylinder(float radiusTop, float radiusBottom, float height, uint32_t sectorCount) {
        std::vector<float> vertices;
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

                vertices.insert(vertices.end(), {x, y, z, 1.0f, 1.0f, 1.0f, u, v, normal.x, normal.y, normal.z});
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
            uint32_t centerIdx = (uint32_t) (vertices.size() / 11);
            glm::vec3 normal(0.0f, top ? 1.0f : -1.0f, 0.0f);

            vertices.insert(vertices.end(), {0.0f, y, 0.0f, 1, 1, 1, 0.5f, 0.5f, normal.x, normal.y, normal.z});

            uint32_t startIdx = (uint32_t) (vertices.size() / 11);
            for (uint32_t j = 0; j <= sectorCount; ++j) {
                float angle = (float) j * (glm::two_pi<float>() / sectorCount);
                float x     = r * cosf(angle);
                float z     = r * sinf(angle);
                float u     = 0.5f + 0.5f * cosf(angle);
                float v     = 0.5f + 0.5f * sinf(angle);

                vertices.insert(vertices.end(), {x, y, z, 1, 1, 1, u, v, normal.x, normal.y, normal.z});
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

        VertexLayout layout;
        layout.Elements.push_back({0, 3, GL_FLOAT, 0});
        layout.Elements.push_back({1, 3, GL_FLOAT, 3 * sizeof(float)});
        layout.Elements.push_back({2, 2, GL_FLOAT, 6 * sizeof(float)});
        layout.Elements.push_back({3, 3, GL_FLOAT, 8 * sizeof(float)});
        layout.Stride = 11 * sizeof(float);

        return std::make_shared<Mesh>(layout, vertices, indices);
    }

    Ref<Mesh> Mesh::CreateCone(float radius, float height, uint32_t sectorCount) {
        return CreateCylinder(0.0f, radius, height, sectorCount);
    }


    Ref<Mesh> Mesh::CreateCapsule(float radius, float cylinderHeight, uint32_t sectorCount, uint32_t hemisphereRings) {
        std::vector<float> vertices;
        std::vector<uint32_t> indices;

        float halfH           = cylinderHeight * 0.5f;
        uint32_t ringsPerHemi = hemisphereRings;
        uint32_t totalRings   = ringsPerHemi * 2 + 1; // +1 for the middle seam is implicit via two cylinder rows

        auto add_ring = [&](float y, float ringRadius, glm::vec3 normalDir, float v) {
            for (uint32_t j = 0; j <= sectorCount; ++j) {
                float angle      = (float) j * (glm::two_pi<float>() / sectorCount);
                float x          = ringRadius * cosf(angle);
                float z          = ringRadius * sinf(angle);
                glm::vec3 normal = glm::normalize(
                    glm::vec3(normalDir.x != 0 || normalDir.z != 0 ? x : 0, normalDir.y, normalDir.x != 0 || normalDir.z != 0 ? z : 0));

                (void) normal;
                float u = (float) j / sectorCount;
                vertices.insert(vertices.end(), {x, y, z, 1.0f, 1.0f, 1.0f, u, v, 0, 0, 0});
            }
        };

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
                vertices.insert(vertices.end(), {x, y, z, 1, 1, 1, u, v, normal.x, normal.y, normal.z});
            }
        }

        //  Bottom ring of cylinder
        for (uint32_t j = 0; j <= sectorCount; ++j) {
            float angle      = (float) j * (glm::two_pi<float>() / sectorCount);
            float x          = radius * cosf(angle);
            float z          = radius * sinf(angle);
            glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
            float u          = (float) j / sectorCount;
            vertices.insert(vertices.end(), {x, -halfH, z, 1, 1, 1, u, 0.5f, normal.x, normal.y, normal.z});
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
                vertices.insert(vertices.end(), {x, y, z, 1, 1, 1, u, v, normal.x, normal.y, normal.z});
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

        (void) totalRings;

        VertexLayout layout;
        layout.Elements.push_back({0, 3, GL_FLOAT, 0});
        layout.Elements.push_back({1, 3, GL_FLOAT, 3 * sizeof(float)});
        layout.Elements.push_back({2, 2, GL_FLOAT, 6 * sizeof(float)});
        layout.Elements.push_back({3, 3, GL_FLOAT, 8 * sizeof(float)});
        layout.Stride = 11 * sizeof(float);

        return std::make_shared<Mesh>(layout, vertices, indices);
    }

    Mesh::~Mesh() {
        if (mVAO) {
            glDeleteVertexArrays(1, &mVAO);
        }

        if (mVBO) {
            glDeleteBuffers(1, &mVBO);
        }

        if (mEBO) {
            glDeleteBuffers(1, &mEBO);
        }
    }

} // namespace golias
