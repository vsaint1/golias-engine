#pragma once

#include "behaviour.h"

#include <sol.hpp>
#include <string>

namespace golias {

    /**
     * @brief A Behaviour driven by a Lua script using class-style OOP.
     *
     * Scripts return a table (the "class") and define methods with `:` syntax:
     *
     *     local MyScript = {}
     *
     *     function MyScript:Awake()
     *         Log("Hello from " .. self:GetName())
     *     end
     *
     *     function MyScript:Update(dt)
     *         -- self delegates to the C++ GameObject automatically
     *         local pos = self:GetPosition()
     *         self:SetPosition(pos + Vec3(0, dt, 0))
     *     end
     *
     *     return MyScript
     *
     * Lifecycle (Unity-style):
     *   Awake  -> Start -> Update(dt) -> OnDestroy
     */
    class LuaBehaviour : public Behaviour {
        GCLASS(LuaBehaviour)

    public:
        LuaBehaviour();
        ~LuaBehaviour() override;

        // ---- Behaviour lifecycle ----
        void Awake() override;
        void Start() override;
        void Update(float deltaTime) override;
        void OnDestroy() override;

        // ---- Property loading from .gscene ----
        void LoadProperties(const nlohmann::json& json) override;

        // ---- Script accessors ----
        void SetScriptPath(const std::string& path);
        const std::string& GetScriptPath() const;

        /// Reload the Lua file at runtime (hot-reload)
        void ReloadScript();

        /// Access the Lua environment for this behaviour
        sol::state& GetLuaState();

    private:
        void InitLuaState();
        void BindEngineAPI();
        void LoadScript();

        void CallMethod(const char* name);
        void CallMethod(const char* name, float arg);

        sol::state lua;
        std::string scriptPath;

        bool isScriptLoaded = false;
        bool didStart       = false;
    };

} // namespace golias
