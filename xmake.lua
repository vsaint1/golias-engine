set_project("ember_engine")
set_languages("cxx20")
set_license("MIT")
set_version("0.0.1", {build = "%Y%m%d%H%M"})

add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
add_rules("mode.debug", "mode.release")

-- add_requires("lua v5.4.7", {configs = {shared = true}})
add_requires("sol2 v3.5.0", {configs = {shared = false}})
add_requires("libsdl3_ttf 3.2.2", {configs = {shared = true, harfbuzz = true, plutosvg = true}})
add_requires("libsdl3 3.2.22","libsdl3_image 3.2.0",{configs = {shared = true}})
add_requires("flecs v4.1.1", {configs = {shared = false}})
add_requires("nlohmann_json v3.12.0", {configs = {shared = false}})
add_requires("glm 1.0.1", {configs = {shared = false}})
add_requires("miniaudio 0.11.23","tinyxml2 11.0.0", {configs = {shared = false}})

if not is_plat("wasm") or not is_plat("android")  or not is_plat("iphoneos") then
    add_requires("doctest v2.4.9", {configs = {shared = false}})
end



target("engine")
    set_kind("static") 
    add_files("engine/private/**/*.cpp")
    add_includedirs("engine/public", {public = true}) 
    set_pcheader("stdafx.h","engine/private/stdafx.cpp")
    add_packages("libsdl3", "libsdl3_ttf", "libsdl3_image", "sol2", "flecs", "nlohmann_json","glm", "miniaudio", "tinyxml2",{public = true})

target("client")
    set_kind("binary")
    add_files("client/*.cpp")
    add_deps("engine")
    add_includedirs("engine/public")


    if is_plat("android") then

        set_kind("shared")
        add_syslinks("log", "android","m","dl")
    end 

    if is_plat("wasm") then 
    
        set_basename("index")

        add_ldflags("-s", "ALLOW_MEMORY_GROWTH=1", "FETCH=1", "USE_SDL=3", "USE_SDL_IMAGE=2", "USE_SDL_TTF=2", "USE_FREETYPE=1","--preload-file=res@/res")

       
    end

    if is_plat("windows") then
       -- todo
    end

    if not is_plat("wasm") or  not is_plat("android") then

        after_build(function (target)

            os.cp("res", path.join(target:targetdir(), "res"))

        end)

    end 

if not is_plat("wasm") or not is_plat("android")  or not is_plat("iphoneos") then

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

