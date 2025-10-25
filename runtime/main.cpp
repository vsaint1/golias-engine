#include "stdafx.h"
#include <SDL3/SDL_main.h>
#include  "core/renderer/opengl/ogl_renderer.h"







// ============================================================================
// MODEL (combines mesh + material)
// ============================================================================


class ObjectLoader {
public:
    static MeshInstance3D load_mesh(const std::string& path) {
        Model model = load_model(path, nullptr);
        return model.meshes.empty() ? MeshInstance3D() : model.meshes[0];
    }

    static Model load_model(const std::string& path, Renderer* renderer = nullptr) {
        Model model;

        const char* extension = strrchr(path.c_str(), '.');

        spdlog::info("Loading Model Path: {}, FileFormat: {}", path, extension ? extension + 1 : "UNKNOWN");

        Assimp::Importer importer;

        constexpr unsigned int ASSIMP_FLAGS =
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace |
            aiProcess_GenNormals;

        const aiScene* scene = importer.ReadFile(path, ASSIMP_FLAGS);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            spdlog::error("Failed to load model: {}, Error: ", path, importer.GetErrorString());
            return model;
        }

        const std::string directory = get_directory(path);
        spdlog::info("  Meshes: {}, Materials: {}, Animations: {}",
                     scene->mNumMeshes, scene->mNumMaterials, scene->mNumAnimations);

        for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
            aiMesh* aiMesh = scene->mMeshes[i];
            spdlog::info("  Parsing Mesh({}) - Name {}", i, aiMesh->mName.C_Str());

            MeshInstance3D mesh = create_mesh(aiMesh);
            model.meshes.push_back(mesh);

            if (renderer) {
                Material material = load_material(scene, aiMesh, directory, *renderer);
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

private:
    static std::string get_directory(const std::string& path) {
        size_t found = path.find_last_of("/\\");
        return (found != std::string::npos) ? path.substr(0, found + 1) : "";
    }


    static MeshInstance3D create_mesh(aiMesh* aiMesh) {
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

    static Material load_material(const aiScene* scene, aiMesh* mesh,
                                  const std::string& directory, Renderer& renderer) {
        Material material;
        if (scene->mNumMaterials <= mesh->mMaterialIndex)
            return material;

        aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
        load_colors(aiMat, material);
        load_textures(aiMat, scene, directory, renderer, material);
        return material;
    }

    static void load_colors(aiMaterial* aiMat, Material& mat) {
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

    static void load_textures(aiMaterial* aiMat, const aiScene* scene,
                              const std::string& dir, Renderer& renderer, Material& mat) {
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

            std::string full = dir + "/" + texStr;
            id               = renderer.load_texture_from_file(full);
            flag             = (id != 0);
            if (flag)
                spdlog::info("    Texture loaded [{}]: {}", name, full);
        };

        load_tex(aiTextureType_DIFFUSE, mat.albedoMap, mat.useAlbedoMap, "albedo");
        load_tex(aiTextureType_NORMALS, mat.normalMap, mat.useNormalMap, "normal");
        load_tex(aiTextureType_METALNESS, mat.metallicMap, mat.useMetallicMap, "metallic");
        load_tex(aiTextureType_DIFFUSE_ROUGHNESS, mat.roughnessMap, mat.useRoughnessMap, "roughness");
        load_tex(aiTextureType_AMBIENT_OCCLUSION, mat.aoMap, mat.useAOMap, "ao");
        load_tex(aiTextureType_EMISSIVE, mat.emissiveMap, mat.useEmissiveMap, "emissive");
    }

    static void parse_bones(aiMesh* mesh, Model& model) {
        spdlog::info("    Bones: {}", mesh->mNumBones);
        for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
            aiBone* bone = mesh->mBones[i];
            spdlog::info("      - Bone {}: {}", i, bone->mName.C_Str());
        }
    }


    static void parse_animations(const aiScene* scene, Model& model) {
        spdlog::info("  → Animations: {}", scene->mNumAnimations);
        for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
            const aiAnimation* anim = scene->mAnimations[i];
            spdlog::info("    Animation {}: {} | Duration: {:.2f} | FPS: {}",
                         i, anim->mName.C_Str(), anim->mDuration, anim->mTicksPerSecond);
        }
    }
};



