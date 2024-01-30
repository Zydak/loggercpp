workspace "LoggerCpp"
    configurations { "Debug", "Release", "Distribution" }
    platforms { "Windows" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
project "logger"
	architecture "x86_64"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
    defines{ "HOST_LOGGER", "CLIENT_LOGGER", "CONSOLE" }

	files 
    {
        "src/**.cpp",
        "src/**.h",
    }

    links
    {
        "Ws2_32.lib",
        "taskschd.lib",
    }
	
	filter "platforms:Windows"
        system "Windows"
        defines { "WIN" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "speed"

    filter "configurations:Distribution"
		defines "DISTRIBUTION"
		runtime "Release"
        optimize "speed"