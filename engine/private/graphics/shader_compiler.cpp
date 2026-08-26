#include "graphics/shader_compiler.h"

namespace golias {

    namespace {
        constexpr CString kOpenglCoreVersion = "#version 330 core";
        constexpr CString kOpenglEsVersion   = "#version 300 es";
        constexpr CString kOpenglEsPrecision = "\nprecision highp float;\nprecision highp int;";

        CString version_for(ShaderTarget target) {
            return target == ShaderTarget::OpenGLES300 ? kOpenglEsVersion : kOpenglCoreVersion;
        }

        String compiled_stage(String source, ShaderTarget target, CString assetPath, CString stageName) {
            const size_t versionStart = source.find("#version");

            if (versionStart != String::npos) {
                const size_t versionEnd = source.find('\n', versionStart);
                source.replace(versionStart,
                               versionEnd == String::npos ? String::npos : versionEnd - versionStart,
                               version_for(target).data(),
                               version_for(target).size());
            } else {
                source.insert(0, version_for(target).data(), version_for(target).size());
                source.insert(version_for(target).size(), 1, '\n');
            }

            if (target == ShaderTarget::OpenGLES300) {
                const size_t precisionEnd = source.find('\n');
                source.insert(precisionEnd == String::npos ? source.size() : precisionEnd, kOpenglEsPrecision);
            }

            return source;
        }
    } // namespace

    CompiledShaderSource ShaderCompiler_Compile(const String& source, ShaderTarget target, CString assetPath) {
        const size_t vertexMarker   = source.find("@vertex");
        const size_t fragmentMarker = source.find("@fragment");

        if (vertexMarker == String::npos || fragmentMarker == String::npos || vertexMarker >= fragmentMarker) {
            GOLIAS_LOG_ERROR("Shader '%s' must contain @vertex followed by @fragment.", assetPath.data());
            return {};
        }

        const size_t vertexStart   = source.find('\n', vertexMarker);
        const size_t fragmentStart = source.find('\n', fragmentMarker);
        if (vertexStart == String::npos || fragmentStart == String::npos) {
            GOLIAS_LOG_ERROR("Shader '%s' has an invalid stage marker.", assetPath.data());
            return {};
        }

        auto vertex = compiled_stage(source.substr(vertexStart + 1, fragmentMarker - vertexStart - 1), target, assetPath, "vertex");

        auto fragment = compiled_stage(source.substr(fragmentStart + 1), target, assetPath, "fragment");

        if (vertex.empty() || fragment.empty()) {
            return {};
        }

        CompiledShaderSource compiled = {std::move(vertex), std::move(fragment)};

        return compiled;
    }


} // namespace golias
