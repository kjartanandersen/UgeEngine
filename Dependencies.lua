-- Uge Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include Directories relative to root folder (Solution Directory)
IncludeDir = {}
IncludeDir["GLFW"]   	    = "%{wks.location}/Uge/thirdparty/GLFW/include"
IncludeDir["GLAD"]   	    = "%{wks.location}/Uge/thirdparty/GLAD/include"
IncludeDir["IMGUI"]  	    = "%{wks.location}/Uge/thirdparty/imgui"
IncludeDir["GLM"]    	    = "%{wks.location}/Uge/thirdparty/glm"
IncludeDir["STBI"]   	    = "%{wks.location}/Uge/thirdparty/stb_image"
IncludeDir["ENTT"]   	    = "%{wks.location}/Uge/thirdparty/entt/include"
IncludeDir["ASSIMP"]        = "%{wks.location}/Uge/thirdparty/assimp/include"
IncludeDir["YAMLCPP"]	    = "%{wks.location}/Uge/thirdparty/yaml-cpp/include"
IncludeDir["IMGUIZMO"]      = "%{wks.location}/Uge/thirdparty/ImGuizmo"
IncludeDir["SHADERC"]       = "%{wks.location}/Uge/thirdparty/shaderc/include"
IncludeDir["SPIRVCROSS"]    = "%{wks.location}/Uge/thirdparty/SPIRV-Cross"
IncludeDir["VULKANSDK"]     = "%{VULKAN_SDK}/Include"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"
