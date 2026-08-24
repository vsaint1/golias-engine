#include "render/model.h"

#include "core/engine.h"
#include <tiny_gltf.h>
#include <tiny_obj_loader.h>

namespace golias {

    namespace {
        constexpr size_t kVertexStride = 11;

        VertexLayout model_layout() {
            VertexLayout layout;
            layout.Elements = {
                {0, 3, GL_FLOAT, 0                },
                {1, 3, GL_FLOAT, 3 * sizeof(float)},
                {2, 2, GL_FLOAT, 6 * sizeof(float)},
                {3, 3, GL_FLOAT, 8 * sizeof(float)},
            };

            layout.Stride = kVertexStride * sizeof(float);

            return layout;
        }

        const unsigned char* accessor_data(const tinygltf::Model& model, const tinygltf::Accessor& accessor, int& stride, int& components) {
            if (accessor.bufferView < 0 || accessor.bufferView >= static_cast<int>(model.bufferViews.size())) {
                return nullptr;
            }

            const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
            if (view.buffer < 0 || view.buffer >= static_cast<int>(model.buffers.size())) {
                return nullptr;
            }

            stride     = accessor.ByteStride(view);
            components = tinygltf::GetNumComponentsInType(static_cast<uint32_t>(accessor.type));
            if (stride <= 0 || components <= 0) {
                return nullptr;
            }

            const auto& buffer  = model.buffers[view.buffer];
            const size_t offset = view.byteOffset + accessor.byteOffset;
            if (offset > buffer.data.size()
                || (accessor.count > 0 && (accessor.count - 1) * static_cast<size_t>(stride) > buffer.data.size() - offset)) {
                return nullptr;
            }

            return buffer.data.data() + offset;
        }

        float component(const unsigned char* data, int type, bool normalized) {
            switch (type) {
            case TINYGLTF_COMPONENT_TYPE_FLOAT:
                return *reinterpret_cast<const float*>(data);
            case TINYGLTF_COMPONENT_TYPE_BYTE:
                return normalized
                         ? std::max(-1.0f, static_cast<float>(*reinterpret_cast<const int8_t*>(data)) / std::numeric_limits<int8_t>::max())
                         : static_cast<float>(*reinterpret_cast<const int8_t*>(data));
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                return normalized ? static_cast<float>(*data) / 255.0f : static_cast<float>(*data);
            case TINYGLTF_COMPONENT_TYPE_SHORT:
                return normalized
                         ? std::max(-1.0f,
                                    static_cast<float>(*reinterpret_cast<const int16_t*>(data)) / std::numeric_limits<int16_t>::max())
                         : static_cast<float>(*reinterpret_cast<const int16_t*>(data));
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                return normalized ? static_cast<float>(*reinterpret_cast<const uint16_t*>(data)) / std::numeric_limits<uint16_t>::max()
                                  : static_cast<float>(*reinterpret_cast<const uint16_t*>(data));
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                return static_cast<float>(*reinterpret_cast<const uint32_t*>(data));
            default:
                return 0.0f;
            }
        }

        bool attribute(const tinygltf::Model& model, int index, size_t vertex, int expected, float* output) {
            if (index < 0 || index >= static_cast<int>(model.accessors.size())) {
                return false;
            }

            const auto& accessor      = model.accessors[index];
            int stride                = 0;
            int components            = 0;
            const unsigned char* data = accessor_data(model, accessor, stride, components);
            if (!data || vertex >= accessor.count || components < expected) {
                return false;
            }

            const size_t size = tinygltf::GetComponentSizeInBytes(static_cast<uint32_t>(accessor.componentType));
            data += vertex * static_cast<size_t>(stride);
            for (int i = 0; i < expected; ++i) {
                output[i] = component(data + i * size, accessor.componentType, accessor.normalized);
            }

            return true;
        }

        bool index_value(const tinygltf::Model& model, const tinygltf::Accessor& accessor, size_t index, uint32_t& value) {
            int stride                = 0;
            int components            = 0;
            const unsigned char* data = accessor_data(model, accessor, stride, components);
            if (!data || components != 1 || index >= accessor.count) {
                return false;
            }

            data += index * static_cast<size_t>(stride);
            switch (accessor.componentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                value = *data;
                return true;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                value = *reinterpret_cast<const uint16_t*>(data);
                return true;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                value = *reinterpret_cast<const uint32_t*>(data);
                return true;
            default:
                return false;
            }
        }

