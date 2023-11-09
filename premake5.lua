project "tracy"
    kind "StaticLib"
    language "C++"
    warnings "Off"   
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    defines("TRACY_ENABLE")
    files
    {
        "include/TracyClient.cpp",
    }