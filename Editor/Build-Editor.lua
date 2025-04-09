project "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"

    files {
        "Source/**.h",
        "Source/**.cpp"
    }

    includedirs
    {
        "Source",
        "../Core/Source",
        "../Core/ThirdParty/Include",
        "../Core/ThirdParty/Include/Glad/include"
    }

    links
    {
        "Core",
        "Glad",
        "ImGui"
    }

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    postbuildcommands
    {
        "{COPYDIR} %{wks.location}Assets ../Binaries/" .. OutputDir .. "/Assets",
        "{COPYDIR} %{wks.location}Shaders ../Binaries/" .. OutputDir .. "/Shaders",
        "{COPYFILE} %{wks.location}Vendor/Binaries/Assimp/assimp-vc143-mt.dll %{cfg.targetdir}"
    }

    filter "system:windows"
        systemversion "latest"
        defines { "WINDOWS" }

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