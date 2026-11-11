-- Uge Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include Directories relative to root folder (Solution Directory)
IncludeDir = {}
IncludeDir["GLFW"]   				= "%{wks.location}/Uge/thirdparty/GLFW/include"
IncludeDir["GLAD"]   				= "%{wks.location}/Uge/thirdparty/GLAD/include"
IncludeDir["IMGUI"]  				= "%{wks.location}/Uge/thirdparty/imgui"
IncludeDir["GLM"]    				= "%{wks.location}/Uge/thirdparty/glm"
IncludeDir["STBI"]   				= "%{wks.location}/Uge/thirdparty/stb_image"
IncludeDir["ENTT"]   				= "%{wks.location}/Uge/thirdparty/entt/include"
IncludeDir["ASSIMP"]				= "%{wks.location}/Uge/thirdparty/assimp/include"
IncludeDir["RAPIDJSON"]				= "%{wks.location}/Uge/thirdparty/assimp/contrib/rapidjson/include"
IncludeDir["YAMLCPP"]				= "%{wks.location}/Uge/thirdparty/yaml-cpp/include"
IncludeDir["JOLT"]					= "%{wks.location}/Uge/thirdparty/JoltPhysics"
IncludeDir["IMGUIZMO"]				= "%{wks.location}/Uge/thirdparty/ImGuizmo"
IncludeDir["MONO"]					= "%{wks.location}/Uge/thirdparty/mono/include"
IncludeDir["FILEWATCH"]    			= "%{wks.location}/Uge/thirdparty/filewatch"
IncludeDir["SHADERC"]				= "%{wks.location}/Uge/thirdparty/shaderc/include"
IncludeDir["SPIRVCROSS"]			= "%{wks.location}/Uge/thirdparty/SPIRV-Cross"
IncludeDir["VULKANSDK"]				= "%{VULKAN_SDK}/Include"
IncludeDir["MSDFGEN"]				= "%{wks.location}/Uge/thirdparty/ch-msdf-atlas-gen/msdfgen"
IncludeDir["MSDF_ATLAS_GEN"]		= "%{wks.location}/Uge/thirdparty/ch-msdf-atlas-gen/msdf-atlas-gen"
IncludeDir["GOOGLETEST"]			= "%{wks.location}/Uge/thirdparty/googletest/googletest/include"
IncludeDir["GOOGLEMOCK"]			= "%{wks.location}/Uge/thirdparty/googletest/googlemock/include"

-- Jolt's compile defines change its ABI. Every project that compiles a file including a
-- Jolt header must use the same set as the Jolt library, or the two disagree on the layout
-- of Jolt's classes. Applied by Uge/thirdparty/JoltPhysics, Uge and Uge-Tests, in every
-- configuration.
--
-- NDEBUG is part of the set: without it, Jolt's headers define JPH_DEBUG, which enables
-- asserts and the members that only exist for them. JPH::RegisterTypes() compares the
-- ABI-relevant defines with the library's and aborts on a mismatch, naming the define.
JoltDefines = {}
JoltDefines["Common"]  = { "JPH_USE_CPU_COMPUTE" }
JoltDefines["Debug"]   = { "JPH_ENABLE_ASSERTS", "JPH_DEBUG_RENDERER", "JPH_PROFILE_ENABLED" }
JoltDefines["Release"] = { "NDEBUG", "JPH_DEBUG_RENDERER", "JPH_PROFILE_ENABLED" }
JoltDefines["Dist"]    = { "NDEBUG" }

-- Library Directories
LibraryDir = {}

LibraryDir["VulkanSDK"]				= "%{VULKAN_SDK}/Lib"
LibraryDir["MONO"] 					= "%{wks.location}/Uge/thirdparty/mono/lib"

-- Libraries
Library = {}

-- Mono is prebuilt in Debug and Release only. Dist links the Release build, which uses the
-- same release C runtime.
Library["MONO_Debug"]				= "%{LibraryDir.MONO}/Debug/libmono-static-sgen.lib"
Library["MONO_Release"]				= "%{LibraryDir.MONO}/Release/libmono-static-sgen.lib"

Library["Vulkan"]					= "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"]				= "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"]			= "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"]		= "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"]	= "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"]		= "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"]			= "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"]		= "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

-- Windows Only Dependencies
Library["WINSOCK"] = "Ws2_32.lib"
Library["WINMM"] = "Winmm.lib"
Library["WINVER"] = "Version.lib"
Library["WINCRYPT"] = "Bcrypt.lib"
