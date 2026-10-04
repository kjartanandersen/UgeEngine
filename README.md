# Uge Engine

A Windows-only C++17 game engine built as a static library, with an editor (`Uge-Editor`) and a
minimal client app (`Sandbox`) on top of it. Rendering is OpenGL 4.5, scenes are ECS-based (EnTT),
rigid-body physics is Jolt, and gameplay can be scripted in C# through an embedded Mono runtime.

## Features

- **Editor** — viewport with ImGuizmo transform gizmos, scene hierarchy and property panels, content
  browser, console, renderer settings and debug panels, and play/stop mode that runs on a copy of the
  edited scene.
- **2D rendering** — batched quads and sprites, MSDF text, and a separate line batch used by the
  editor overlays. Vertices carry an entity ID so the viewport supports pixel-accurate mouse picking.
- **3D rendering** — model/mesh rendering with materials and texture maps, cubemap environments,
  bloom and post-processing, plus standalone cube primitives.
- **Shaders** — GLSL compiled to SPIR-V at runtime via shaderc/SPIRV-Cross, cached on disk.
- **Scenes / ECS** — `Scene` wraps an `entt::registry`; entities are handles carrying a `UUID` and a
  tag. Scenes serialize to YAML (`.uge`).
- **Assets** — handle-based asset manager (textures, meshes, materials, environments, scenes) driven
  by a per-project registry.
- **Physics** — Jolt-backed rigid bodies with box, sphere, capsule and mesh colliders, a fixed
  timestep accumulator, buffered contact events, and collider wireframe debug rendering in the editor.
- **Scripting** — C# scripts via Mono, with the managed side living in `Uge-ScriptCore`.
- **Projects** — a project is a YAML `.ugproj` file describing the asset directory, start scene and
  script module.

## Requirements

- Windows 10/11 with Visual Studio 2026 (the generator targets `vs2026`; `msbuild` is used to build)
- Git (the third-party libraries are submodules)
- Python 3 — only needed for `scripts/Setup.py`
- **Vulkan SDK**, with the `VULKAN_SDK` environment variable set. The engine links shaderc and
  SPIRV-Cross from it for runtime shader compilation; without it the build will not configure.
- Optional: Doxygen and Graphviz, for building the API documentation

## Getting started

```bash
# 1. Clone with submodules
git clone --recurse-submodules <repo-url>
cd Uge
# (already cloned?)  git submodule update --init --recursive

# 2. Generate the Visual Studio solution
scripts\Win-GenProjects.bat
#   or, to validate the Vulkan SDK first:
python scripts/Setup.py

# 3. Build
msbuild Uge.sln /p:Configuration=Debug /p:Platform=x64 /m
```

Then open `Uge.sln` and run — `Uge-Editor` is the startup project. It runs with `UgeEditor/` as its
working directory and opens `UgeEditor/SandboxProject/Sandbox.ugproj`.

Binaries land in `bin/<Config>-windows-x86_64/<Project>/`.

### Configurations

| Configuration | Define       | Notes                                        |
| ------------- | ------------ | -------------------------------------------- |
| `Debug`       | `UG_DEBUG`   | Asserts and verifies on, full logging         |
| `Release`     | `UG_RELEASE` | Optimized, asserts off, verifies on           |
| `Dist`        | `UG_DIST`    | Shipping build, asserts and verifies compiled out |

### Regenerating projects

The `.sln`/`.vcxproj` files are **generated artifacts** — never edit them by hand. Each project picks
up its sources through `src/**.h` / `src/**.cpp` globs in its `premake5.lua`, so re-run
`scripts\Win-GenProjects.bat` after adding, renaming or deleting a file.

## Repository layout

```
Uge/               the engine static library
  src/Uge/         Asset, Core, Debug, Events, ImGui, Math, Physics,
                   Project, Renderer, Scene, Scripting, UI, Utils
  src/Platform/    platform/API backends (Windows, OpenGL)
  thirdparty/      submodules: GLFW, GLAD, ImGui, ImGuizmo, glm, EnTT,
                   assimp, yaml-cpp, spdlog, JoltPhysics, msdf-atlas-gen,
                   stb_image, mono, googletest
UgeEditor/         the editor application (startup project)
  SandboxProject/  the sample project the editor opens
  assets/          editor shaders, fonts and the SPIR-V shader cache
Sandbox/           a minimal client app built on the engine
Uge-ScriptCore/    the C# scripting core (.NET Framework 4.7.2)
Uge-Tests/         GoogleTest runner for the engine and the editor
scripts/           project generation, build and documentation scripts
docs/              Doxyfile and generated API documentation
```

## Tests

`Uge-Tests` is a GoogleTest runner covering both the engine and the editor.

```bash
msbuild Uge.sln /t:Tests\Uge-Tests /p:Configuration=Debug /p:Platform=x64 /m
bin/Debug-windows-x86_64/Uge-Tests/Uge-Tests.exe
```

Run it **from the repository root** — that is what the project's debug directory is set to, and some
fixtures resolve asset paths relative to the working directory.

Because there is no window or GPU in the test process, the suite ships fixtures for headless ImGui
frames, a temporary on-disk project, a GPU-less texture, and physics init/shutdown. Anything that
touches OpenGL (texture, mesh and environment import) is out of reach; `.uge` scenes, physics and
pure-math code are not.

## Documentation

- **API reference** — `scripts\BuildDocs.bat` runs Doxygen and writes `docs/html/index.html`;
  warnings go to `docs/doxygen-warnings.log`. The build is warning-free and should stay that way, so
  document new public types, members and parameters on their declaration in the header.
- **Contributor notes** — `CLAUDE.md` describes the architecture, conventions and the sharp edges of
  each subsystem in more detail.

## Conventions

- Every `.cpp` under `Uge/src/` starts with `#include <ugpch.h>`, the precompiled header.
- Use `Uge::Ref<T>` / `Uge::CreateRef` (shared) and `Uge::Scope<T>` / `Uge::CreateScope` (unique)
  rather than the raw STL names.
- Members are `m_camelCase`, statics `s_camelCase`, and everything lives in `namespace Uge`.
- Logging and asserts: `UG_CORE_*` inside the engine, `UG_*` in client apps.
- `Uge.h` is the single umbrella header for client apps; engine-internal code includes specific
  headers instead.

## Acknowledgements

Uge follows the architecture of [Hazel](https://github.com/TheCherno/Hazel) and the Game Engine
series by TheCherno, and is built on the open-source libraries listed above.
