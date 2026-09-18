#include "core/script/script_runtime.h"

#include <pocketpy.h>


namespace golias {

    bool ScriptRuntime::Initialize() {

        py_initialize();

        RegisterModules();

        GOLIAS_LOG_INFO("Python runtime initialized.");
        return true;
    }

    void* ScriptRuntime::FindInstance(CString name) {

        return nullptr;
    }

    bool ScriptRuntime::StoreInstance(CString name, void* instance) {
        return true;
    }

    bool ScriptRuntime::Exec(const String& source, CString name) {
        if (!py_exec(source.c_str(), name.data(), EXEC_MODE, nullptr)) {
            py_printexc(); // dump traceback
            GOLIAS_LOG_ERROR("Python exec failed for '%s'.", name.data());
            return false;
        }

        py_callbacks()->flush();

        return true;
    }

    void ScriptRuntime::RegisterModules() {
    }

    void ScriptRuntime::Shutdown() {
        py_finalize();
    }

} // namespace golias
