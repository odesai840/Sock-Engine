-- premake5.lua
workspace "SockEngine"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Editor"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Dependencies"
   include "Core/ThirdParty/Include/Glad/Build-Glad.lua"
   include "Core/ThirdParty/Include/imgui/Build-ImGui.lua"
group ""

group "Core"
	include "Core/Build-Core.lua"
group ""

group "Editor"
   include "Editor/Build-Editor.lua"
group ""