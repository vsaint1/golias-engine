set_project("ember_engine")
set_languages("cxx20")
set_license("MIT")

local base_version = "0.0.3"

set_version(base_version, {build = "%Y%m%d%H%M"})


add_defines(string.format("ENGINE_VERSION_STR=\"%s\"", base_version))

add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
add_rules("mode.debug", "mode.release")

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
add_requires("nuklear 4.12.7", {configs = {shared = false}})

add_options("mode", {description = "Engine mode 2D/3D", default = "2D", values = {"2D", "3D"}})

if not (is_plat("wasm") or is_plat("android") or is_plat("iphoneos")) then
    add_requires("doctest v2.4.9", {configs = {shared = false}})
end

if get_config("mode") == "3D" then
    add_defines("EMBER_3D")
    printf("Ember Engine - Building in 3D mode (OPENGL) | Version %s | Date: %s\n", base_version, os.date("%Y-%m-%d %H:%M"))
else
    add_defines("EMBER_2D")
    printf("Ember Engine - Building in 2D mode (AUTO) | Version %s | Date: %s\n", base_version, os.date("%Y-%m-%d %H:%M"))
end


target("glad")
    set_kind("static")
    add_files("vendor/glad/src/glad.c")
    add_includedirs("vendor/glad/include", {public = true})

target("engine")
    set_kind("static")
    add_files("engine/private/**/*.cpp")
    add_files("engine/private/*.cpp")
    add_includedirs("engine/public", {public = true})
   
    add_deps("glad") -- using glad vendored, from repository cant build to wasm

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
        "nuklear",
        {public = true}
    )

    if is_plat("windows") then
        set_toolchains("msvc")

        add_defines("NOMINMAX", "WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS")
    end

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
           "-s FULL_ES3=1",
           "-s MIN_WEBGL_VERSION=2",
           "-s MAX_WEBGL_VERSION=2",
           "-s ASSERTIONS=1",
           "-s FETCH=1",
           "-s USE_SDL=3",
           "-s USE_SDL_IMAGE=2",
           "-s USE_SDL_TTF=2",
           "-s USE_FREETYPE=1",
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
