import os
import subprocess
import CheckPython

# Make sure everything we need is installed
CheckPython.ValidatePackages()

import procVulkan

# Change from Scripts directory to root
os.chdir('../')

if (not procVulkan.CheckVulkanSDK()):
    print("Vulkan SDK not installed.")
    
# if (not procVulkan.CheckVulkanSDKDebugLibs()):
#     print("Vulkan SDK debug libs not found.")

print("Running premake...")
subprocess.call(["thirdparty/premake/bin/premake5.exe", "vs2026"])
