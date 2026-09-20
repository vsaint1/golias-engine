#include "scene/components/python_script_component.h"

#include "core/engine.h"
#include "core/io/file_system.h"
#include "core/script/golias_module.h"
#include "core/script/script_runtime.h"
#include "scene/game_object.h"
#include <pocketpy.h>

namespace golias {


    constexpr const char* kInstancePrefix = "gpc:"; // golias python component

    bool to_python_value(py_OutRef out, const Json& val) {
        if (val.is_number_integer()) {
            py_newint(out, (py_i64) val.get<int64_t>());
            return true;
        }

        if (val.is_number_float()) {
            py_newfloat(out, val.get<double>());
            return true;
        }

        if (val.is_boolean()) {
            py_newbool(out, val.get<bool>());
            return true;
        }

        if (val.is_string()) {
            py_newstr(out, val.get<std::string>().c_str());
            return true;
        }

        if (val.is_object() && val.contains("x") && val.contains("y") && val.contains("z")) {
            push_vector3(out, Vector3{val["x"].get<float>(), val["y"].get<float>(), val["z"].get<float>()});
            return true;
        }

        return false;
    }

    bool from_python_value(py_Ref val, Json& out) {
        if (py_isbool(val)) {
            out = py_tobool(val);
            return true;
        }

        if (py_isint(val)) {
            out = (int64_t) py_toint(val);
            return true;
        }

        if (py_isfloat(val)) {
            out = py_tofloat(val);
            return true;
        }

        if (py_isstr(val)) {
            out = py_tostr(val);
            return true;
        }

        if (py_isinstance(val, get_vector3_type())) {
            Vector3 v;
            if (unpack_vector3(val, v)) {
                out      = Json::object();
                out["x"] = v.x;
                out["y"] = v.y;
                out["z"] = v.z;
                return true;
            }
        }

        return false;
    }


    PythonScriptComponent::~PythonScriptComponent() {

        // Ensure the VM still alive
        if (!mScriptLoaded || !ScriptRuntime::IsAlive()) {
            return;
        }

        CallPyFunction("on_destroy");

        if (!py_getattr(py_getmodule("golias.engine"), py_name("_instances"))) {
            py_clearexc(nullptr);
            return;
        }

        py_delitem(py_retval(), py_name2ref(py_name(mInstanceKey.c_str())));
        if (py_checkexc()) {
            py_clearexc(nullptr);
        }
    }

    bool PythonScriptComponent::LoadProperties(const Json& properties) {
        Component::LoadProperties(properties);

        if (properties.contains("script") && properties["script"].is_object()) {
            const Json& script = properties["script"];

            mScriptPath = script.value("path", "");
            mClassName  = script.value("class_name", "");

            if (script.contains("properties") && script["properties"].is_object()) {
                mProperties = script["properties"];
            }
        }

        if (mScriptPath.empty()) {
            GOLIAS_LOG_ERROR("PythonScriptComponent has no 'script' property.");
            return false;
        }

        // Default class = file stem, e.g. "scripts/spin.py" -> "Spin".
        if (mClassName.empty()) {
            mClassName = Path(mScriptPath).stem().string();
        }


        return true;
    }

    bool PythonScriptComponent::SaveProperties(Json& properties) const {

        Component::SaveProperties(properties);

        Json& script = properties["script"];

        script["path"]       = mScriptPath;
        script["class_name"] = mClassName;

        Json saved = Json::object();

        if (mScriptLoaded && ScriptRuntime::IsAlive()) {
            ScriptRuntime& rt = Engine::GetInstance().GetScriptRuntime();
            py_Ref instance   = (py_Ref) rt.FindInstance(mInstanceKey.c_str());

            if (instance) {
                py_assign(py_r2(), instance);

                for (auto it = mProperties.begin(); it != mProperties.end(); ++it) {
                    const char* key = it.key().c_str();

                    // TODO: Later we can have better logic to Serialize PyObjects (like annotations)
                    if (key[0] == '_') {
                        continue; // python-private: never serialized
                    }

                    if (!py_getattr(py_r2(), py_name(key))) {
                        py_clearexc(nullptr);
                        continue;
                    }

                    Json out;
                    if (from_python_value(py_retval(), out)) {
                        saved[key] = out;
                    }
                }
            }
        }

        if (!saved.empty()) {
            properties["properties"] = saved;
        }

        return true;
    }

    void PythonScriptComponent::Start() {

        if (!LoadScript()) {
            SetEnabled(false);
            return;
        }

        CallPyFunction("start");
    }

