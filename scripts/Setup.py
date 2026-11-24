import os
import subprocess
import sys

import CheckPython

# Install the packages in requirements.txt.
CheckPython.ValidatePackages()

import procVulkan

# Run from the repo root, which the premake path and procVulkan's paths are relative to.
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

if not procVulkan.CheckVulkanSDK():
    if procVulkan.VULKAN_SDK is None:
        # Dependencies.lua takes shaderc and SPIRV-Cross from the SDK.
        print("The Vulkan SDK is required. Install it, then re-run this script.")
        sys.exit(1)
    print("Continuing with the Vulkan SDK that is installed.")

# if (not procVulkan.CheckVulkanSDKDebugLibs()):
#     print("Vulkan SDK debug libs not found.")

print("Running premake...")
# os.path.join, as Windows can't start a program from a relative path with forward slashes.
result = subprocess.call([os.path.join("thirdparty", "premake", "bin", "premake5.exe"), "vs2026"])
if result != 0:
    print(f"premake failed with exit code {result}.")
sys.exit(result)