class Engine {
    SDL_Window* window      = nullptr;
    SDL_GLContext glContext = nullptr;
    bool running            = false;
    int width, height;

    std::unique_ptr<Renderer> renderer;
    flecs::world ecs;

    // Input state
    bool keys[SDL_SCANCODE_COUNT] = {false};
    bool firstMouse               = true;
    float lastX                   = 0.0f;
    float lastY                   = 0.0f;
    bool mouseCaptured            = false;

public:
    Engine(const std::string& title, int w, int h) : width(w), height(h) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            spdlog::error("SDL initialization failed: {}", SDL_GetError());
            return;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        window = SDL_CreateWindow(
            title.c_str(),
            width, height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
            );

        if (!window) {
            spdlog::error("Window creation failed: {}", SDL_GetError());
            SDL_Quit();
            return;
        }

        glContext = SDL_GL_CreateContext(window);
        if (!glContext) {
            spdlog::error("OpenGL context creation failed: {}", SDL_GetError());
            SDL_DestroyWindow(window);
            SDL_Quit();
            return;
        }

        SDL_GL_SetSwapInterval(0);

        renderer = std::make_unique<OpenGLRenderer>();
        if (!renderer->initialize(width, height)) {
            spdlog::error("Renderer initialization failed: UNKNOWN");
            cleanup();
            return;
        }

        lastX = width / 2.0f;
        lastY = height / 2.0f;

        running = true;
    }

    void handle_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                width = event.window.data1;
                height = event.window.data2;
                renderer->resize(width, height);
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode < SDL_SCANCODE_COUNT) {
                    keys[event.key.scancode] = true;
                }
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    mouseCaptured = !mouseCaptured;
                    SDL_SetWindowRelativeMouseMode(window, mouseCaptured);
                    firstMouse = true;
                }
                break;

            case SDL_EVENT_KEY_UP:
                if (event.key.scancode < SDL_SCANCODE_COUNT) {
                    keys[event.key.scancode] = false;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (mouseCaptured) {
                    float xpos = event.motion.x;
                    float ypos = event.motion.y;

                    if (firstMouse) {
                        lastX      = xpos;
                        lastY      = ypos;
                        firstMouse = false;
                    }

                    float xoffset = event.motion.xrel;
                    float yoffset = -event.motion.yrel;

                    ecs.each([&](Camera3D& cam) {
                        cam.yaw += xoffset * cam.mouseSensitivity;
                        cam.pitch += yoffset * cam.mouseSensitivity;

                        if (cam.pitch > 89.0f)
                            cam.pitch = 89.0f;
                        if (cam.pitch < -89.0f)
                            cam.pitch = -89.0f;

                        cam.update_vectors();
                    });
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT && !mouseCaptured) {
                    mouseCaptured = true;
                    SDL_SetWindowRelativeMouseMode(window, true);
                    firstMouse = true;
                }
                break;
            default: ;
            }
        }
    }

    void process_camera_movement(float deltaTime) {
        ecs.each([&](Camera3D& cam) {
            float velocity = cam.movementSpeed * deltaTime;

            if (keys[SDL_SCANCODE_W])
                cam.position += cam.front * velocity;
            if (keys[SDL_SCANCODE_S])
                cam.position -= cam.front * velocity;
            if (keys[SDL_SCANCODE_A])
                cam.position -= glm::normalize(glm::cross(cam.front, cam.up)) * velocity;
            if (keys[SDL_SCANCODE_D])
                cam.position += glm::normalize(glm::cross(cam.front, cam.up)) * velocity;
            if (keys[SDL_SCANCODE_SPACE])
                cam.position += cam.up * velocity;
            if (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL])
                cam.position -= cam.up * velocity;

            if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])
                cam.movementSpeed = 500.0f;
            else
                cam.movementSpeed = 100.0f;
        });
    }

    void update(float deltaTime) {
        process_camera_movement(deltaTime);
        ecs.progress(deltaTime);
    }

    void render() {
        std::vector<DirectionalLight> directionalLights;
        glm::mat4 lightSpaceMatrix(1.0f);

        Camera3D mainCamera;
        ecs.each([&](const Camera3D& cam) {
            mainCamera = cam;
        });

        ecs.each([&](flecs::entity e, Transform3D& t, DirectionalLight& light) {
            directionalLights.push_back(light);

            if (light.castShadows && lightSpaceMatrix == glm::mat4(1.0f)) {
                lightSpaceMatrix = light.get_light_space_matrix();
            }
        });

        std::vector<std::pair<Transform3D, SpotLight>> spotLights;
        ecs.each([&](flecs::entity e, Transform3D& t, SpotLight& light) {
            spotLights.push_back({t, light});
        });

        // Shadow pass
        renderer->begin_shadow_pass();
        ecs.each([&](Transform3D& t, MeshInstance3D& mesh, Material& mat) {
            renderer->render_shadow_pass(t, mesh, lightSpaceMatrix);
        });
        renderer->end_shadow_pass();

        // Main render pass
        renderer->begin_render_target();
        ecs.each([&](Transform3D& t, MeshInstance3D& mesh, Material& mat) {
            renderer->render_entity(t, mesh, mat, mainCamera, lightSpaceMatrix,
                                    directionalLights, spotLights);
        });
        renderer->end_render_target();

        SDL_GL_SwapWindow(window);
    }

    void run() {
        uint64_t lastTime = SDL_GetPerformanceCounter();

        while (running) {
            uint64_t currentTime = SDL_GetPerformanceCounter();
            float deltaTime      = (float) (currentTime - lastTime) / SDL_GetPerformanceFrequency();
            lastTime             = currentTime;

            handle_events();
            update(deltaTime);
            render();
        }
    }

    flecs::world& get_world() {
        return ecs;
    }

    Renderer* get_renderer() const {
        return renderer.get();
    }

    void cleanup() {
        if (renderer)
            renderer->cleanup();

        if (glContext)
            SDL_GL_DestroyContext(glContext);
        if (window)
            SDL_DestroyWindow(window);
        SDL_Quit();
    }

    ~Engine() {
        cleanup();
    }
};


