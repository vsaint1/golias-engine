add_rules("mode.debug", "mode.release")
set_languages("c99")

target("mini_audio")
    set_kind("static")

    add_includedirs("include", {public = true})

    add_files("src/stb_vorbis.c")

    if is_plat("wasm") then
        add_cxflags("-std=gnu99")
    end

    if is_plat("iphoneos") then
        add_frameworks("AudioToolbox", "AVFoundation", "CoreFoundation", "Foundation")
    elseif is_plat("macosx") then
        add_defines("MA_NO_RUNTIME_LINKING")
        add_frameworks("AudioToolbox", "CoreAudio", "AudioUnit", "AVFoundation", "CoreFoundation", "Foundation")
    end

    if is_plat("linux", "bsd") then
        add_syslinks("pthread")
    end


    if is_plat("macosx", "iphoneos") then
        print("Building mini_audio with Objective-C")
        add_files("src/miniaudio.m")
    else
        add_files("src/miniaudio.c")
    end

