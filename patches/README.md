# Patches to third-party submodules

Local fixes to upstream dependencies that aren't committed in the submodule itself.

## assimp-fbx-rotation-loop.patch

- **Applies to:** `Uge/thirdparty/assimp`, written against submodule commit `929bff1830755719edb5cf9f0fc6adc867ec21e5`
- **File:** `code/AssetLib/FBX/FBXConverter.cpp`, `FBXConverter::GetRotationKeyframeList`
- **Fixes:** an infinite loop while resampling rotation keyframes. When two keys are too far apart to subdivide at tick resolution, are out of time order, or share a time stamp (which makes `step` NaN), the computed `step` is not positive. The loop then keeps appending the same key forever, and importing that FBX hangs the editor. The fix breaks out of the loop in that case.

**Applied automatically:** every premake run (`scripts\Win-GenProjects.bat`, `scripts/Setup.py`) applies it if it isn't applied yet. If it no longer applies — after updating the assimp submodule, say — premake stops with an error, and you update the patch or delete it. To do it by hand, from the repository root:

```bash
git -C Uge/thirdparty/assimp apply --check ../../../patches/assimp-fbx-rotation-loop.patch   # dry run
git -C Uge/thirdparty/assimp apply ../../../patches/assimp-fbx-rotation-loop.patch
```

To change the fix, edit `FBXConverter.cpp` in the submodule, then regenerate the patch: `git -C Uge/thirdparty/assimp diff > patches/assimp-fbx-rotation-loop.patch`.

It's a genuine assimp bug; once it's fixed upstream and the submodule is updated past the fix, delete the patch.