int main(int argc, char* argv[]) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/output.log", true);
    file_sink->set_level(spdlog::level::info);

    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("GoliasEngine", sinks.begin(), sinks.end());

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::info);

    Engine engine("depressao gamer", 1280, 720);

    auto& ecs = engine.get_world();

    auto renderer = engine.get_renderer();

    // Create camera
    auto camera = ecs.entity()
                     .set(Camera3D{glm::vec3(0, 3, 10), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), 45.0f});

    // Create directional light (sun) with shadows
    auto dirLight = ecs.entity()
                       .set(Transform3D{})
                       .set(DirectionalLight{glm::vec3(1, -2.5, 1), glm::vec3(1.0f, 0.95f, 0.8f), 2.0f, true});

    // Create spot lights (no shadows)
    auto spotLight1 = ecs.entity()
                         .set(Transform3D{glm::vec3(5, 5, 5)})
                         .set(SpotLight{glm::vec3(-1, -1, -1), glm::vec3(1.0f, 0.3f, 0.3f), 30.0f, 12.5f, 17.5f});

    auto spotLight2 = ecs.entity()
                         .set(Transform3D{glm::vec3(-5, 5, 5)})
                         .set(SpotLight{glm::vec3(1, -1, -1), glm::vec3(0.3f, 0.3f, 1.0f), 50.0f, 12.5f, 17.5f});


    Model carModel = ObjectLoader::load_model("res/sprites/obj/Car2.obj", renderer);
    for (size_t i = 0; i < carModel.meshes.size(); i++) {
        ecs.entity(("Car_Mesh_" + std::to_string(i)).c_str())
           .set(Transform3D{glm::vec3(-10, 0, -5), glm::vec3(0), glm::vec3(1.0f)})
           .set(carModel.meshes[i])
           .set(carModel.materials[i]);
    }


    Model damagedHelmet = ObjectLoader::load_model("res/sprites/obj/DamagedHelmet.glb", renderer);
    for (size_t i = 0; i < damagedHelmet.meshes.size(); i++) {
        ecs.entity(("Helmet_Mesh_" + std::to_string(i)).c_str())
           .set(Transform3D{glm::vec3(15, 0, 0), glm::vec3(0), glm::vec3(1.0f)})
           .set(damagedHelmet.meshes[i])
           .set(damagedHelmet.materials[i]);
    }

    // Model sponza = ObjectLoader::load_model("res/sprites/obj/sponza/sponza.glb", renderer);
    // for (size_t i = 0; i < sponza.meshes.size(); i++) {
    //     ecs.entity(("Sponza_Mesh_" + std::to_string(i)).c_str())
    //         .set(Transform3D{glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(1.0f)})
    //         .set(sponza.meshes[i])
    //         .set(sponza.materials[i]);
    // }

    Model nagonford = ObjectLoader::load_model("res/sprites/obj/nagonford/Nagonford_Animated.glb", renderer);
    for (size_t i = 0; i < nagonford.meshes.size(); i++) {
        ecs.entity(("Nagonford_Mesh_" + std::to_string(i)).c_str())
           .set(Transform3D{glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(1.0f)})
           .set(nagonford.meshes[i])
           .set(nagonford.materials[i]);
    }

    MeshInstance3D cylinderMesh = ObjectLoader::load_mesh("res/models/cylinder.obj");
    ecs.entity("Cylinder").set(Transform3D{glm::vec3(0, 0, -10), glm::vec3(0), glm::vec3(1.0f)}).set(cylinderMesh).set(Material{
        .albedo = glm::vec3(0, 0, 1.0f)});

    MeshInstance3D torusMesh = ObjectLoader::load_mesh("res/models/torus.obj");
    ecs.entity("Torus").set(Transform3D{glm::vec3(0, 0, 5), glm::vec3(0), glm::vec3(1.0f)}).set(torusMesh).set(Material{
        .albedo = glm::vec3(1.f, 0.f, 1.0), .metallic = 1.f,.roughness = 0.1, .emissive = glm::vec3(1, 0,0), .emissiveStrength = 1.0f});

    MeshInstance3D coneMesh = ObjectLoader::load_mesh("res/models/cone.obj");
    ecs.entity("Cone").set(Transform3D{glm::vec3(0, 0, 20), glm::vec3(0), glm::vec3(1.0f)}).set(coneMesh).set(Material{
        .albedo = glm::vec3(0, 1.0f, 1.0)});

    MeshInstance3D blenderMonkeyMesh = ObjectLoader::load_mesh("res/models/blender_monkey.obj");
    ecs.entity("Cone").set(Transform3D{glm::vec3(0, 0, 10), glm::vec3(0), glm::vec3(1.0f)}).set(blenderMonkeyMesh).set(Material{
        .albedo = glm::vec3(1.0f, 0.0f, 1.0)});

    MeshInstance3D cubeMesh = ObjectLoader::load_mesh("res/models/cube.obj");
    ecs.entity("Red Cube")
       .set(Transform3D{glm::vec3(3, 0, 0), glm::vec3(0), glm::vec3(1.5f)})
       .set(cubeMesh)
       .set(Material{glm::vec3(0.8f, 0.1f, 0.1f), 0.0f, 0.3f, 1.0f});

    MeshInstance3D sphereMesh = ObjectLoader::load_mesh("res/models/sphere.obj");

    ecs.entity("Sphere")
       .set(Transform3D{glm::vec3(-3, 0, 0), glm::vec3(0), glm::vec3(1.5f)})
       .set(sphereMesh)
       .set(Material{glm::vec3(1.f), 0.0f, 0.1f, 1.0f});

    ecs.entity("Cube")
       .set(Transform3D{glm::vec3(3, 0, 0), glm::vec3(0), glm::vec3(1.f)})
       .set(cubeMesh)
       .set(Material{glm::vec3(1.f), 0.0f, 0.0f, 1.0f,});

    ecs.entity("Ground")
       .set(Transform3D{glm::vec3(0, -2, 0), glm::vec3(0), glm::vec3(20.0f, 0.1f, 20.0f)})
       .set(cubeMesh)
       .set(Material{glm::vec3(1.0f), 0.0f, 0.8f, 1.0f});

    engine.run();

    spdlog::shutdown();
    return 0;
}
