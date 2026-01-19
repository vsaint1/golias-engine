#include "core/model.h"

#include "core/engine.h"
#include "core/graphics/material.h"
#include "core/graphics/structs.h"
#include "scene/3d/animation_component.h"
#include "scene/3d/mesh_component.h"
#include "scene/3d/skeleton_animation_component.h"
#include "scene/game_object.h"
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <unordered_map>
#include <unordered_set>

#include <assimp/Importer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace golias {

    static std::unordered_map<std::string, int> g_boneNameToIndex;

    // ============================================================================
    // Utility Functions
    // ============================================================================

    glm::mat4 AiMatrix4x4ToGlm(const aiMatrix4x4& from) {
        return glm::transpose(glm::make_mat4(&from.a1));
    }

    glm::vec3 AiVector3DToGlm(const aiVector3D& vec) {
        return glm::vec3(vec.x, vec.y, vec.z);
    }

    glm::quat AiQuaternionToGlm(const aiQuaternion& q) {
        return glm::quat(q.w, q.x, q.y, q.z);
    }

    // ============================================================================
    // Vertex Layout Creation
    // ============================================================================

    VertexLayout CreateStandardVertexLayout() {
        VertexLayout layout;
        Uint32 offset = 0;

        layout.elements.push_back({0, 3, EDataType::FLOAT, false, offset});
        offset += 12;

        layout.elements.push_back({1, 3, EDataType::FLOAT, false, offset});
        offset += 12;

        layout.elements.push_back({2, 2, EDataType::FLOAT, false, offset});
        offset += 8;

        layout.elements.push_back({3, 3, EDataType::FLOAT, false, offset});
        offset += 12;

        layout.stride = offset;
        return layout;
    }

    VertexLayout CreateSkinnedVertexLayout() {
        VertexLayout layout;
        Uint32 offset = 0;

        // Position
        layout.elements.push_back({0, 3, EDataType::FLOAT, false, offset});
        offset += 12;

        // Color
        layout.elements.push_back({1, 3, EDataType::FLOAT, false, offset});
        offset += 12;

        // TexCoord
        layout.elements.push_back({2, 2, EDataType::FLOAT, false, offset});
        offset += 8;

        // Normal
        layout.elements.push_back({3, 3, EDataType::FLOAT, false, offset});
        offset += 12;

        // Bone Indices (4 floats)
        layout.elements.push_back({4, 4, EDataType::FLOAT, false, offset});
        offset += 16;

        // Bone Weights (4 floats)
        layout.elements.push_back({5, 4, EDataType::FLOAT, false, offset});
        offset += 16;

        layout.stride = offset;
        return layout;
    }

    // ============================================================================
    // Texture Loading
    // ============================================================================

    std::shared_ptr<Texture2D> LoadAssimpTexture(const aiMaterial* material,
                                                 aiTextureType type,
                                                 const std::string& base_path,
                                                 const aiScene* scene,
                                                 const std::string& fallback_name = "") {
        if (material->GetTextureCount(type) == 0) {
            return nullptr;
        }

        aiString path;
        if (material->GetTexture(type, 0, &path) != AI_SUCCESS) {
            return nullptr;
        }

        auto& assetManager = Engine::GetInstance().GetAssetManager();
        std::string texPath(path.C_Str());

        // Check if texture is embedded
        if (texPath[0] == '*') {
            int texIndex = std::atoi(texPath.c_str() + 1);
            if (texIndex >= 0 && texIndex < static_cast<int>(scene->mNumTextures)) {
                aiTexture* embeddedTex = scene->mTextures[texIndex];

                spdlog::info("Loading embedded texture index {}: {}", texIndex, fallback_name);

                if (embeddedTex->mHeight == 0) {
                    // Compressed texture
                    int width, height, channels;
                    unsigned char* image_data = stbi_load_from_memory(
                        reinterpret_cast<unsigned char*>(embeddedTex->pcData), embeddedTex->mWidth, &width, &height, &channels, 0);

                    if (!image_data) {
                        spdlog::warn("Failed to decode embedded compressed texture {}: {}", texIndex, stbi_failure_reason());
                        return nullptr;
                    }

                    std::string embedded_path = "embedded://" + fallback_name + "_" + std::to_string(texIndex);

                    ETextureFormat format;
                    if (channels == 1) {
                        format = ETextureFormat::R8;
                    } else if (channels == 2) {
                        format = ETextureFormat::RG8;
                    } else if (channels == 3) {
                        format = ETextureFormat::RGB8;
                    } else if (channels == 4) {
                        format = ETextureFormat::RGBA8;
                    } else {
                        stbi_image_free(image_data);
                        return nullptr;
                    }

                    try {
                        auto loaded_texture = assetManager.EnsureTexture2D(embedded_path, width, height, format, image_data);
                        if (loaded_texture) {
                            spdlog::debug("Created embedded texture: {} ({}x{})", embedded_path, width, height);
                        }
                        return loaded_texture;
                    } catch (const std::exception& e) {
                        stbi_image_free(image_data);
                        spdlog::warn("Failed to create embedded texture: {}", e.what());
                        return nullptr;
                    }
                } else {
                    // Uncompressed texture
                    int width  = embeddedTex->mWidth;
                    int height = embeddedTex->mHeight;

                    std::string embedded_path = "embedded://" + fallback_name + "_" + std::to_string(texIndex);

                    ETextureFormat format     = ETextureFormat::RGBA8;
                    unsigned char* image_data = reinterpret_cast<unsigned char*>(embeddedTex->pcData);

                    try {
                        auto loaded_texture = assetManager.EnsureTexture2D(embedded_path, width, height, format, image_data);
                        if (loaded_texture) {
                            spdlog::debug("Created uncompressed embedded texture: {} ({}x{})", embedded_path, width, height);
                        }
                        return loaded_texture;
                    } catch (const std::exception& e) {
                        spdlog::warn("Failed to create embedded texture: {}", e.what());
                        return nullptr;
                    }
                }
            }
        } else {
            // External texture file
            std::string full_path = base_path + texPath;
            try {
                auto loaded_texture = assetManager.EnsureTexture2D(full_path);
                if (loaded_texture) {
                    spdlog::debug("Loaded external texture: {}", full_path);
                }
                return loaded_texture;
            } catch (const std::exception& e) {
                spdlog::warn("Failed to load external texture {}: {}", full_path, e.what());
                return nullptr;
            }
        }

        return nullptr;
    }

    // ============================================================================
    // Material Creation
    // ============================================================================

    std::shared_ptr<Material> CreateMaterialFromAssimp(const aiMaterial* ai_mat, const std::string& base_path, const aiScene* scene) {
        auto& engine = Engine::GetInstance();
        auto rd      = engine.GetSceneRenderer().GetRenderingDevice();

        std::shared_ptr<Material> material = std::make_shared<Material>();
        std::shared_ptr<Shader> shader     = rd->GetDefaultShader3D();
        material->SetShader(shader);

        ETextureFlags textureFlags = ETextureFlags::NONE;
        glm::vec4 base_color(1.0f);
        float metallic  = 0.0f;
        float roughness = 1.0f;
        glm::vec3 emissive(0.0f);
        float emissiveStrength = 1.0f;

        EBlendMode blendMode     = EBlendMode::BLEND_MODE_OPAQUE;
        bool depthWrite          = true;
        ECullMode cullMode       = ECullMode::CULL_MODE_BACK;
        float alphaClipThreshold = 0.5f;

        aiString matName;
        if (ai_mat->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS) {
            spdlog::debug("Processing material: {}", matName.C_Str());
        }

        // Get base color / diffuse
        aiColor4D diffuse(1.0f);
        if (ai_mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
            base_color = glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
        }

        // Get opacity
        float opacity = 1.0f;
        if (ai_mat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
            base_color.a *= opacity;
        }

        // Check for transparency
        if (base_color.a < 1.0f) {
            blendMode  = EBlendMode::BLEND_MODE_ALPHA;
            depthWrite = false;
        }

        // Get metallic/roughness (PBR)
        ai_mat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
        ai_mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);

        // Fallback: use shininess to estimate roughness
        float shininess = 0.0f;
        if (ai_mat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            roughness = 1.0f - glm::clamp(shininess / 1000.0f, 0.0f, 1.0f);
        }

        // Get emissive
        aiColor3D emissiveColor(0.0f);
        if (ai_mat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == AI_SUCCESS) {
            emissive = glm::vec3(emissiveColor.r, emissiveColor.g, emissiveColor.b);
        }

        // Check two-sided
        int twoSided = 0;
        if (ai_mat->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS && twoSided) {
            cullMode = ECullMode::CULL_MODE_DISABLED;
        }

        // Load textures
        auto albedoTex = LoadAssimpTexture(ai_mat, aiTextureType_DIFFUSE, base_path, scene, "albedo");
        if (albedoTex) {
            material->SetParameter("ALBEDO_TEXTURE", albedoTex);
            textureFlags |= ETextureFlags::HAS_ALBEDO;
        }

        // Try base color texture (GLTF)
        if (!albedoTex) {
            albedoTex = LoadAssimpTexture(ai_mat, aiTextureType_BASE_COLOR, base_path, scene, "base_color");
            if (albedoTex) {
                material->SetParameter("ALBEDO_TEXTURE", albedoTex);
                textureFlags |= ETextureFlags::HAS_ALBEDO;
            }
        }

        auto normalTex = LoadAssimpTexture(ai_mat, aiTextureType_NORMALS, base_path, scene, "normal");
        if (normalTex) {
            material->SetParameter("NORMAL_TEXTURE", normalTex);
            textureFlags |= ETextureFlags::HAS_NORMAL;
        }

        auto metallicTex = LoadAssimpTexture(ai_mat, aiTextureType_METALNESS, base_path, scene, "metallic");
        if (metallicTex) {
            material->SetParameter("METALLIC_TEXTURE", metallicTex);
            textureFlags |= ETextureFlags::HAS_METALLIC;
        }

        auto roughnessTex = LoadAssimpTexture(ai_mat, aiTextureType_DIFFUSE_ROUGHNESS, base_path, scene, "roughness");
        if (roughnessTex) {
            material->SetParameter("ROUGHNESS_TEXTURE", roughnessTex);
            textureFlags |= ETextureFlags::HAS_ROUGHNESS;
        }

        auto aoTex = LoadAssimpTexture(ai_mat, aiTextureType_AMBIENT_OCCLUSION, base_path, scene, "ao");
        if (aoTex) {
            material->SetParameter("AO_TEXTURE", aoTex);
            textureFlags |= ETextureFlags::HAS_AO;
        }

        auto emissiveTex = LoadAssimpTexture(ai_mat, aiTextureType_EMISSIVE, base_path, scene, "emissive");
        if (emissiveTex) {
            material->SetParameter("EMISSIVE_TEXTURE", emissiveTex);
            textureFlags |= ETextureFlags::HAS_EMISSIVE;
        }

        bool shouldUseIBL = false;
        if (metallic > 0.5f) {
            shouldUseIBL = true;
            spdlog::debug("Material has high metallic ({:.2f}), enabling IBL", metallic);
        } else if (roughness < 0.3f) {
            shouldUseIBL = true;
            spdlog::debug("Material has low roughness ({:.2f}), enabling IBL", roughness);
        } else if (metallicTex || roughnessTex) {
            shouldUseIBL = true;
            spdlog::debug("Material has metallic/roughness textures, enabling IBL");
        } else {
            shouldUseIBL = false;
            spdlog::debug("Material using studio lighting (metallic: {:.2f}, roughness: {:.2f})", metallic, roughness);
        }

        material->SetImageBasedLighting(shouldUseIBL);
        material->SetParameter("TEXTURE_FLAGS", static_cast<int>(textureFlags));
        material->SetParameter("u_material.modulate", base_color);
        material->SetParameter("u_material.metallicFactor", metallic);
        material->SetParameter("u_material.roughnessFactor", roughness);
        material->SetParameter("u_material.emissiveFactor", emissive);
        material->SetParameter("u_material.emissiveStrength", emissiveStrength);

        material->SetBlendMode(blendMode);
        material->SetDepthWriteEnabled(depthWrite);
        material->SetCullMode(cullMode);
        material->SetAlphaClipThreshold(alphaClipThreshold);
        material->SetDepthTestEnabled(true);
        material->SetDepthFunc(EComparisonFunc::COMPARISON_LESS);

        return material;
    }

    // ============================================================================
    // Mesh Processing
    // ============================================================================

    void ProcessAssimpMesh(const aiMesh* mesh, GameObject* gameObject, const aiScene* scene, const std::string& base_path) {
        auto& engine = Engine::GetInstance();
        auto rd      = engine.GetSceneRenderer().GetRenderingDevice();

        bool hasSkinning    = mesh->HasBones();
        VertexLayout layout = hasSkinning ? CreateSkinnedVertexLayout() : CreateStandardVertexLayout();

        std::vector<float> vertices;
        std::vector<Uint32> indices;

        size_t floatsPerVertex = hasSkinning ? 19 : 11;
        vertices.reserve(mesh->mNumVertices * floatsPerVertex);

        // Bone data structures for skinning
        std::vector<std::vector<std::pair<int, float>>> vertexBoneData;
        if (hasSkinning) {
            vertexBoneData.resize(mesh->mNumVertices);
        }

        // Process bone weights - CRITICAL: Map to GLOBAL skeleton indices
        if (hasSkinning) {
            for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
                aiBone* bone         = mesh->mBones[b];
                std::string boneName = bone->mName.C_Str();

                // Get the GLOBAL skeleton index for this bone
                auto it = g_boneNameToIndex.find(boneName);
                if (it == g_boneNameToIndex.end()) {
                    spdlog::warn("Bone '{}' not found in global skeleton", boneName);
                    continue;
                }

                int globalBoneIndex = it->second;

                for (unsigned int w = 0; w < bone->mNumWeights; ++w) {
                    unsigned int vertexId = bone->mWeights[w].mVertexId;
                    float weight          = bone->mWeights[w].mWeight;
                    // Store GLOBAL bone index, not local mesh index
                    vertexBoneData[vertexId].push_back({globalBoneIndex, weight});
                }
            }
        }

        // Process vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            // Position
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);

            // Color
            if (mesh->HasVertexColors(0)) {
                vertices.push_back(mesh->mColors[0][i].r);
                vertices.push_back(mesh->mColors[0][i].g);
                vertices.push_back(mesh->mColors[0][i].b);
            } else {
                vertices.insert(vertices.end(), {1.0f, 1.0f, 1.0f});
            }

            // TexCoord
            if (mesh->HasTextureCoords(0)) {
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);
            } else {
                vertices.insert(vertices.end(), {0.0f, 0.0f});
            }

            // Normal
            if (mesh->HasNormals()) {
                vertices.push_back(mesh->mNormals[i].x);
                vertices.push_back(mesh->mNormals[i].y);
                vertices.push_back(mesh->mNormals[i].z);
            } else {
                vertices.insert(vertices.end(), {0.0f, 1.0f, 0.0f});
            }

            // Bone data
            if (hasSkinning) {
                float boneIndices[4] = {0.0f, 0.0f, 0.0f, 0.0f};
                float boneWeights[4] = {0.0f, 0.0f, 0.0f, 0.0f};

                auto& boneData = vertexBoneData[i];
                for (size_t j = 0; j < std::min<size_t>(4, boneData.size()); ++j) {
                    boneIndices[j] = static_cast<float>(boneData[j].first);
                    boneWeights[j] = boneData[j].second;
                }

                // Normalize weights
                float totalWeight = boneWeights[0] + boneWeights[1] + boneWeights[2] + boneWeights[3];
                if (totalWeight > 0.0f) {
                    boneWeights[0] /= totalWeight;
                    boneWeights[1] /= totalWeight;
                    boneWeights[2] /= totalWeight;
                    boneWeights[3] /= totalWeight;
                } else {
                    boneWeights[0] = 1.0f;
                }

                vertices.insert(vertices.end(), {boneIndices[0], boneIndices[1], boneIndices[2], boneIndices[3]});
                vertices.insert(vertices.end(), {boneWeights[0], boneWeights[1], boneWeights[2], boneWeights[3]});
            }
        }

        // Process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }

        std::shared_ptr<Mesh> meshObj = rd->CreateMeshFromData(layout, vertices, indices);
        std::shared_ptr<Material> material;

        if (mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < scene->mNumMaterials) {
            material = CreateMaterialFromAssimp(scene->mMaterials[mesh->mMaterialIndex], base_path, scene);
        } else {
            material = std::make_shared<Material>();
            material->SetShader(rd->GetDefaultShader3D());
        }

        gameObject->AddComponent(new MeshRendererComponent(meshObj, material));
    }

    // ============================================================================
    // Animation Loading
    // ============================================================================

    void LoadAssimpAnimations(const aiScene* scene, GameObject* rootObject) {
        if (scene->mNumAnimations == 0) {
            return;
        }

        std::vector<std::shared_ptr<AnimationClip>> clips;

        for (unsigned int ai = 0; ai < scene->mNumAnimations; ++ai) {
            aiAnimation* anim    = scene->mAnimations[ai];
            auto clip            = std::make_shared<AnimationClip>();
            clip->name           = anim->mName.C_Str();
            float ticksPerSecond = (anim->mTicksPerSecond > 0.0) ? static_cast<float>(anim->mTicksPerSecond) : 25.0f;
            clip->duration       = static_cast<float>(anim->mDuration) / ticksPerSecond;

            spdlog::debug("Processing animation: {} with {} channels, duration: {}", clip->name, anim->mNumChannels, clip->duration);

            for (unsigned int ci = 0; ci < anim->mNumChannels; ++ci) {
                aiNodeAnim* channel = anim->mChannels[ci];

                TransformTrack track;
                track.targetName = channel->mNodeName.C_Str();

                // Position keys
                for (unsigned int pi = 0; pi < channel->mNumPositionKeys; ++pi) {
                    aiVectorKey& key = channel->mPositionKeys[pi];
                    KeyFrameVec3 keyframe;
                    keyframe.time  = static_cast<float>(key.mTime) / ticksPerSecond;
                    keyframe.value = AiVector3DToGlm(key.mValue);
                    track.positions.push_back(keyframe);
                }

                // Rotation keys
                for (unsigned int ri = 0; ri < channel->mNumRotationKeys; ++ri) {
                    aiQuatKey& key = channel->mRotationKeys[ri];
                    KeyFrameQuat keyframe;
                    keyframe.time  = static_cast<float>(key.mTime) / ticksPerSecond;
                    keyframe.value = AiQuaternionToGlm(key.mValue);
                    track.rotations.push_back(keyframe);
                }

                // Scale keys
                for (unsigned int si = 0; si < channel->mNumScalingKeys; ++si) {
                    aiVectorKey& key = channel->mScalingKeys[si];
                    KeyFrameVec3 keyframe;
                    keyframe.time  = static_cast<float>(key.mTime) / ticksPerSecond;
                    keyframe.value = AiVector3DToGlm(key.mValue);
                    track.scales.push_back(keyframe);
                }

                clip->tracks.push_back(track);
            }

            clips.push_back(clip);
        }

        if (!clips.empty()) {
            auto animComp = new AnimationComponent();
            rootObject->AddComponent(animComp);

            for (auto& clip : clips) {
                animComp->RegisterClip(clip->name, clip);
            }

            spdlog::debug("Loaded {} animation clips", clips.size());
        }
    }

    // ============================================================================
    // Skeleton Loading
    // ============================================================================

    void LoadAssimpSkeleton(const aiScene* scene, GameObject* rootObject) {
        bool hasSkeleton = false;
        for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
            if (scene->mMeshes[i]->HasBones()) {
                hasSkeleton = true;
                break;
            }
        }

        if (!hasSkeleton) {
            return;
        }

        auto skeleton  = std::make_shared<Skeleton>();
        skeleton->name = "Skeleton";

        std::vector<aiBone*> allBones;

        // Collect all unique bones from all meshes
        for (unsigned int mi = 0; mi < scene->mNumMeshes; ++mi) {
            aiMesh* mesh = scene->mMeshes[mi];
            for (unsigned int bi = 0; bi < mesh->mNumBones; ++bi) {
                aiBone* bone         = mesh->mBones[bi];
                std::string boneName = bone->mName.C_Str();

                if (g_boneNameToIndex.find(boneName) == g_boneNameToIndex.end()) {
                    g_boneNameToIndex[boneName] = static_cast<int>(allBones.size());
                    allBones.push_back(bone);
                }
            }
        }

        // Create joints
        skeleton->joints.resize(allBones.size());

        // Build node hierarchy map
        std::unordered_map<std::string, aiNode*> nodeMap;
        std::function<void(aiNode*)> buildNodeMap = [&](aiNode* node) {
            nodeMap[node->mName.C_Str()] = node;
            for (unsigned int i = 0; i < node->mNumChildren; ++i) {
                buildNodeMap(node->mChildren[i]);
            }
        };
        buildNodeMap(scene->mRootNode);

        // First pass: create all joints with LOCAL transforms
        for (size_t i = 0; i < allBones.size(); ++i) {
            aiBone* bone            = allBones[i];
            SkeletonJoint& joint    = skeleton->joints[i];
            joint.name              = bone->mName.C_Str();
            joint.inverseBindMatrix = AiMatrix4x4ToGlm(bone->mOffsetMatrix);
            joint.parentIndex       = -1;

            // Get LOCAL transform from node (not global)
            auto nodeIt = nodeMap.find(bone->mName.C_Str());
            if (nodeIt != nodeMap.end()) {
                aiNode* node = nodeIt->second;

                // Extract LOCAL transform components
                aiVector3D position, scaling;
                aiQuaternion rotation;
                node->mTransformation.Decompose(scaling, rotation, position);

                joint.position = AiVector3DToGlm(position);
                joint.rotation = AiQuaternionToGlm(rotation);
                joint.scale    = AiVector3DToGlm(scaling);
            } else {
                joint.position = glm::vec3(0.0f);
                joint.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
                joint.scale    = glm::vec3(1.0f);
            }
        }

        // Second pass: establish parent relationships
        for (size_t i = 0; i < allBones.size(); ++i) {
            aiBone* bone = allBones[i];
            auto nodeIt  = nodeMap.find(bone->mName.C_Str());

            if (nodeIt != nodeMap.end() && nodeIt->second->mParent) {
                auto parentIt = g_boneNameToIndex.find(nodeIt->second->mParent->mName.C_Str());
                if (parentIt != g_boneNameToIndex.end()) {
                    skeleton->joints[i].parentIndex = parentIt->second;
                }
            }
        }

        // Process skeletal animations
        std::vector<std::shared_ptr<SkeletonAnimationClip>> skeletonClips;

        for (unsigned int ai = 0; ai < scene->mNumAnimations; ++ai) {
            aiAnimation* anim    = scene->mAnimations[ai];
            auto clip            = std::make_shared<SkeletonAnimationClip>();
            clip->name           = anim->mName.C_Str();
            float ticksPerSecond = (anim->mTicksPerSecond > 0.0) ? static_cast<float>(anim->mTicksPerSecond) : 25.0f;
            clip->duration       = static_cast<float>(anim->mDuration) / ticksPerSecond;

            for (unsigned int ci = 0; ci < anim->mNumChannels; ++ci) {
                aiNodeAnim* channel  = anim->mChannels[ci];
                std::string nodeName = channel->mNodeName.C_Str();

                // Check if this is a bone
                if (g_boneNameToIndex.find(nodeName) == g_boneNameToIndex.end()) {
                    continue;
                }

                SkeletonAnimationTrack track;
                track.targetJointName = nodeName;

                for (unsigned int pi = 0; pi < channel->mNumPositionKeys; ++pi) {
                    aiVectorKey& key = channel->mPositionKeys[pi];
                    KeyFrameVec3 keyframe;
                    keyframe.time  = static_cast<float>(key.mTime) / ticksPerSecond;
                    keyframe.value = AiVector3DToGlm(key.mValue);
                    track.positions.push_back(keyframe);
                }

                for (unsigned int ri = 0; ri < channel->mNumRotationKeys; ++ri) {
                    aiQuatKey& key = channel->mRotationKeys[ri];
                    KeyFrameQuat keyframe;
                    keyframe.time  = static_cast<float>(key.mTime) / ticksPerSecond;
                    keyframe.value = AiQuaternionToGlm(key.mValue);
                    track.rotations.push_back(keyframe);
                }

                for (unsigned int si = 0; si < channel->mNumScalingKeys; ++si) {
                    aiVectorKey& key = channel->mScalingKeys[si];
                    KeyFrameVec3 keyframe;
                    keyframe.time  = static_cast<float>(key.mTime) / ticksPerSecond;
                    keyframe.value = AiVector3DToGlm(key.mValue);
                    track.scales.push_back(keyframe);
                }

                clip->tracks.push_back(track);
            }

            skeletonClips.push_back(clip);
        }

        if (!skeleton->joints.empty()) {
            auto skelAnimComp = new SkeletonAnimationComponent();
            rootObject->AddComponent(skelAnimComp);
            skelAnimComp->SetSkeleton(skeleton);

            for (auto& clip : skeletonClips) {
                skelAnimComp->RegisterClip(clip->name, clip);
                spdlog::debug("Registered skeleton animation: {} with {} tracks", clip->name, clip->tracks.size());
            }

            spdlog::debug("Loaded skeleton with {} joints", skeleton->joints.size());
        }
    }

    // ============================================================================
    // Scene Graph Parsing
    // ============================================================================

    void ProcessAssimpNode(aiNode* node, GameObject* parent, const aiScene* scene, const std::string& base_path, Scene* sceneObj) {
        std::string nodeName = node->mName.C_Str();
        if (nodeName.empty()) {
            nodeName = "Node";
        }

        GameObject* nodeObject = sceneObj->CreateObject(nodeName, parent);

        aiMatrix4x4 transform = node->mTransformation;
        aiVector3D position, scaling;
        aiQuaternion rotation;
        transform.Decompose(scaling, rotation, position);

        nodeObject->SetPosition(AiVector3DToGlm(position));
        nodeObject->SetRotation(AiQuaternionToGlm(rotation));
        nodeObject->SetScale(AiVector3DToGlm(scaling));

        // Process meshes
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessAssimpMesh(mesh, nodeObject, scene, base_path);
        }

        // Process children recursively
        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            ProcessAssimpNode(node->mChildren[i], nodeObject, scene, base_path, sceneObj);
        }
    }

    // ============================================================================
    // Public API Implementation
    // ============================================================================

    GameObject* Model::Load(std::string_view path, Scene* scene) {
        if (!scene) {
            spdlog::error("Model::Load failed: Scene is invalid");
            return nullptr;
        }

        return LoadAssimp(path, scene);
    }

    GameObject* Model::LoadAssimp(std::string_view path, Scene* scene) {
        auto& fs = Engine::GetInstance().GetFileSystem();

        std::string fullPath = fs.GetAssetsPath() + std::string(path);
        size_t s             = path.find_last_of("/\\");
        std::string basePath = (s == std::string::npos) ? "" : std::string(path.substr(0, s + 1));

        std::string model_name(path);
        size_t last_slash = model_name.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            model_name = model_name.substr(last_slash + 1);
        }

        Assimp::Importer importer;

        unsigned int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
                           | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_LimitBoneWeights
                           | aiProcess_RemoveRedundantMaterials | aiProcess_SplitLargeMeshes | aiProcess_ValidateDataStructure
                           | aiProcess_OptimizeMeshes;

        const aiScene* aiScene = importer.ReadFile(fullPath, flags);

        if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiScene->mRootNode) {
            spdlog::error("Failed to load model: {} - {}", path, importer.GetErrorString());
            return nullptr;
        }

        if (aiScene->mNumMeshes == 0) {
            spdlog::error("No mesh data found in model: {}", path);
            return nullptr;
        }

        GameObject* rootObject = scene->CreateObject(model_name, nullptr);

        g_boneNameToIndex.clear();
        LoadAssimpSkeleton(aiScene, rootObject);

        ProcessAssimpNode(aiScene->mRootNode, rootObject, aiScene, basePath, scene);

        LoadAssimpAnimations(aiScene, rootObject);

        spdlog::info("Successfully loaded model '{}' with {} meshes, {} materials, {} animations",
                     path,
                     aiScene->mNumMeshes,
                     aiScene->mNumMaterials,
                     aiScene->mNumAnimations);

        return rootObject;
    }

} // namespace golias
