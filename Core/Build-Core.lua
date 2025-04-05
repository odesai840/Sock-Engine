project "Core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"

    files {
        "Source/**.h",
        "Source/**.cpp",
    }

    includedirs {
        "Source",
        "ThirdParty/Include",
        "ThirdParty/Include/Glad/include"
    }

    libdirs {
        "ThirdParty/Lib",
    }

    links {
        "opengl32.lib",
        "Glad",
        "ImGui",
        "ThirdParty/Lib/glfw3.lib",
        "ThirdParty/Lib/SOIL2.lib",
        "ThirdParty/Lib/assimp-vc143-mt.lib",
    }

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"