    void PythonScriptComponent::Update(float deltaTime) {

        if (mScriptLoaded) {
            py_TValue args[1];
            py_newfloat(&args[0], deltaTime);
            CallPyFunction("update", args, 1);
        }
    }

    void PythonScriptComponent::OnEnable() {
        CallPyFunction("on_enable");
    }

    void PythonScriptComponent::OnDisable() {
        CallPyFunction("on_disable");
    }

    bool PythonScriptComponent::LoadScript() {

        ScriptRuntime& rt = Engine::GetInstance().GetScriptRuntime();

        const String source = Engine::GetInstance().GetFileSystem().LoadAssetFileText(mScriptPath.c_str());
        if (source.empty()) {
            GOLIAS_LOG_ERROR("PythonScript '%s' not found or empty.", mScriptPath.c_str());
            return false;
        }

        // Try to execute the script in the __main__ module.
        if (!rt.Exec(source, mScriptPath.c_str())) {
            GOLIAS_LOG_ERROR("Failed to execute Python script '%s'.", mScriptPath.c_str());
            return false;
        }

        // Fetch the class object.
        py_ItemRef cls = py_getglobal(py_name(mClassName.c_str()));
        if (!cls) {
            GOLIAS_LOG_ERROR("PythonScript '%s' has no class '%s'.", mScriptPath.c_str(), mClassName.c_str());
            return false;
        }

        // ** Must be a subclass of PythonBehavior **.
        const py_Type behaviorType = py_gettype("__main__", py_name("PythonBehavior"));
        if (!py_issubclass(py_totype(cls), behaviorType)) {
            GOLIAS_LOG_ERROR("'%s' in '%s' does not subclass PythonBehavior.", mClassName.c_str(), mScriptPath.c_str());
            return false;
        }

        // One instance per component - py_retval().
        if (!py_tpcall(py_totype(cls), 0, nullptr)) {
            py_printexc();
            py_clearexc(nullptr);
            GOLIAS_LOG_ERROR("PythonScript 'class %s' failed to construct.", mClassName.c_str());
            return false;
        }


        mInstanceKey = String_Format("%s_%s", kInstancePrefix, GetOwner()->GetName().c_str());
        rt.StoreInstance(mInstanceKey.c_str(), py_retval());

        py_Ref instance = (py_Ref) rt.FindInstance(mInstanceKey.c_str());
        if (!instance) {
            GOLIAS_LOG_ERROR("PythonScript '%s' instance could not be stored.", mScriptPath.c_str());
            return false;
        }

        // Inject owner and serialized fields (before lifecycle)
        inject_game_object(instance, GetOwner());

        for (auto it = mProperties.begin(); it != mProperties.end(); ++it) {
            if (to_python_value(py_r0(), it.value())) {
                py_setattr(instance, py_name(it.key().c_str()), py_r0());
            } else {
                GOLIAS_LOG_WARN("Property '%s' skipped (unmapped JSON type).", it.key().c_str());
            }
        }

        GOLIAS_LOG_INFO("PythonScript '%s' (%s) loaded on '%s'.", mScriptPath.c_str(), mClassName.c_str(), GetOwner()->GetName().c_str());

        mScriptLoaded = true;
        return true;
    }

    bool PythonScriptComponent::CallPyFunction(const char* name, py_TValue* args, int argc) {

        if (!mScriptLoaded || !ScriptRuntime::IsAlive()) {
            return true;
        }

        ScriptRuntime& rt = Engine::GetInstance().GetScriptRuntime();
        py_Ref instance   = (py_Ref) rt.FindInstance(mInstanceKey.c_str());
        if (!instance) {
            return true; // script not loaded
        }

        py_assign(py_r2(), instance);

        if (!py_getattr(py_r2(), py_name(name))) {
            py_printexc();
            py_clearexc(nullptr);
            GOLIAS_LOG_ERROR("Python '%s' on '%s' raised:", name, mScriptPath.c_str());
            SetEnabled(false);
            return false;
        }

        py_assign(py_r0(), py_retval()); // bound method; retval can't be re-input

        if (!py_call(py_r0(), argc, args)) {
            GOLIAS_LOG_ERROR("Python '%s' on '%s' raised:", name, mScriptPath.c_str());
            py_printexc();
            py_clearexc(nullptr);
            SetEnabled(false);
            return false;
        }

        return true;
    }


} // namespace golias
