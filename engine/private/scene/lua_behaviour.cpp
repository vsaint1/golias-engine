#include "scene/lua_behaviour.h"

#include "audio/audio.h"
#include "core/debug.h"
#include "core/engine.h"
#include "core/input/cursor.h"
#include "core/time.h"
#include "physics/3d/collision_info.h"
#include "scene/scene.h"
#include "scene/scene_manager.h"

// 3D Components
#include "scene/3d/animation_component.h"
#include "scene/3d/audio_component.h"
#include "scene/3d/audio_listener_component.h"
#include "scene/3d/camera_component.h"
#include "scene/3d/character_controller_component.h"
#include "scene/3d/directional_light_component.h"
#include "scene/3d/mesh_component.h"
#include "scene/3d/physics_component.h"
#include "scene/3d/pointlight_component.h"
#include "scene/3d/skeleton_animation_component.h"
#include "scene/3d/spotlight_component.h"
#include "scene/3d/world_environment_component.h"

// 2D Components
#include "scene/2d/sprite_component_2d.h"

// UI Components
#include "scene/ui/button_component.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/dropdown_component.h"
#include "scene/ui/image_component.h"
#include "scene/ui/inputfield_component.h"
#include "scene/ui/panel_component.h"
#include "scene/ui/progressbar_component.h"
#include "scene/ui/rect_transform_component.h"
#include "scene/ui/scrollrect_component.h"
#include "scene/ui/slider_component.h"
#include "scene/ui/text_component.h"
#include "scene/ui/toggle_component.h"
#include <spdlog/spdlog.h>

namespace golias {

    // ================================================================
    //  Lua runtime helper  creates an instance table whose __index
    //  first checks the class table (user methods), then delegates
    //  to the C++ GameObject so you can write  self:GetName(),
    //  self:SetPosition(...)  etc. directly.
    // ================================================================
    static const char* LUA_RUNTIME = R"(
function __createInstance(classTable, gameObject)
    local instance = {
        gameObject = gameObject
    }

    local mt = {}
    mt.__index = function(t, key)
        local classVal = rawget(classTable, key)
        if classVal ~= nil then
            return classVal
        end

        local go = rawget(t, "gameObject")
        if go == nil then return nil end

        local val = go[key]
        if val == nil then return nil end

        if type(val) == "function" then
            return function(_, ...)
                return val(go, ...)
            end
        end
        return val
    end