        // https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html
        bool is_json_gltf(const std::vector<char>& data) {
            const Json json = Json::parse(data.begin(), data.end());
            return json.contains("asset") && json["asset"].is_object() && json["asset"].contains("version");
        }

        // https://code.blender.org/2013/08/fbx-binary-file-format-specification/
        bool is_fbx(const std::vector<char>& data) {
            static constexpr char magic[] = "Kaydara FBX Binary";
            if (data.size() >= sizeof(magic) - 1 && std::equal(magic, magic + sizeof(magic) - 1, data.begin())) {
                return true;
            }

            const std::string text(data.begin(), data.end());
            return text.find("FBXHeaderExtension") != std::string::npos;
        }

        // https://en.wikipedia.org/wiki/Wavefront_.obj_file#File_format
        bool is_obj(const std::vector<char>& data) {
            std::istringstream stream(std::string(data.begin(), data.end()));
            std::string line;
            while (std::getline(stream, line)) {
                const auto first = line.find_first_not_of(" \t\r");
                if (first == std::string::npos) {
                    continue;
                }

                const std::string token = line.substr(first, line.find_first_of(" \t", first) - first);
                return token == "#" || token == "v" || token == "vn" || token == "vt" || token == "f" || token == "o" || token == "g"
                    || token == "s" || token == "mtllib" || token == "usemtl";
            }

            return false;
        }

        // workaround fixing the coordinate system from right-handed to left-handed
        void convert_to_left_handed(float* position, float* normal) {
            position[2] = -position[2];
            normal[2] = -normal[2];
        }

        // workaround reversing the triangle winding order for left-handed coordinate system
        void reverse_triangle_winding(std::vector<uint32_t>& indices, size_t start) {
            for (size_t i = start; i + 2 < indices.size(); i += 3) {
                std::swap(indices[i + 1], indices[i + 2]);
            }
        }
    } // namespace

    Ref<Model> Model::Load(CString path) {
        FileSystem& fileSystem = Engine::GetInstance().GetFileSystem();
        const std::string pathString(path);
        const std::vector<char> data = fileSystem.LoadAssetFile(path);
        if (data.empty()) {
            return nullptr;
        }

        if (is_fbx(data)) {
            GOLIAS_LOG_ERROR("FBX model loading is unimplemented: %s", pathString.c_str());
            return nullptr;
        }

        if (data.size() >= 4 && std::equal(data.begin(), data.begin() + 4, "glTF")) {
            return std::make_shared<GltfModel>(data, path);
        }

        if (is_json_gltf(data)) {
            return std::make_shared<GltfModel>(data, path);
        }

        if (is_obj(data)) {
            return std::make_shared<ObjModel>(data, path);
        }

        GOLIAS_LOG_ERROR("Unknown model format: %s", pathString.c_str());
        return nullptr;
    }


    const VertexLayout& Model::GetVertexLayout() const {
        return mVertexLayout;
    }

    const std::vector<float>& Model::GetVertices() const {
        return mVertices;
    }

    const std::vector<uint32_t>& Model::GetIndices() const {
        return mIndices;
    }

    void Model::AppendVertex(const float* position, const float* color, const float* texCoord, const float* normal) {
        mVertices.insert(mVertices.end(), position, position + 3);
        mVertices.insert(mVertices.end(), color, color + 3);
        mVertices.insert(mVertices.end(), texCoord, texCoord + 2);
        mVertices.insert(mVertices.end(), normal, normal + 3);
    }

