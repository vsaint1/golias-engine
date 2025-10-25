#include  "core/utility/obj_loader.h"

#include "core/io/assimp_io.h"

MeshInstance3D ObjectLoader::load_mesh(const std::string& path) {
    Model model = load_model(path, nullptr);
    return model.meshes.empty() ? MeshInstance3D() : model.meshes[0];
}

Model ObjectLoader::load_model(const std::string& path, Renderer* renderer) {
    Model model;

    std::string path_str = path;

    std::size_t pos = path_str.find("//");
    if (pos != std::string::npos && pos + 2 < path_str.size()) {
        path_str = path_str.substr(pos + 2);
    }

    std::string base_dir = ASSETS_PATH;

    size_t last_slash = path_str.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        base_dir += path_str.substr(0, last_slash + 1);
    }

    spdlog::info("Loading model: {} (base_dir: {})", path, base_dir);

    auto importer   = std::make_shared<Assimp::Importer>();
    std::string ext = path_str.substr(path_str.find_last_of('.') + 1);

    auto ioSystem = new SDLIOSystem(base_dir);
    importer->SetIOHandler(ioSystem);

    constexpr auto ASSIMP_FLAGS = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals
                                  | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;

    std::string filename = (last_slash != std::string::npos) ? path_str.substr(last_slash + 1) : path_str;

    const aiScene* scene = importer->ReadFile(filename, ASSIMP_FLAGS);


    spdlog::info("  Meshes: {}, Materials: {}, Animations: {}",
                 scene->mNumMeshes, scene->mNumMaterials, scene->mNumAnimations);

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* aiMesh = scene->mMeshes[i];
        spdlog::info("  Parsing Mesh({}) - Name {}", i, aiMesh->mName.C_Str());

        MeshInstance3D mesh = create_mesh(aiMesh);
        model.meshes.push_back(mesh);

        if (renderer) {
            Material material = load_material(scene, aiMesh, base_dir, *renderer);
            model.materials.push_back(material);
            spdlog::info("    Material [{}]: Albedo ({:.2f},{:.2f},{:.2f}) | Metallic {:.2f} | Roughness {:.2f} | AO {:.2f}",
                         aiMesh->mName.C_Str(), material.albedo.r, material.albedo.g, material.albedo.b,
                         material.metallic, material.roughness, material.ao);
        }

        if (aiMesh->HasBones()) {
            parse_bones(aiMesh, model);
        }
    }

    if (scene->HasAnimations()) {
        parse_animations(scene, model);
    }


    return model;
}

std::string ObjectLoader::get_directory(const std::string& path) {
    size_t found = path.find_last_of("/\\");
    return (found != std::string::npos) ? path.substr(0, found + 1) : "";
}

MeshInstance3D ObjectLoader::create_mesh(aiMesh* aiMesh) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    vertices.reserve(aiMesh->mNumVertices);

    for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i) {
        Vertex vertex;

        // Position
        vertex.position = {
            aiMesh->mVertices[i].x,
            aiMesh->mVertices[i].y,
            aiMesh->mVertices[i].z
        };

        // Normal
        if (aiMesh->HasNormals()) {
            vertex.normal = {
                aiMesh->mNormals[i].x,
                aiMesh->mNormals[i].y,
                aiMesh->mNormals[i].z
            };
        } else {
            vertex.normal = {0.0f, 0.0f, 0.0f};
        }

        // UV
        if (aiMesh->mTextureCoords[0]) {
            vertex.uv = {
                aiMesh->mTextureCoords[0][i].x,
                aiMesh->mTextureCoords[0][i].y
            };
        } else {
            vertex.uv = {0.0f, 0.0f};
        }

        vertices.push_back(vertex);
    }

    // Process indices
    for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i) {
        const aiFace& face = aiMesh->mFaces[i];
        indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
    }

    MeshInstance3D mesh;
    mesh.indexCount = indices.size();

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    spdlog::info("  Mesh created: {} vertices, {} triangles", aiMesh->mNumVertices, indices.size() / 3);
    return mesh;
}

