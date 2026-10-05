# Dependency build scripts

The root `premake5.lua` includes a `premake5.lua` from each dependency under `Uge/thirdparty/`. For the seven below, that file can't be committed in the submodule: the submodules point at the upstream repositories.

**The scripts in this folder are the source of truth. Edit them here.** Every premake run — `scripts\Win-GenProjects.bat`, `scripts/Setup.py`, or `premake5 vs2026` by hand — first copies each one into its submodule (`Uge/thirdparty/<name>/premake5.lua`), which is the copy premake actually executes. Its relative paths keep working because it runs from inside the submodule folder. The submodule copies are generated: after a fresh clone or `git clean` they simply come back on the next run.

| Situation when premake runs | What happens |
| --- | --- |
| The submodule copy is missing | it is copied in (`Restored …`) |
| It matches this folder's copy (ignoring CRLF/LF differences) | nothing |
| It differs | premake **stops with an error** and overwrites nothing |
| The submodule isn't checked out | premake stops: run `git submodule update --init --recursive` |

The error case protects an edit made in the submodule copy by mistake. Move the change into this folder, delete the submodule copy, and run again.

| Dependency | Submodule commit the script was written against |
| --- | --- |
| GLFW | `dbadda26835ec5089ef922e6c290bcf58cf12056` |
| imgui | `9ca7ea00c80c1cb95b31cb3efa6f88cea33edf2a` |
| glm | `8f6213d379a904f5ae910e09a114e066e25faf57` |
| yaml-cpp | `f0329d0efd2e62ceda5ee899c343ee445f6f7bd2` |
| JoltPhysics | `5129257c495a8fe69cd7b7f4d8597a75c2fb7dde` |
| googletest | `4f472747b124e9ddf3434c0e77ea032c799244d5` |
| assimp | `929bff1830755719edb5cf9f0fc6adc867ec21e5` |

When you update one of these submodules, check that its script still matches the new upstream layout (file lists, include paths), and update the table.

assimp's script regenerates the headers CMake would normally create (`include/assimp/config.h`, `include/assimp/revision.h`, `contrib/zlib/zconf.h`). assimp also needs the source fix in `patches/`, which the same premake run applies (see `patches/README.md`).

## Adding another dependency the same way

1. Put its script in `premake/thirdparty/<name>/premake5.lua`.
2. Add `<name>` to `thirdPartyScripts` at the top of the root `premake5.lua`.
3. Add `include "Uge/thirdparty/<name>"` in the `Dependencies` group.
