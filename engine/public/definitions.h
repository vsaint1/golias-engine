#pragma once
#define ENGINE_NAME        "GOLIAS_ENGINE"
#define ENGINE_VERSION_STR "0.0.5"

#define ENGINE_DEFAULT_FOLDER_NAME "Golias Engine"
#define ENGINE_PACKAGE_NAME        "com.golias.engine.app"

#define ALBEDO_TEXTURE_UNIT            0
#define SPECULAR_TEXTURE_UNIT          1
#define METALLIC_TEXTURE_UNIT          2
#define ROUGHNESS_TEXTURE_UNIT         3
#define NORMAL_MAP_TEXTURE_UNIT        4
#define AMBIENT_OCCLUSION_TEXTURE_UNIT 5
#define EMISSIVE_TEXTURE_UNIT          6
#define SHADOW_TEXTURE_UNIT            7
#define ENVIRONMENT_TEXTURE_UNIT       8

#define MAX_VERTEX_MEMORY  (512 * 1024)
#define MAX_ELEMENT_MEMORY (128 * 1024)

#if !defined(NDEBUG)
    #if defined(_MSC_VER)
        #define GOLIAS_ASSERT_BREAK() __debugbreak()
    #elif defined(__clang__) || defined(__GNUC__)
        #define GOLIAS_ASSERT_BREAK() __builtin_trap()
    #else
        #define GOLIAS_ASSERT_BREAK() std::abort()
    #endif
#endif

/*!
* @defgroup Components
* @defgroup Systems
* @defgroup Core
* @defgroup Tags
* @defgroup FileSystem
* @defgroup Configuration
* @defgroup Rendering
* @defgroup Time
* @defgroup Logging
*/