Material ObjectLoader::load_material(const aiScene* scene, aiMesh* mesh, const std::string& directory, Renderer& renderer) {
    Material material;
    if (scene->mNumMaterials <= mesh->mMaterialIndex)
        return material;

    aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
    load_colors(aiMat, material);
    load_textures(aiMat, scene, directory, renderer, material);
    return material;
}

void ObjectLoader::load_colors(aiMaterial* aiMat, Material& mat) {
    aiColor3D color(1, 1, 1);
    aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    mat.albedo = {color.r, color.g, color.b};

    aiColor3D emissive(0, 0, 0);
    if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS)
        mat.emissive = {emissive.r, emissive.g, emissive.b};

    aiMat->Get(AI_MATKEY_EMISSIVE_INTENSITY, mat.emissiveStrength);
    aiMat->Get(AI_MATKEY_METALLIC_FACTOR, mat.metallic);
    aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, mat.roughness);
}

void ObjectLoader::load_textures(aiMaterial* aiMat, const aiScene* scene, const std::string& dir, Renderer& renderer, Material& mat) {
    auto load_tex = [&](aiTextureType type, GLuint& id, bool& flag, const char* name) {
        if (aiMat->GetTextureCount(type) == 0)
            return;

        aiString texPath;
        aiMat->GetTexture(type, 0, &texPath);
        std::string texStr = texPath.C_Str();

        if (texStr.empty())
            return;

        if (texStr[0] == '*') {
            int texIndex = std::atoi(texStr.c_str() + 1);
            if (texIndex >= 0 && texIndex < (int) scene->mNumTextures) {
                const aiTexture* embedded = scene->mTextures[texIndex];
                if (!embedded)
                    return;

                if (embedded->mHeight == 0) {
                    id = renderer.load_texture_from_memory(
                        reinterpret_cast<const unsigned char*>(embedded->pcData),
                        embedded->mWidth);
                } else {
                    id = renderer.load_texture_from_raw_data(
                        reinterpret_cast<const unsigned char*>(embedded->pcData),
                        embedded->mWidth, embedded->mHeight);
                }

                flag = (id != 0);
                if (flag)
                    spdlog::info("    Embedded Texture loaded: {}", name);
                return;
            }
        }

        std::string tex_dir = dir  + texStr;
        id               = renderer.load_texture_from_file(tex_dir);
        flag             = (id != 0);
        if (flag)
            spdlog::info("    Texture loaded [{}]: {}", name, tex_dir);
    };

    load_tex(aiTextureType_DIFFUSE, mat.albedoMap, mat.useAlbedoMap, "albedo");
    load_tex(aiTextureType_NORMALS, mat.normalMap, mat.useNormalMap, "normal");
    load_tex(aiTextureType_METALNESS, mat.metallicMap, mat.useMetallicMap, "metallic");
    load_tex(aiTextureType_DIFFUSE_ROUGHNESS, mat.roughnessMap, mat.useRoughnessMap, "roughness");
    load_tex(aiTextureType_AMBIENT_OCCLUSION, mat.aoMap, mat.useAOMap, "ao");
    load_tex(aiTextureType_EMISSIVE, mat.emissiveMap, mat.useEmissiveMap, "emissive");
}

void ObjectLoader::parse_bones(aiMesh* mesh, Model& model) {
    spdlog::info("    Bones: {}", mesh->mNumBones);
    for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
        aiBone* bone = mesh->mBones[i];
        spdlog::info("      - Bone {}: {}", i, bone->mName.C_Str());
    }
}

void ObjectLoader::parse_animations(const aiScene* scene, Model& model) {
    spdlog::info("  → Animations: {}", scene->mNumAnimations);
    for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
        const aiAnimation* anim = scene->mAnimations[i];
        spdlog::info("    Animation {}: {} | Duration: {:.2f} | FPS: {}",
                     i, anim->mName.C_Str(), anim->mDuration, anim->mTicksPerSecond);
    }
}
