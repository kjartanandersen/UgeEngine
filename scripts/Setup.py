import os
import subprocess
import sys

import CheckPython

# Make sure everything we need is installed
CheckPython.ValidatePackages()

import procVulkan

# Work from the repo root, wherever the script was started from: the premake path below and
# the Vulkan SDK paths in procVulkan are relative to it.
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

if not procVulkan.CheckVulkanSDK():
    if procVulkan.VULKAN_SDK is None:
        # Dependencies.lua takes shaderc and SPIRV-Cross from the SDK, so the projects can't
        # be generated without it.
        print("The Vulkan SDK is required. Install it, then re-run this script.")
        sys.exit(1)
    print("Continuing with the Vulkan SDK that is installed.")

# if (not procVulkan.CheckVulkanSDKDebugLibs()):
#     print("Vulkan SDK debug libs not found.")

print("Running premake...")
# Windows can't start a program from a relative path written with forward slashes, so build it with os.path.join.
result = subprocess.call([os.path.join("thirdparty", "premake", "bin", "premake5.exe"), "vs2026"])
if result != 0:
    print(f"premake failed with exit code {result}.")
sys.exit(result)