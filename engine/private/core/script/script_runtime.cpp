#include "core/script/script_runtime.h"

#include "golias_module.h"

#include <pocketpy.h>

namespace golias {

    bool ScriptRuntime::sInitialized = false;

    bool ScriptRuntime::IsAlive() {
        return sInitialized;
    }

    bool ScriptRuntime::Initialize() {

        py_initialize();
        
        RegisterModules();

        sInitialized = true;

        GOLIAS_LOG_INFO("Python runtime initialized.");
        return true;
    }

    void* ScriptRuntime::FindInstance(CString name) {

        if (!py_getattr(py_getmodule("golias.engine"), py_name("_instances"))) {
            py_clearexc(nullptr);
            return nullptr;
        }
        py_Ref instances = py_retval();

        py_GlobalRef key = py_name2ref(py_name(std::string(name).c_str()));
        if (!py_getitem(instances, key)) {
            py_clearexc(nullptr);
            return nullptr;
        }

        // Borrowed ref
        return py_retval();
    }

    bool ScriptRuntime::StoreInstance(CString name, void* instance) {

        py_assign(py_r1(), (py_Ref)instance);

        if (!py_getattr(py_getmodule("golias.engine"), py_name("_instances"))) {
            py_printexc();
            py_clearexc(nullptr);
            return false;
        }
        py_Ref instances = py_retval();

        py_GlobalRef key = py_name2ref(py_name(std::string(name).c_str()));
        if (!py_setitem(instances, key, py_r1())) {
            py_printexc();
            py_clearexc(nullptr);
            return false;
        }

        return true;
    }

    bool ScriptRuntime::Exec(const String& source, CString name) {
        if (!py_exec(source.c_str(), name.data(), EXEC_MODE, nullptr)) {
            py_printexc(); // dump traceback
            py_clearexc(nullptr);
            GOLIAS_LOG_ERROR("Python exec failed for '%s'.", name.data());
            return false;
        }

        py_callbacks()->flush();

        return true;
    }

    void ScriptRuntime::RegisterModules() {
        register_golias_py_module();
    }

    void ScriptRuntime::Shutdown() {
        sInitialized = false;
        py_finalize();
    }

} // namespace golias
