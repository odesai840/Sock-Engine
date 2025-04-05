project "ImGui"
	kind "StaticLib"
	language "C++"

	targetdir ("Binaries/" .. OutputDir .. "/%{prj.name}")
	objdir ("Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

	includedirs {
		"../../Include"
	}

	files { "**.h", "**.cpp" }

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"