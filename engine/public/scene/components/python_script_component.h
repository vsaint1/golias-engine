#pragma once

#include "component.h"

extern "C" {
    struct py_TValue; // (pocketpy) forward declare
}

namespace golias {

    class PythonScriptComponent : public Component {
        COMPONENT(PythonScriptComponent)
    public:
        PythonScriptComponent() = default;
        ~PythonScriptComponent() override;

        bool LoadProperties(const Json& properties) override;
        bool SaveProperties(Json& properties) const override;

        void Start() override;
        void Update(float deltaTime) override;

        void OnEnable() override;
        void OnDisable() override;

    private:
        bool LoadScript();

        bool CallPyFunction(const char* name, py_TValue* args = nullptr, int argc = 0);

    private:
        String mScriptPath;
        String mClassName;
        Json mProperties = Json::object();

        String mInstanceKey;
        bool mScriptLoaded = false;
    };

} // namespace golias