    setmetatable(instance, mt)
    return instance
end
)";

    // ================================================================
    //  Component resolver registry
    //  Each component type registers a resolver function that, given
    //  a sol::this_state and a GameObject*, returns the component
    //  wrapped as a sol::object (or nil).
    // ================================================================
    using ComponentResolver = std::function<sol::object(sol::this_state, GameObject*)>;
    using ResolverMap       = std::unordered_map<std::string, ComponentResolver>;

    template <typename T>
    static void RegisterResolver(ResolverMap& map, const std::string& fullName, const std::string& alias = "") {
        auto resolver = [](sol::this_state ts, GameObject* go) -> sol::object {
            sol::state_view lua(ts);
            if (auto* c = go->GetComponent<T>()) {
                return sol::make_object(lua, c);
            }
            return sol::make_object(lua, sol::nil);
        };
        map[fullName] = resolver;
        if (!alias.empty()) {
            map[alias] = resolver;
        }
    }

    // Build the registry once (thread-safe since each LuaBehaviour has its own state
    // but the resolver map is stateless w.r.t. the lua state).
    static ResolverMap& GetComponentResolvers() {
        static ResolverMap resolvers = [] {
            ResolverMap m;
            // UI
            RegisterResolver<ButtonWidgetComponent>(m, "ButtonWidgetComponent", "Button");
            RegisterResolver<TextWidgetComponent>(m, "TextWidgetComponent", "Text");
            RegisterResolver<CanvasComponent>(m, "CanvasComponent", "Canvas");
            RegisterResolver<RectTransformComponent>(m, "RectTransformComponent", "RectTransform");
            RegisterResolver<PanelWidgetComponent>(m, "PanelWidgetComponent", "Panel");
            RegisterResolver<ImageWidgetComponent>(m, "ImageWidgetComponent", "Image");
            RegisterResolver<SliderWidgetComponent>(m, "SliderWidgetComponent", "Slider");
            RegisterResolver<ToggleWidgetComponent>(m, "ToggleWidgetComponent", "Toggle");
            RegisterResolver<InputFieldWidgetComponent>(m, "InputFieldWidgetComponent", "InputField");
            RegisterResolver<ProgressBarWidgetComponent>(m, "ProgressBarWidgetComponent", "ProgressBar");
            RegisterResolver<DropdownWidgetComponent>(m, "DropdownWidgetComponent", "Dropdown");
            RegisterResolver<ScrollRectWidgetComponent>(m, "ScrollRectWidgetComponent", "ScrollRect");
            // 3D
            RegisterResolver<CameraComponent>(m, "CameraComponent", "Camera");
            RegisterResolver<MeshRendererComponent>(m, "MeshRendererComponent", "MeshRenderer");
            RegisterResolver<PhysicsComponent>(m, "PhysicsComponent", "Physics");
            RegisterResolver<CharacterControllerComponent>(m, "CharacterControllerComponent", "CharacterController");
            RegisterResolver<AnimationComponent>(m, "AnimationComponent", "Animation");
            RegisterResolver<SkeletonAnimationComponent>(m, "SkeletonAnimationComponent", "SkeletonAnimation");
            RegisterResolver<AudioComponent>(m, "AudioComponent", "AudioSource");
            RegisterResolver<AudioListenerComponent>(m, "AudioListenerComponent", "AudioListener");
            RegisterResolver<WorldEnvironmentComponent>(m, "WorldEnvironmentComponent", "WorldEnvironment");
            RegisterResolver<DirectionalLightComponent>(m, "DirectionalLightComponent", "DirectionalLight");
            RegisterResolver<PointLightComponent>(m, "PointLightComponent", "PointLight");
            RegisterResolver<SpotlightComponent>(m, "SpotlightComponent", "Spotlight");
            // 2D
            RegisterResolver<SpriteComponent2D>(m, "SpriteComponent2D", "Sprite2D");
            return m;
        }();
        return resolvers;
    }

    // ================================================================
    //  Construction / Destruction
    // ================================================================
    LuaBehaviour::LuaBehaviour() {
        InitLuaState();
    }

    LuaBehaviour::~LuaBehaviour() = default;

    // ================================================================
    //  Lifecycle
    // ================================================================
    void LuaBehaviour::Awake() {
        Behaviour::Awake();

        // Script path is typically not set yet (LoadProperties runs after
        // Awake in the scene loader).  Try anyway  if the path is empty
        // LoadScript returns immediately and we retry in Start().
        if (!isScriptLoaded) {
            LoadScript();
        }
        CallMethod("Awake");
    }

    void LuaBehaviour::Start() {
        Behaviour::Start();

        // Handle late loading: the scene loader calls Awake() before
        // LoadProperties(), so the script path was empty during Awake.
        // Load it now and fire the Awake callback the user missed.
        if (!isScriptLoaded) {
            LoadScript();
            CallMethod("Awake");
        }

        if (!didStart) {
            didStart = true;
            CallMethod("Start");
        }
    }

    void LuaBehaviour::Update(float deltaTime) {
        Behaviour::Update(deltaTime);
        CallMethod("Update", deltaTime);
    }

    void LuaBehaviour::OnDestroy() {
        CallMethod("OnDestroy");
        Behaviour::OnDestroy();
    }

    // ================================================================
    //  Collision callbacks -> Lua
    // ================================================================
    static sol::table BuildCollisionTable(sol::state& lua, const CollisionInfo& collision) {
        sol::table t = lua.create_table();

        // Other game object
        if (collision.gameObject) {
            t["gameObject"] = collision.gameObject;
            t["name"]       = collision.gameObject->GetName();
            t["tag"]        = collision.gameObject->GetTag();
        }

        t["relativeVelocity"] = collision.relativeVelocity;

        // Contact points array
        sol::table contacts = lua.create_table();
        for (size_t i = 0; i < collision.contacts.size(); ++i) {
            sol::table cp = lua.create_table();
            cp["point"]    = collision.contacts[i].point;
            cp["normal"]   = collision.contacts[i].normal;
            cp["impulse"]  = collision.contacts[i].impulse;
            cp["distance"] = collision.contacts[i].distance;
            contacts[i + 1] = cp;
        }
        t["contacts"] = contacts;
        t["contactCount"] = static_cast<int>(collision.contacts.size());

        return t;
    }

    void LuaBehaviour::OnCollisionEnter(const CollisionInfo& collision) {
        if (!isScriptLoaded) return;
        sol::table t = BuildCollisionTable(lua, collision);
        CallMethod("OnCollisionEnter", t);
    }

    void LuaBehaviour::OnCollisionStay(const CollisionInfo& collision) {
        if (!isScriptLoaded) return;
        sol::table t = BuildCollisionTable(lua, collision);
        CallMethod("OnCollisionStay", t);
    }

    void LuaBehaviour::OnCollisionExit(const CollisionInfo& collision) {
        if (!isScriptLoaded) return;
        sol::table t = BuildCollisionTable(lua, collision);
        CallMethod("OnCollisionExit", t);
    }

    // ================================================================
    //  Properties
    // ================================================================
    void LuaBehaviour::LoadProperties(const nlohmann::json& json) {
        GameObject::LoadProperties(json);

        if (json.contains("script")) {
            scriptPath = json["script"].get<std::string>();
        }

        // Stash custom properties so we can inject them into the instance
        // once the script is loaded.
        if (json.contains("properties")) {
            lua["_pendingProps"] = json["properties"].dump();
        }
    }

    void LuaBehaviour::SetScriptPath(const std::string& path) {
        scriptPath = path;
    }
    const std::string& LuaBehaviour::GetScriptPath() const {
        return scriptPath;
    }
    sol::state& LuaBehaviour::GetLuaState() {
        return lua;
    }

    // ================================================================
    //  Method dispatch
    // ================================================================
    void LuaBehaviour::CallMethod(const char* name) {
        sol::object instObj = lua["_instance"];
        if (!instObj.valid() || instObj.get_type() != sol::type::table) {
            return;
        }

        sol::table inst   = instObj.as<sol::table>();
        sol::object fnObj = inst[name];
        if (!fnObj.valid() || !fnObj.is<sol::protected_function>()) {
            return;
        }

        sol::protected_function fn = fnObj.as<sol::protected_function>();
        auto result                = fn(inst); // inst becomes self in the : call
        if (!result.valid()) {
            sol::error err = result;
            spdlog::error("[Lua {}::{}] {}", scriptPath, name, err.what());
        }
    }

    void LuaBehaviour::CallMethod(const char* name, float arg) {
        sol::object instObj = lua["_instance"];
        if (!instObj.valid() || instObj.get_type() != sol::type::table) {
            return;
        }

        sol::table inst   = instObj.as<sol::table>();
        sol::object fnObj = inst[name];
        if (!fnObj.valid() || !fnObj.is<sol::protected_function>()) {
            return;
        }

        sol::protected_function fn = fnObj.as<sol::protected_function>();
        auto result                = fn(inst, arg);
        if (!result.valid()) {
            sol::error err = result;
            spdlog::error("[Lua {}::{}] {}", scriptPath, name, err.what());
        }
    }

    void LuaBehaviour::CallMethod(const char* name, sol::table arg) {
        sol::object instObj = lua["_instance"];
        if (!instObj.valid() || instObj.get_type() != sol::type::table) {
            return;
        }

        sol::table inst   = instObj.as<sol::table>();
        sol::object fnObj = inst[name];
        if (!fnObj.valid() || !fnObj.is<sol::protected_function>()) {
            return;
        }

        sol::protected_function fn = fnObj.as<sol::protected_function>();
        auto result                = fn(inst, arg);
        if (!result.valid()) {
            sol::error err = result;
            spdlog::error("[Lua {}::{}] {}", scriptPath, name, err.what());
        }
    }

    // ================================================================
    //  Script loading
    // ================================================================
    void LuaBehaviour::LoadScript() {
        if (isScriptLoaded || scriptPath.empty()) {
            return;
        }

        auto& fs         = Engine::GetInstance().GetFileSystem();
        std::string code = fs.LoadAssetFileText(scriptPath);

        if (code.empty()) {
            spdlog::error("LuaBehaviour: failed to load '{}'", scriptPath);
            return;
        }

        auto result = lua.safe_script(code, sol::script_pass_on_error);
        if (!result.valid()) {
            sol::error err = result;
            spdlog::error("LuaBehaviour: error in '{}': {}", scriptPath, err.what());
            return;
        }

        // The script must return a table (the class).
        sol::object returnVal = result;
        sol::table classTable;

        if (returnVal.get_type() == sol::type::table) {
            classTable = returnVal.as<sol::table>();
        } else {
            spdlog::warn("LuaBehaviour: '{}' did not return a table  add 'return ClassName' at the end.", scriptPath);
            classTable = lua.create_table();
        }

        // Create the behaviour instance via the Lua helper.
        sol::protected_function createFn = lua["__createInstance"];
        auto createResult                = createFn(classTable, static_cast<GameObject*>(this));
        if (!createResult.valid()) {
            sol::error err = createResult;
            spdlog::error("LuaBehaviour: failed to create instance for '{}': {}", scriptPath, err.what());
            return;
        }

        sol::table inst = createResult.get<sol::table>();

        // Inject scene-file properties as  self.properties  table.
        if (lua["_pendingProps"].valid()) {
            std::string raw = lua["_pendingProps"].get<std::string>();
            try {
                nlohmann::json props = nlohmann::json::parse(raw);
                sol::table luaProps  = lua.create_table();
                for (auto& [key, value] : props.items()) {
                    if (value.is_number_float()) {
                        luaProps[key] = value.get<double>();
                    } else if (value.is_number_integer()) {
                        luaProps[key] = value.get<int>();
                    } else if (value.is_string()) {
                        luaProps[key] = value.get<std::string>();
                    } else if (value.is_boolean()) {
                        luaProps[key] = value.get<bool>();
                    }
                }
                inst["properties"] = luaProps;
            } catch (...) {
                inst["properties"] = raw;
            }
            lua["_pendingProps"] = sol::nil;
        }

        lua["_instance"] = inst;
        isScriptLoaded   = true;
        spdlog::info("LuaBehaviour: loaded '{}'", scriptPath);
    }

    void LuaBehaviour::ReloadScript() {
        isScriptLoaded   = false;
        didStart         = false;
        lua["_instance"] = sol::nil;

        LoadScript();
        CallMethod("Awake");
        CallMethod("Start");
        didStart = true;
    }

    // ================================================================
    //  Lua state initialisation & engine API bindings
    // ================================================================
    void LuaBehaviour::InitLuaState() {
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::coroutine);

        // Load the instance-creation helper
        lua.safe_script(LUA_RUNTIME);

        BindEngineAPI();
    }

    void LuaBehaviour::BindEngineAPI() {

        // ============================================================
        //  GLM types
        // ============================================================
        lua.new_usertype<glm::vec2>(
            "Vec2",
            sol::constructors<glm::vec2(), glm::vec2(float, float)>(),
            "x",
            &glm::vec2::x,
            "y",
            &glm::vec2::y,
            sol::meta_function::addition,
            [](const glm::vec2& a, const glm::vec2& b) { return a + b; },
            sol::meta_function::subtraction,
            [](const glm::vec2& a, const glm::vec2& b) { return a - b; },
            sol::meta_function::multiplication,
            sol::overload([](const glm::vec2& a, float s) { return a * s; }, [](float s, const glm::vec2& a) { return s * a; }),
            sol::meta_function::unary_minus,
            [](const glm::vec2& a) { return -a; },
            sol::meta_function::to_string,
            [](const glm::vec2& v) { return fmt::format("Vec2({:.2f}, {:.2f})", v.x, v.y); });

        lua.new_usertype<glm::vec3>(
            "Vec3",
            sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
            "x",
            &glm::vec3::x,
            "y",
            &glm::vec3::y,
            "z",
            &glm::vec3::z,
            sol::meta_function::addition,
            [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
            sol::meta_function::subtraction,
            [](const glm::vec3& a, const glm::vec3& b) { return a - b; },
            sol::meta_function::multiplication,
            sol::overload([](const glm::vec3& a, float s) { return a * s; }, [](float s, const glm::vec3& a) { return s * a; }),
            sol::meta_function::unary_minus,
            [](const glm::vec3& a) { return -a; },
            sol::meta_function::to_string,
            [](const glm::vec3& v) { return fmt::format("Vec3({:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z); });

        lua.new_usertype<glm::vec4>(
            "Vec4",
            sol::constructors<glm::vec4(), glm::vec4(float, float, float, float)>(),
            "x",
            &glm::vec4::x,
            "y",
            &glm::vec4::y,
            "z",
            &glm::vec4::z,
            "w",
            &glm::vec4::w,
            "r",
            &glm::vec4::r,
            "g",
            &glm::vec4::g,
            "b",
            &glm::vec4::b,
            "a",
            &glm::vec4::a,
            sol::meta_function::addition,
            [](const glm::vec4& a, const glm::vec4& b) { return a + b; },
            sol::meta_function::subtraction,
            [](const glm::vec4& a, const glm::vec4& b) { return a - b; },
            sol::meta_function::multiplication,
            sol::overload([](const glm::vec4& a, float s) { return a * s; }, [](float s, const glm::vec4& a) { return s * a; }),
            sol::meta_function::to_string,
            [](const glm::vec4& v) { return fmt::format("Vec4({:.2f}, {:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z, v.w); });

        lua.new_usertype<glm::quat>(
            "Quat",
            sol::constructors<glm::quat(), glm::quat(float, float, float, float)>(),
            "x",
            &glm::quat::x,
            "y",
            &glm::quat::y,
            "z",
            &glm::quat::z,
            "w",
            &glm::quat::w,
            sol::meta_function::to_string,
            [](const glm::quat& q) { return fmt::format("Quat({:.2f}, {:.2f}, {:.2f}, {:.2f})", q.x, q.y, q.z, q.w); });

        // Math helpers
        lua.set_function("EulerAngles", [](float pitch, float yaw, float roll) -> glm::quat {
            return glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), glm::radians(roll)));
        });
        lua.set_function(
            "Normalize",
            sol::overload([](const glm::vec3& v) { return glm::normalize(v); }, [](const glm::vec2& v) { return glm::normalize(v); }));
        lua.set_function("Distance",
                         sol::overload([](const glm::vec3& a, const glm::vec3& b) { return glm::distance(a, b); },
                                       [](const glm::vec2& a, const glm::vec2& b) { return glm::distance(a, b); }));
        lua.set_function("Lerp",
                         sol::overload([](float a, float b, float t) { return glm::mix(a, b, t); },
                                       [](const glm::vec3& a, const glm::vec3& b, float t) { return glm::mix(a, b, t); },
                                       [](const glm::vec2& a, const glm::vec2& b, float t) { return glm::mix(a, b, t); }));
        lua.set_function("Dot",
                         sol::overload([](const glm::vec3& a, const glm::vec3& b) { return glm::dot(a, b); },
                                       [](const glm::vec2& a, const glm::vec2& b) { return glm::dot(a, b); }));
        lua.set_function("Cross", [](const glm::vec3& a, const glm::vec3& b) { return glm::cross(a, b); });

        // ============================================================
        //  Component base
        // ============================================================
        lua.new_usertype<Component>("Component", sol::no_constructor, "GetOwner", &Component::GetOwner);

        // ============================================================
        //  Component usertypes
        // ============================================================

        // -- UI --
        lua.new_usertype<ButtonWidgetComponent>("ButtonWidgetComponent",
                                                sol::no_constructor,
                                                sol::base_classes,
                                                sol::bases<Component>(),
                                                "SetColor",
                                                &ButtonWidgetComponent::SetColor,
                                                "GetColor",
                                                &ButtonWidgetComponent::GetColor,
                                                "OnClick",
                                                sol::writeonly_property([](ButtonWidgetComponent* btn, sol::protected_function fn) {
                                                    btn->OnButtonClick = [fn]() mutable {
                                                        auto r = fn();
                                                        if (!r.valid()) {
                                                            sol::error e = r;
                                                            spdlog::error("[Lua button] {}", e.what());
                                                        }
                                                    };
                                                }));

        lua.new_usertype<TextWidgetComponent>("TextWidgetComponent",
                                              sol::no_constructor,
                                              sol::base_classes,
                                              sol::bases<Component>(),
                                              "SetText",
                                              &TextWidgetComponent::SetText,
                                              "GetText",
                                              &TextWidgetComponent::GetText,
                                              "SetTextColor",
                                              &TextWidgetComponent::SetTextColor,
                                              "GetTextColor",
                                              &TextWidgetComponent::GetTextColor,
                                              "SetFontSize",
                                              &TextWidgetComponent::SetFontSize,
                                              "GetFontSize",
                                              &TextWidgetComponent::GetFontSize,
                                              "SetShadowEnabled",
                                              &TextWidgetComponent::SetShadowEnabled,
                                              "SetOutlineEnabled",
                                              &TextWidgetComponent::SetOutlineEnabled);

        lua.new_usertype<CanvasComponent>("CanvasComponent",
                                          sol::no_constructor,
                                          sol::base_classes,
                                          sol::bases<Component>(),
                                          "SetReceivesInput",
                                          &CanvasComponent::SetReceivesInput,
                                          "GetReceivesInput",
                                          &CanvasComponent::GetReceivesInput,
                                          "SetCanvasMode",
                                          &CanvasComponent::SetCanvasMode,
                                          "GetCanvasMode",
                                          &CanvasComponent::GetCanvasMode);

        lua.new_usertype<RectTransformComponent>("RectTransformComponent",
                                                 sol::no_constructor,
                                                 sol::base_classes,
                                                 sol::bases<Component>(),
                                                 "GetSize",
                                                 &RectTransformComponent::GetSize,
                                                 "SetSize",
                                                 &RectTransformComponent::SetSize,
                                                 "GetAnchor",
                                                 &RectTransformComponent::GetAnchor,
                                                 "SetAnchor",
                                                 &RectTransformComponent::SetAnchor,
                                                 "GetPivot",
                                                 &RectTransformComponent::GetPivot,
                                                 "SetPivot",
                                                 &RectTransformComponent::SetPivot,
                                                 "GetScreenPosition",
                                                 &RectTransformComponent::GetScreenPosition);

        lua.new_usertype<PanelWidgetComponent>("PanelWidgetComponent",
                                               sol::no_constructor,
                                               sol::base_classes,
                                               sol::bases<Component>(),
                                               "SetColor",
                                               &PanelWidgetComponent::SetColor,
                                               "GetColor",
                                               &PanelWidgetComponent::GetColor);

        lua.new_usertype<ImageWidgetComponent>("ImageWidgetComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<SliderWidgetComponent>("SliderWidgetComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<ToggleWidgetComponent>("ToggleWidgetComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<InputFieldWidgetComponent>("InputFieldWidgetComponent",
                                                    sol::no_constructor,
                                                    sol::base_classes,
                                                    sol::bases<Component>(),
                                                    "SetText",
                                                    &InputFieldWidgetComponent::SetText,
                                                    "GetText",
                                                    &InputFieldWidgetComponent::GetText,
                                                    "SetPlaceholder",
                                                    &InputFieldWidgetComponent::SetPlaceholder,
                                                    "GetPlaceholder",
                                                    &InputFieldWidgetComponent::GetPlaceholder,
                                                    "SetInteractable",
                                                    &InputFieldWidgetComponent::SetInteractable,
                                                    "IsInteractable",
                                                    &InputFieldWidgetComponent::IsInteractable,
                                                    "SetReadOnly",
                                                    &InputFieldWidgetComponent::SetReadOnly,
                                                    "IsReadOnly",
                                                    &InputFieldWidgetComponent::IsReadOnly,
                                                    "Focus",
                                                    &InputFieldWidgetComponent::Focus,
                                                    "Unfocus",
                                                    &InputFieldWidgetComponent::Unfocus,
                                                    "IsFocused",
                                                    &InputFieldWidgetComponent::IsFocused,
                                                    "OnValueChanged",
                                                    sol::writeonly_property([](InputFieldWidgetComponent* f, sol::protected_function fn) {
                                                        f->OnValueChanged = [fn](const std::string& val) mutable {
                                                            auto r = fn(val);
                                                            if (!r.valid()) {
                                                                sol::error e = r;
                                                                spdlog::error("[Lua inputfield] {}", e.what());
                                                            }
                                                        };
                                                    }),
                                                    "OnSubmit",
                                                    sol::writeonly_property([](InputFieldWidgetComponent* f, sol::protected_function fn) {
                                                        f->OnSubmit = [fn](const std::string& val) mutable {
                                                            auto r = fn(val);
                                                            if (!r.valid()) {
                                                                sol::error e = r;
                                                                spdlog::error("[Lua inputfield] {}", e.what());
                                                            }
                                                        };
                                                    }),
                                                    "OnEndEdit",
                                                    sol::writeonly_property([](InputFieldWidgetComponent* f, sol::protected_function fn) {
                                                        f->OnEndEdit = [fn](const std::string& val) mutable {
                                                            auto r = fn(val);
                                                            if (!r.valid()) {
                                                                sol::error e = r;
                                                                spdlog::error("[Lua inputfield] {}", e.what());
                                                            }
                                                        };
                                                    }));

        lua.new_usertype<ProgressBarWidgetComponent>(
            "ProgressBarWidgetComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<DropdownWidgetComponent>(
            "DropdownWidgetComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<ScrollRectWidgetComponent>(
            "ScrollRectWidgetComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        // -- 3D --
        lua.new_usertype<CameraComponent>("CameraComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<MeshRendererComponent>("MeshRendererComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<PhysicsComponent>("PhysicsComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<CharacterControllerComponent>(
            "CharacterControllerComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<AnimationComponent>("AnimationComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<SkeletonAnimationComponent>("SkeletonAnimationComponent",
                                                     sol::no_constructor,
                                                     sol::base_classes,
                                                     sol::bases<Component>(),
                                                     "Play",
                                                     &SkeletonAnimationComponent::Play);

        lua.new_usertype<AudioComponent>("AudioComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<AudioListenerComponent>("AudioListenerComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<WorldEnvironmentComponent>(
            "WorldEnvironmentComponent", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        lua.new_usertype<DirectionalLightComponent>("DirectionalLightComponent",
                                                    sol::no_constructor,
                                                    sol::base_classes,
                                                    sol::bases<Component>(),
                                                    "SetDirection",
                                                    &DirectionalLightComponent::SetDirection,
                                                    "SetColor",
                                                    &DirectionalLightComponent::SetColor,
                                                    "SetIntensity",
                                                    &DirectionalLightComponent::SetIntensity,
                                                    "SetCastShadows",
                                                    &DirectionalLightComponent::SetCastShadows);

        lua.new_usertype<PointLightComponent>("PointLightComponent",
                                              sol::no_constructor,
                                              sol::base_classes,
                                              sol::bases<Component>(),
                                              "SetPosition",
                                              &PointLightComponent::SetPosition,
                                              "SetColor",
                                              &PointLightComponent::SetColor,
                                              "SetIntensity",
                                              &PointLightComponent::SetIntensity);

        lua.new_usertype<SpotlightComponent>("SpotlightComponent",
                                             sol::no_constructor,
                                             sol::base_classes,
                                             sol::bases<Component>(),
                                             "SetPosition",
                                             &SpotlightComponent::SetPosition,
                                             "SetDirection",
                                             &SpotlightComponent::SetDirection,
                                             "SetColor",
                                             &SpotlightComponent::SetColor,
                                             "SetIntensity",
                                             &SpotlightComponent::SetIntensity,
                                             "SetRange",
                                             &SpotlightComponent::SetRange);

        // -- 2D --
        lua.new_usertype<SpriteComponent2D>("SpriteComponent2D", sol::no_constructor, sol::base_classes, sol::bases<Component>());

        // ============================================================
        //  GetComponent  auto-dispatched via the resolver registry
        // ============================================================
        auto getComponentFn = [](sol::this_state ts, GameObject* go, const std::string& typeName) -> sol::object {
            auto& resolvers = GetComponentResolvers();
            auto it         = resolvers.find(typeName);
            if (it != resolvers.end()) {
                return it->second(ts, go);
            }
            spdlog::warn("[Lua] GetComponent: unknown type '{}'", typeName);
            return sol::make_object(sol::state_view(ts), sol::nil);
        };

        // ============================================================
        //  GameObject
        // ============================================================
        lua.new_usertype<GameObject>("GameObject",
                                     sol::no_constructor,

                                     // Identity
                                     "GetName",
                                     &GameObject::GetName,
                                     "SetName",
                                     &GameObject::SetName,
                                     "GetTag",
                                     &GameObject::GetTag,
                                     "SetTag",
                                     &GameObject::SetTag,
                                     "CompareTag",
                                     &GameObject::CompareTag,
                                     "IsActive",
                                     &GameObject::IsActive,
                                     "SetActive",
                                     &GameObject::SetActive,
                                     "Destroy",
                                     &GameObject::Destroy,
                                     "IsAlive",
                                     &GameObject::IsAlive,

                                     // Hierarchy
                                     "GetParent",
                                     &GameObject::GetParent,
                                     "FindChildByName",
                                     &GameObject::FindChildByName,

                                     // 3D Transform
                                     "GetPosition",
                                     &GameObject::GetPosition,
                                     "SetPosition",
                                     &GameObject::SetPosition,
                                     "GetWorldPosition",
                                     &GameObject::GetWorldPosition,
                                     "SetWorldPosition",
                                     &GameObject::SetWorldPosition,
                                     "GetRotation",
                                     &GameObject::GetRotation,
                                     "SetRotation",
                                     &GameObject::SetRotation,
                                     "GetWorldRotation",
                                     &GameObject::GetWorldRotation,
                                     "SetWorldRotation",
                                     &GameObject::SetWorldRotation,
                                     "GetScale",
                                     &GameObject::GetScale,
                                     "SetScale",
                                     &GameObject::SetScale,

                                     // 2D Transform
                                     "GetPosition2D",
                                     &GameObject::GetPosition2D,
                                     "SetPosition2D",
                                     &GameObject::SetPosition2D,
                                     "GetWorldPosition2D",
                                     &GameObject::GetWorldPosition2D,
                                     "GetRotation2D",
                                     &GameObject::GetRotation2D,
                                     "SetRotation2D",
                                     &GameObject::SetRotation2D,
                                     "GetScale2D",
                                     &GameObject::GetScale2D,
                                     "SetScale2D",
                                     &GameObject::SetScale2D,

                                     // Scene
                                     "GetScene",
                                     &GameObject::GetScene,

                                     // Generic component access
                                     "GetComponent",
                                     getComponentFn);

        // ============================================================
        //  Scene
        // ============================================================
        lua.new_usertype<Scene>("Scene",
                                sol::no_constructor,
                                "GetName",
                                &Scene::GetName,
                                "GetMainCamera",
                                &Scene::GetMainCamera,
                                "CreateObject",
                                sol::overload(static_cast<GameObject* (Scene::*) (const std::string&, GameObject*)>(&Scene::CreateObject),
                                              static_cast<GameObject* (Scene::*) (GameObject*)>(&Scene::CreateObject)),
                                "ChangeTo",
                                [](const std::string& path, bool transition) { Scene::ChangeTo(path, transition); });

        // ============================================================
        //  Input  (using new InputManager with SDL_Keycode)
        // ============================================================
        auto input = lua.create_named_table("Input");
        input.set_function("IsKeyPressed", [](SDL_Keycode k) { return Engine::GetInstance().GetInputManager().IsKeyPressed(k); });
        input.set_function("IsKeyJustPressed", [](SDL_Keycode k) { return Engine::GetInstance().GetInputManager().IsKeyJustPressed(k); });
        input.set_function("IsKeyJustReleased", [](SDL_Keycode k) { return Engine::GetInstance().GetInputManager().IsKeyJustReleased(k); });
        input.set_function("IsMouseButtonPressed", [](int b) { return Engine::GetInstance().GetInputManager().IsMouseButtonPressed(b); });
        input.set_function("IsMouseButtonJustPressed",
                           [](int b) { return Engine::GetInstance().GetInputManager().IsMouseButtonJustPressed(b); });
        input.set_function("GetMousePosition", []() { return Engine::GetInstance().GetInputManager().GetMousePosition(); });
        input.set_function("GetMouseDelta", []() { return Engine::GetInstance().GetInputManager().GetMouseDelta(); });
        input.set_function("GetMouseWheelDelta", []() { return Engine::GetInstance().GetInputManager().GetMouseWheelDelta(); });
        input.set_function("GetAxis", [](const std::string& n) { return Engine::GetInstance().GetInputManager().GetAxis(n); });
        input.set_function("GetAxisRaw", [](const std::string& n) { return Engine::GetInstance().GetInputManager().GetAxisRaw(n); });
        input.set_function("AnyKey", []() { return Engine::GetInstance().GetInputManager().AnyKey(); });
        input.set_function("AnyKeyDown", []() { return Engine::GetInstance().GetInputManager().AnyKeyDown(); });
        input.set_function("BindAction",
                           [](const std::string& a, SDL_Keycode k) { Engine::GetInstance().GetInputManager().BindAction(a, k); });
        input.set_function("IsActionPressed",
                           [](const std::string& a) { return Engine::GetInstance().GetInputManager().IsActionPressed(a); });
        input.set_function("IsActionJustPressed",
                           [](const std::string& a) { return Engine::GetInstance().GetInputManager().IsActionJustPressed(a); });
        input.set_function("IsActionJustReleased",
                           [](const std::string& a) { return Engine::GetInstance().GetInputManager().IsActionJustReleased(a); });

        // ============================================================
        //  Cursor
        // ============================================================
        auto cursor = lua.create_named_table("Cursor");
        cursor.set_function("Lock", []() { Cursor::SetCursorLockState(ECursorLockState::CURSOR_LOCKED); });
        cursor.set_function("Unlock", []() { Cursor::SetCursorLockState(ECursorLockState::CURSOR_UNLOCKED); });
        cursor.set_function("SetEnabled", [](bool e) { Cursor::SetCursorEnabled(e); });

        // ============================================================
        //  Audio
        // ============================================================
        lua.new_usertype<Audio>("Audio",
                                sol::no_constructor,
                                "Play",
                                &Audio::Play,
                                "Stop",
                                &Audio::Stop,
                                "Pause",
                                &Audio::Pause,
                                "Resume",
                                &Audio::Resume,
                                "SetVolume",
                                &Audio::SetVolume,
                                "GetVolume",
                                &Audio::GetVolume,
                                "SetPitch",
                                &Audio::SetPitch,
                                "IsPlaying",
                                &Audio::IsPlaying,
                                "Load",
                                [](const std::string& path) { return Audio::Load(path); });

        // ============================================================
        //  Engine / SceneManager
        // ============================================================
        auto eng = lua.create_named_table("Engine");
        eng.set_function("GetScene", []() -> Scene* { return Engine::GetInstance().GetScene(); });
        eng.set_function("Quit", []() {
            if (auto* app = Engine::GetInstance().GetApplication()) {
                app->Close();
            }
        });

        auto sceneMgr = lua.create_named_table("SceneManager");
        sceneMgr.set_function("LoadScene", [](const std::string& path, bool transition) { Scene::ChangeTo(path, transition); });

        // ============================================================
        //  Debug  (delegates to C++ Debug class)
        // ============================================================
        auto debug = lua.create_named_table("Debug");
        debug.set_function("print", [](sol::variadic_args va) {
            std::string msg;
            for (auto v : va) {
                if (!msg.empty()) {
                    msg += "\t";
                }
                msg += sol::state_view(v.lua_state())["tostring"](v.get<sol::object>()).get<std::string>();
            }
            Debug::Log(msg);
        });
        debug.set_function("Log", [](const std::string& m) { Debug::Log(m); });
        debug.set_function("LogWarning", [](const std::string& m) { Debug::LogWarning(m); });
        debug.set_function("LogError", [](const std::string& m) { Debug::LogError(m); });

        // ============================================================
        //  Time  (delegates to C++ Time class)
        // ============================================================
        auto time = lua.create_named_table("Time");
        time.set_function("GetTime", []() { return Time::GetTime(); });
        time.set_function("GetDeltaTime", []() { return Time::GetDeltaTime(); });
        time.set_function("GetUnscaledDeltaTime", []() { return Time::GetUnscaledDeltaTime(); });
        time.set_function("GetTimeScale", []() { return Time::GetTimeScale(); });
        time.set_function("SetTimeScale", [](float s) { Time::SetTimeScale(s); });
        time.set_function("GetRealtimeSinceStartup", []() { return Time::GetRealtimeSinceStartup(); });
        time.set_function("GetFrameCount", []() { return Time::GetFrameCount(); });
        time.set_function("GetFps", []() { return Time::GetFps(); });
        time.set_function("GetSmoothDeltaTime", []() { return Time::GetSmoothDeltaTime(); });
        time.set_function("GetFixedDeltaTime", []() { return Time::GetFixedDeltaTime(); });
        time.set_function("SetFixedDeltaTime", [](float dt) { Time::SetFixedDeltaTime(dt); });

        // ============================================================
        //  Key / Mouse constants
        // ============================================================
        auto keys            = lua.create_named_table("Keys");
        keys["A"]            = SDLK_A;
        keys["B"]            = SDLK_B;
        keys["C"]            = SDLK_C;
        keys["D"]            = SDLK_D;
        keys["E"]            = SDLK_E;
        keys["F"]            = SDLK_F;
        keys["G"]            = SDLK_G;
        keys["H"]            = SDLK_H;
        keys["I"]            = SDLK_I;
        keys["J"]            = SDLK_J;
        keys["K"]            = SDLK_K;
        keys["L"]            = SDLK_L;
        keys["M"]            = SDLK_M;
        keys["N"]            = SDLK_N;
        keys["O"]            = SDLK_O;
        keys["P"]            = SDLK_P;
        keys["Q"]            = SDLK_Q;
        keys["R"]            = SDLK_R;
        keys["S"]            = SDLK_S;
        keys["T"]            = SDLK_T;
        keys["U"]            = SDLK_U;
        keys["V"]            = SDLK_V;
        keys["W"]            = SDLK_W;
        keys["X"]            = SDLK_X;
        keys["Y"]            = SDLK_Y;
        keys["Z"]            = SDLK_Z;
        keys["Space"]        = SDLK_SPACE;
        keys["Return"]       = SDLK_RETURN;
        keys["Escape"]       = SDLK_ESCAPE;
        keys["Tab"]          = SDLK_TAB;
        keys["Backspace"]    = SDLK_BACKSPACE;
        keys["Delete"]       = SDLK_DELETE;
        keys["LShift"]       = SDLK_LSHIFT;
        keys["RShift"]       = SDLK_RSHIFT;
        keys["LCtrl"]        = SDLK_LCTRL;
        keys["RCtrl"]        = SDLK_RCTRL;
        keys["LAlt"]         = SDLK_LALT;
        keys["RAlt"]         = SDLK_RALT;
        keys["Up"]           = SDLK_UP;
        keys["Down"]         = SDLK_DOWN;
        keys["Left"]         = SDLK_LEFT;
        keys["Right"]        = SDLK_RIGHT;
        keys["Home"]         = SDLK_HOME;
        keys["End"]          = SDLK_END;
        keys["PageUp"]       = SDLK_PAGEUP;
        keys["PageDown"]     = SDLK_PAGEDOWN;
        keys["Insert"]       = SDLK_INSERT;
        keys["CapsLock"]     = SDLK_CAPSLOCK;
        keys["Num0"]         = SDLK_0;
        keys["Num1"]         = SDLK_1;
        keys["Num2"]         = SDLK_2;
        keys["Num3"]         = SDLK_3;
        keys["Num4"]         = SDLK_4;
        keys["Num5"]         = SDLK_5;
        keys["Num6"]         = SDLK_6;
        keys["Num7"]         = SDLK_7;
        keys["Num8"]         = SDLK_8;
        keys["Num9"]         = SDLK_9;
        keys["F1"]           = SDLK_F1;
        keys["F2"]           = SDLK_F2;
        keys["F3"]           = SDLK_F3;
        keys["F4"]           = SDLK_F4;
        keys["F5"]           = SDLK_F5;
        keys["F6"]           = SDLK_F6;
        keys["F7"]           = SDLK_F7;
        keys["F8"]           = SDLK_F8;
        keys["F9"]           = SDLK_F9;
        keys["F10"]          = SDLK_F10;
        keys["F11"]          = SDLK_F11;
        keys["F12"]          = SDLK_F12;
        keys["Minus"]        = SDLK_MINUS;
        keys["Equals"]       = SDLK_EQUALS;
        keys["LeftBracket"]  = SDLK_LEFTBRACKET;
        keys["RightBracket"] = SDLK_RIGHTBRACKET;
        keys["Backslash"]    = SDLK_BACKSLASH;
        keys["Semicolon"]    = SDLK_SEMICOLON;
        keys["Quote"]        = SDLK_APOSTROPHE;
        keys["Comma"]        = SDLK_COMMA;
        keys["Period"]       = SDLK_PERIOD;
        keys["Slash"]        = SDLK_SLASH;
        keys["Backquote"]    = SDLK_GRAVE;

        auto mouse      = lua.create_named_table("Mouse");
        mouse["Left"]   = SDL_BUTTON_LEFT;
        mouse["Right"]  = SDL_BUTTON_RIGHT;
        mouse["Middle"] = SDL_BUTTON_MIDDLE;
    }

} // namespace golias
