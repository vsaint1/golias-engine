set_project("ember_engine")
set_languages("cxx20")
set_license("MIT")
set_version("0.0.1", {build = "%Y%m%d%H%M"})

add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
add_rules("mode.debug", "mode.release")

-- === Dependencies ===
-- Build static on macOS/iOS/Emscripten, shared otherwise
local use_shared = not (is_plat("macosx") or is_plat("iphoneos") or is_plat("wasm"))

add_requires("lua v5.4.7", {configs = {shared = use_shared}})
add_requires("libsdl3 3.2.22", {configs = {shared = use_shared}})
add_requires("libsdl3_ttf 3.2.2", {configs = {shared = use_shared, harfbuzz = true, plutosvg = true}})
add_requires("libsdl3_image 3.2.0", {configs = {shared = use_shared}})
add_requires("flecs v4.1.1", {configs = {shared = false}})
add_requires("nlohmann_json v3.12.0", {configs = {shared = false}})
add_requires("glm 1.0.1", {configs = {shared = false}})
add_requires("miniaudio 0.11.23", "tinyxml2 11.0.0", {configs = {shared = false}})
add_requires("assimp v5.4.0", {configs = {shared = false}})


if not (is_plat("wasm") or is_plat("android") or is_plat("iphoneos")) then
    add_requires("doctest v2.4.9", {configs = {shared = false}})
end

target("glad")
    set_kind("static")
    add_files("vendor/glad/src/glad.c")
    add_includedirs("vendor/glad/include", {public = true})

-- === Engine Target ===
target("engine")
    set_kind("static")
    add_files("engine/private/**/*.cpp")
    add_includedirs("engine/public", {public = true})
    
    -- glad (gl/es2)
    add_deps("glad")

    add_includedirs("vendor/glad/include", {public = true})
    add_files("vendor/glad/src/glad.c")

    set_pcxxheader("engine/public/stdafx.h")

    add_packages(
        "libsdl3",
        "libsdl3_ttf",
        "libsdl3_image",
        "lua",
        "flecs",
        "nlohmann_json",
        "glad",
        "glm",
        "miniaudio",
        "tinyxml2",
        "assimp",
        {public = true}
    )

    if is_plat("android") or is_plat("linux") then
        add_cxflags("-fPIC")
    end

    if is_plat("macosx") or is_plat("iphoneos") then
        add_cxflags("-fobjc-arc", "-fPIC")
    end

-- === Client Target ===
target("client")
    set_kind("binary")
    add_files("client/*.cpp")
    add_deps("engine")
    add_includedirs("engine/public")

    if is_plat("android") then
        set_kind("shared")
        add_syslinks("log", "android", "m", "dl")
    end

 if is_plat("wasm") then
        
       set_basename("index")

       add_ldflags(
           "-sFULL_ES3=1",
           "-sMIN_WEBGL_VERSION=2",
           "-sMAX_WEBGL_VERSION=2",
           "-sASSERTIONS=1",
           "-sFETCH=1",
           "-sUSE_SDL=3",
           "-sUSE_SDL_IMAGE=2",
           "-sUSE_SDL_TTF=2",
           "-sUSE_FREETYPE=1",
           "-s ALLOW_MEMORY_GROWTH=1 ",
           "-s EXPORTED_RUNTIME_METHODS=cwrap",
           "-s STACK_SIZE=1mb",
           "--preload-file=res@/res",
           "-g")

     end

    if not (is_plat("wasm") or is_plat("android")) then
        after_build(function (target)
            os.cp("res", path.join(target:targetdir(), "res"))
        end)
    end

-- === Tests ===
if not (is_plat("wasm") or is_plat("android") or is_plat("iphoneos")) then
    for _, file in ipairs(os.files("tests/test_*.cpp")) do
        local name = path.basename(file)
        target(name)
            set_kind("binary")
            set_default(false)
            add_files(file)
            add_packages("doctest")
            add_defines("DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN")
            add_deps("engine")
            add_includedirs("engine/public")
            add_tests(name)
    end
end
