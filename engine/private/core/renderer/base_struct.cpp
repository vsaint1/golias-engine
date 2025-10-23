#include "core/renderer/base_struct.h"

#include <core/engine.h>


Font::~Font() {
    if (_font) {
        TTF_CloseFont(_font);
        _font = nullptr;
    }
}

glm::vec2 Font::get_size(const std::string& text) {
    int w = 0, h = 0;

    if (_font && TTF_GetStringSize(_font, text.c_str(), text.size(), &w, &h) == 0) {
        return {float(w), float(h)};
    }

    return {0, 0};
}

TTF_Font* Font::get_font() const {
    return _font;
}

// in case i forget xd
Texture::~Texture() {
    if (pixels != nullptr) {
        free(pixels);
        pixels = nullptr;
    }
}

bool Material::is_valid() const {
    const bool albedo_valid = albedo_texture != nullptr && albedo_texture->is_valid();
    const bool normal_valid = normal_texture != nullptr && normal_texture->is_valid();

    const bool validated = albedo_valid || normal_valid;
    return validated;
}

void Material::bind() const {

    if (shader && shader->is_valid()) {

        shader->activate();

        shader->set_value("material.albedo", albedo, 1);
        shader->set_value("material.ao", ambient_occlusion.value);
        shader->set_value("material.metallic", metallic.value);
        shader->set_value("material.roughness", roughness);
    }

    if (albedo_texture && albedo_texture->is_valid()) {
        albedo_texture->bind(ALBEDO_TEXTURE_UNIT);
        if (shader && shader->is_valid()) {
            shader->set_value("ALBEDO_TEXTURE", ALBEDO_TEXTURE_UNIT);
            shader->set_value("USE_ALBEDO_TEXTURE", true);

        }
    } else {
        if (shader && shader->is_valid()) {
            shader->set_value("USE_ALBEDO_TEXTURE", false);
        }
    }

    if (normal_texture && normal_texture->is_valid()) {
        normal_texture->bind(NORMAL_MAP_TEXTURE_UNIT);
        if (shader && shader->is_valid()) {
            shader->set_value("NORMAL_MAP_TEXTURE", NORMAL_MAP_TEXTURE_UNIT);
            shader->set_value("USE_NORMAL_MAP_TEXTURE", true);
        }
    } else {
        if (shader && shader->is_valid()) {
            shader->set_value("USE_NORMAL_MAP_TEXTURE", false);
        }
    }

    if (metallic.texture && metallic.texture->is_valid()) {
        metallic.texture->bind(METALLIC_ROUGHNESS_TEXTURE_UNIT);
        if (shader && shader->is_valid()) {
            shader->set_value("METALLIC_ROUGHNESS_TEXTURE", METALLIC_ROUGHNESS_TEXTURE_UNIT);
            shader->set_value("USE_METALLIC_ROUGHNESS_TEXTURE", true);
        }
    } else {
        if (shader && shader->is_valid()) {
            shader->set_value("USE_METALLIC_ROUGHNESS_TEXTURE", false);
        }
    }

    if (ambient_occlusion.texture && ambient_occlusion.texture->is_valid()) {
        ambient_occlusion.texture->bind(AMBIENT_OCCLUSION_TEXTURE_UNIT);
        if (shader && shader->is_valid()) {
            shader->set_value("AO_TEXTURE", AMBIENT_OCCLUSION_TEXTURE_UNIT);
            shader->set_value("USE_AO_TEXTURE", true);
        }
    } else {
        if (shader && shader->is_valid()) {
            shader->set_value("USE_AO_TEXTURE", false);
        }
    }

}

// TODO: implement this function
void parse_material(aiMesh* ai_mesh, const aiScene* scene, const std::string& base_dir, Mesh& mesh_ref) {
}


