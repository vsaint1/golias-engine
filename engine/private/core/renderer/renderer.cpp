#include "core/renderer/renderer.h"

#include <core/engine.h>


void Renderer::set_default_fonts(const std::string& text_font, const std::string& emoji_font) {
    _default_font_name = text_font;
    _emoji_font_name   = emoji_font;

    LOG_DEBUG("Default Fonts Alias: Text: %s | Emoji: %s", text_font.c_str(), emoji_font.c_str());
}


std::string Renderer::vformat(const char* fmt, va_list args) {
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    return std::string(buf);
}

std::shared_ptr<Texture> Renderer::load_texture(const std::string& name, const std::string& path, const aiTexture* ai_embedded_tex) {


    SDL_Surface* surf = nullptr;

    if (ai_embedded_tex) {

        LOG_INFO("Loading embedded texture: %s (%ux%u)", name.c_str(), ai_embedded_tex->mWidth, ai_embedded_tex->mHeight);


        // (mHeight == 0 means compressed)
        if (ai_embedded_tex->mHeight == 0) {
            // Compressed texture (e.g., PNG, JPEG)
            SDL_IOStream* rw = SDL_IOFromConstMem(ai_embedded_tex->pcData, ai_embedded_tex->mWidth);
            if (!rw) {
                LOG_ERROR("Failed to create SDL_IOStream from embedded texture data");
                return nullptr;
            }

            surf = IMG_Load_IO(rw, true);
            if (!surf) {
                LOG_ERROR("Failed to load compressed embedded texture: %s", SDL_GetError());
                return nullptr;
            }
        } else {
            // Uncompressed ARGB8888 texture
            surf = SDL_CreateSurface(ai_embedded_tex->mWidth, ai_embedded_tex->mHeight, SDL_PIXELFORMAT_RGBA32);
            if (!surf) {
                LOG_ERROR("Failed to create surface for embedded texture: %s", SDL_GetError());
                return nullptr;
            }

            aiTexel* texels = ai_embedded_tex->pcData;
            Uint32* pixels  = (Uint32*) surf->pixels;

            for (unsigned int i = 0; i < ai_embedded_tex->mWidth * ai_embedded_tex->mHeight; i++) {
                // ARGB to RGBA
                Uint8 r   = texels[i].r;
                Uint8 g   = texels[i].g;
                Uint8 b   = texels[i].b;
                Uint8 a   = texels[i].a;
                pixels[i] = SDL_MapSurfaceRGBA(surf, r, g, b, a);
            }
        }
    } else {


        LOG_INFO("Loading texture: %s", path.c_str());

        FileAccess file_access(path, ModeFlags::READ);

        if (!file_access.is_open()) {
            LOG_ERROR("Failed to open texture file: %s", path.c_str());
            return nullptr;
        }

        surf = IMG_Load_IO(file_access.get_handle(), false);
        // SDL_Surface* surf = IMG_Load(path.c_str());

        if (!surf) {
            LOG_ERROR("Failed to load texture: %s, Error: %s", path.c_str(), SDL_GetError());
            return nullptr;
        }
    }

    std::shared_ptr<Texture> texture = nullptr;

    switch (GEngine->get_config().get_renderer_device().backend) {
    case Backend::GL_COMPATIBILITY:
        texture = std::make_shared<OpenglTexture>();
        break;
    case Backend::AUTO:
        texture = std::make_shared<SDLTexture>();
        break;
    case Backend::VK_FORWARD:
        // texture = std::make_shared<VulkanTexture>();
        LOG_ERROR("Vulkan texture loading not implemented yet");
        break;
    case Backend::DIRECTX12:
        // texture = std::make_shared<DirectX12Texture>();
        LOG_ERROR("DirectX12 texture loading not implemented yet");
        break;
    case Backend::METAL:
        // texture = std::make_shared<MetalTexture>();
        LOG_ERROR("Metal texture loading not implemented yet");
        break;
    default:
        texture = std::make_shared<Texture>();
        break;
    }


    surf = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);

    if (!surf) {
        LOG_ERROR("Failed to convert texture to RGBA32: %s", SDL_GetError());
        SDL_DestroySurface(surf);
        return nullptr;
    }

    texture->width   = surf->w;
    texture->height  = surf->h;
    texture->path    = path;
    texture->surface = surf;

    LOG_INFO("Texture Info: Size %dx%d | Path: %s | Embedded: %s", texture->width, texture->height, texture->path.data(),
             ai_embedded_tex != nullptr ? "Yes" : "No");
    return texture;
}


std::shared_ptr<Model> Renderer::load_model(const char* path) {

    std::string base_dir = ASSETS_PATH + path;

    auto importer = std::make_shared<Assimp::Importer>();

    std::string ext = base_dir.substr(base_dir.find_last_of(".") + 1);

    const auto ASSIMP_FLAGS = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals
                            | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;

    const aiScene* scene = importer->ReadFile(base_dir, ASSIMP_FLAGS);

    if (!scene || !scene->mRootNode) {
        LOG_ERROR("Failed to load Model: %s, Error: %s", path, importer->GetErrorString());
        return nullptr;
    }

    auto model  = std::make_shared<Model>();
    model->path = path;

    // Store importer and scene to keep animation data alive
    model->importer = importer;
    model->scene    = scene;

    // Store global inverse transform for animation
    glm::mat4 globalTransform = glm::mat4(1.0f);
    if (scene->mRootNode) {
        aiMatrix4x4 root = scene->mRootNode->mTransformation;
        globalTransform  = glm::transpose(glm::make_mat4(&root.a1));
    }

    model->global_inverse_transform = glm::inverse(globalTransform);

    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* mat = scene->mMaterials[i];
        aiString name;
        if (mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
            LOG_DEBUG("Material %d/%d Name: %s", i + 1, scene->mNumMaterials, name.C_Str());
        }
    }

    base_dir = base_dir.substr(0, base_dir.find_last_of("/\\") + 1);
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        LOG_DEBUG("Loading Mesh %d/%d  Name: %s", i + 1, scene->mNumMeshes, scene->mMeshes[i]->mName.C_Str());
        model->meshes.push_back(load_mesh(scene->mMeshes[i], scene, base_dir));
    }


    if (scene->HasAnimations()) {


        LOG_INFO("Loaded Model: %s | Meshes: %zu | Animations: %u | Format: %s", path, model->meshes.size(), scene->mNumAnimations,
                 ext.c_str());

        for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
            LOG_DEBUG("    [%u] - %s", i, scene->mAnimations[i]->mName.C_Str());
        }

    } else {
        LOG_INFO("Loaded Model: %s | Meshes: %zu | Format: %s", path, model->meshes.size(), ext.c_str());
    }

    return model;
}
