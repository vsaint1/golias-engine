#pragma once
#include "stdafx.h"

namespace golias {

    class ScriptRuntime {
    public:
        bool Initialize();

        void Shutdown();

        /// Finds an instance of a script object with the given name. 
        void* FindInstance(CString name);

        bool Exec(const String& source, CString name);

        /// Stores an instance of a script object with the given name. 
        bool StoreInstance(CString name, void* instance);

    private:
        void RegisterModules();
    };
} // namespace golias