    GltfModel::GltfModel(const std::vector<char>& data, CString path) {
        mVertexLayout = model_layout();
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string error;
        std::string warning;
        const Path assetPath{std::string(path)};
        bool loaded = false;

        if (data.size() >= 4 && std::equal(data.begin(), data.begin() + 4, "glTF")) {
            loaded =
                loader.LoadBinaryFromMemory(&model, &error, &warning, reinterpret_cast<const unsigned char*>(data.data()), data.size(), "");
        } else {
            const std::string source(data.data(), data.size());
            const Path base = Engine::GetInstance().GetFileSystem().GetAssetsFolder() / assetPath.parent_path();
            loaded          = loader.LoadASCIIFromString(&model, &error, &warning, source.c_str(), source.size(), base.string());
        }

        if (!loaded) {
            GOLIAS_LOG_ERROR("Failed to parse glTF '%s': %s", std::string(path).c_str(), error.c_str());
            return;
        }

        for (const auto& mesh : model.meshes) {
            for (const auto& primitive : mesh.primitives) {
                if (primitive.mode != -1 && primitive.mode != TINYGLTF_MODE_TRIANGLES) {
                    continue;
                }

                const auto position = primitive.attributes.find("POSITION");
                if (position == primitive.attributes.end()) {
                    continue;
                }

                const size_t base  = mVertices.size() / kVertexStride;
                const size_t count = model.accessors[position->second].count;
                for (size_t i = 0; i < count; ++i) {
                    float position_value[3]  = {};
                    float color_value[3]     = {1.0f, 1.0f, 1.0f};
                    float tex_coord_value[2] = {};
                    float normal_value[3]    = {0.0f, 1.0f, 0.0f};

                    attribute(model, position->second, i, 3, position_value);
                    auto color_attribute = primitive.attributes.find("COLOR_0");

                    if (color_attribute != primitive.attributes.end()) {
                        attribute(model, color_attribute->second, i, 3, color_value);
                    }

                    auto tex_coord_attribute = primitive.attributes.find("TEXCOORD_0");
                    if (tex_coord_attribute != primitive.attributes.end()) {
                        attribute(model, tex_coord_attribute->second, i, 2, tex_coord_value);
                    }

                    auto normal_attribute = primitive.attributes.find("NORMAL");
                    if (normal_attribute != primitive.attributes.end()) {
                        attribute(model, normal_attribute->second, i, 3, normal_value);
                    }

                    convert_to_left_handed(position_value, normal_value);
                    AppendVertex(position_value, color_value, tex_coord_value, normal_value);
                }

                const size_t index_start = mIndices.size();
                if (primitive.indices >= 0) {
                    const auto& accessor = model.accessors[primitive.indices];
                    for (size_t i = 0; i < accessor.count; ++i) {
                        uint32_t loaded_index = 0;
                        if (index_value(model, accessor, i, loaded_index)) {
                            mIndices.push_back(static_cast<uint32_t>(base) + loaded_index);
                        }
                    }
                } else {
                    for (size_t i = 0; i < count; ++i) {
                        mIndices.push_back(static_cast<uint32_t>(base + i));
                    }
                }

                reverse_triangle_winding(mIndices, index_start);
            }
        }
    }

    ObjModel::ObjModel(const std::vector<char>& data, CString path) {
        mVertexLayout = model_layout();
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string error;
        const std::string source(data.data(), data.size());
        std::istringstream stream(source);
        const Path base = Engine::GetInstance().GetFileSystem().GetAssetsFolder() / Path(path).parent_path();
        tinyobj::MaterialFileReader materialReader(base.string());

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &error, &stream, &materialReader, true)) {
            GOLIAS_LOG_ERROR("Failed to parse OBJ '%s': %s", path.data(), error.c_str());
            return;
        }

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                float position_value[3]  = {};
                float color_value[3]     = {1.0f, 1.0f, 1.0f};
                float tex_coord_value[2] = {};
                float normal_value[3]    = {0.0f, 1.0f, 0.0f};

                if (index.vertex_index >= 0) {
                    for (int i = 0; i < 3; ++i) {
                        position_value[i] = attrib.vertices[3 * index.vertex_index + i];
                    }
                }

                if (index.normal_index >= 0) {
                    for (int i = 0; i < 3; ++i) {
                        normal_value[i] = attrib.normals[3 * index.normal_index + i];
                    }
                }

                if (index.texcoord_index >= 0) {
                    for (int i = 0; i < 2; ++i) {
                        tex_coord_value[i] = attrib.texcoords[2 * index.texcoord_index + i];
                    }
                }

                convert_to_left_handed(position_value, normal_value);
                AppendVertex(position_value, color_value, tex_coord_value, normal_value);
                mIndices.push_back(static_cast<uint32_t>(mIndices.size()));
            }
        }

        reverse_triangle_winding(mIndices, 0);
    }

} // namespace golias