void parse_meshes(aiMesh* ai_mesh, const aiScene* scene, const std::string& base_dir, Mesh& mesh_ref) {
    if (!ai_mesh || !scene) {
        return;
    }

    mesh_ref.name = ai_mesh->mName.C_Str();


    if (scene->mNumMaterials > ai_mesh->mMaterialIndex) {
        aiMaterial* mat = scene->mMaterials[ai_mesh->mMaterialIndex];

        aiColor3D kd(0.0f, 0.0f, 0.0f);
        mat->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
        mesh_ref.material->albedo = {kd.r, kd.g, kd.b};

        // **DEPRECATED** this shouldnt be used in PBR
        // aiColor3D ka(0.0f, 0.0f, 0.0f);
        // mat->Get(AI_MATKEY_COLOR_AMBIENT, ka);
        // mesh_ref.material->ambient = {ka.r, ka.g, ka.b};

        // aiColor3D ks(0.0f, 0.0f, 0.0f);
        // mat->Get(AI_MATKEY_COLOR_SPECULAR, ks);
        // mesh_ref.material->metallic.specular = {ks.r, ks.g, ks.b};

        float metalness = 0.0f;
        mat->Get(AI_MATKEY_METALLIC_FACTOR, metalness);
        mesh_ref.material->metallic.value = metalness;

        float roughness = 0.0f;
        mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
        mesh_ref.material->roughness = roughness;

    }

    std::vector<Vertex> verts;
    std::vector<Uint32> indices; // EBO

    verts.reserve(ai_mesh->mNumVertices);
    indices.reserve(ai_mesh->mNumFaces * 3);

    for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++) {
        const aiVector3D& v = ai_mesh->mVertices[i];
        Vertex vert;
        vert.position = glm::vec3(v.x, v.y, v.z);

        if (ai_mesh->HasNormals()) {
            vert.normal = glm::vec3(ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z);
        } else {
            vert.normal = glm::vec3(0);
        }

        if (ai_mesh->HasTextureCoords(0)) {
            vert.uv = glm::vec2(ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y);
        } else {
            vert.uv = glm::vec2(0);
        }

        verts.push_back(vert);
    }

    for (unsigned int i = 0; i < ai_mesh->mNumFaces; i++) {
        const aiFace& face = ai_mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    mesh_ref.vertex_count = ai_mesh->mNumVertices;
    mesh_ref.index_count  = indices.size();

    mesh_ref.vertices = std::move(verts);
    mesh_ref.indices  = std::move(indices);
}


void parse_bones(aiMesh* ai_mesh, std::vector<glm::ivec4>& bone_ids, std::vector<glm::vec4>& bone_weights, Mesh& mesh_ref) {


    // Process bone data if present
    if (ai_mesh->HasBones()) {
        mesh_ref.has_bones = true;


        bone_ids.resize(ai_mesh->mNumVertices, glm::ivec4(0));
        bone_weights.resize(ai_mesh->mNumVertices, glm::vec4(0.0f));


        std::vector<int> bone_counts(ai_mesh->mNumVertices, 0);

        LOG_DEBUG("Mesh '%s' has %u bones", mesh_ref.name.data(), ai_mesh->mNumBones);

        // Build bone map and extract bone data
        for (unsigned int i = 0; i < ai_mesh->mNumBones; i++) {
            aiBone* bone          = ai_mesh->mBones[i];
            std::string bone_name = bone->mName.C_Str();

            int bone_index = -1;
            if (mesh_ref.bone_map.find(bone_name) == mesh_ref.bone_map.end()) {
                bone_index                   = mesh_ref.bones.size();
                mesh_ref.bone_map[bone_name] = bone_index;

                Bone b;
                b.name = bone_name;

                // Store offset matrix (inverse bind pose)
                aiMatrix4x4 offset = bone->mOffsetMatrix;
                b.offset_matrix    = glm::transpose(glm::make_mat4(&offset.a1));

                mesh_ref.bones.push_back(b);
            } else {
                bone_index = mesh_ref.bone_map[bone_name];
            }


            for (unsigned int j = 0; j < bone->mNumWeights; j++) {
                unsigned int vertex_id = bone->mWeights[j].mVertexId;
                float weight           = bone->mWeights[j].mWeight;

                if (weight == 0.0f) {
                    continue;
                }

                if (vertex_id >= ai_mesh->mNumVertices) {
                    LOG_DEBUG("Bone '%s' references out-of-bounds vertex %u", bone_name.c_str(), vertex_id);
                    continue;
                }

                int slot = bone_counts[vertex_id];
                if (slot < 4) {
                    bone_ids[vertex_id][slot]     = bone_index;
                    bone_weights[vertex_id][slot] = weight;
                    bone_counts[vertex_id]++;
                } else {
                    LOG_DEBUG("Vertex %u has more than 4 bone influences (skipping)", vertex_id);
                }
            }
        }


        LOG_DEBUG("Mesh '%s': Vertices: %u | Indices: %u | Bones: %zu | Has Texture: %s", mesh_ref.name.data(), mesh_ref.vertex_count,
                  mesh_ref.index_count, mesh_ref.bones.size(), mesh_ref.material->is_valid() ? "Yes" : "No");
    } else {
        LOG_DEBUG("Mesh '%s': Vertices: %u | Indices: %u | Has Texture: %s (no bones)", mesh_ref.name.data(), mesh_ref.vertex_count,
                  mesh_ref.index_count, mesh_ref.material->is_valid() ? "Yes" : "No");
    }
}
