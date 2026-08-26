#pragma once

#include "stdafx.h"

namespace golias {

    enum class ShaderTarget {
        OpenGLCore330,
        OpenGLES300,
    };

    struct CompiledShaderSource {
        String Vertex;
        String Fragment;

        bool IsValid() const {
            return !Vertex.empty() && !Fragment.empty();
        }
    };


    /// @brief A simple shader compiler that preprocesses shader source code.
    CompiledShaderSource ShaderCompiler_Compile(const String& source, ShaderTarget target, CString assetPath);


} // namespace golias
