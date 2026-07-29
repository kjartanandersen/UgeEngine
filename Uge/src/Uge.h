/**
 * @file Uge.h
 * @brief Umbrella header for client applications; the only engine header they include.
 *
 * Editor and game code include this one header and get the whole public API. Engine
 * code must **not** include it: internal translation units include the specific
 * headers they need, which keeps compile times down and dependencies explicit.
 *
 * @code
 * #include <Uge.h>
 * #include <Uge/Core/EntryPoint.h>   // in exactly one translation unit
 * @endcode
 *
 * @note This header does not declare `main`; include Uge/Core/EntryPoint.h for that.
 */

#pragma once

// For use by Uge applications

#include "Uge/Core/Application.h"
#include "Uge/Core/Layer.h"
#include "Uge/Core/Log.h"

#include "Uge/Core/Timestep.h"

#include "Uge/Core/Input.h"
#include "Uge/Core/KeyCodes.h"
#include "Uge/Core/MouseButtonCodes.h"
#include "Uge/Renderer/OrthographicCameraController.h"
#include "Uge/Renderer/PerspectiveCameraController.h"

#include "Uge/ImGui/ImGuiLayer.h"

#include "Uge/Scene/Scene.h"
#include "Uge/Scene/Entity.h"
#include "Uge/Scene/ScriptableEntity.h"
#include "Uge/Scene/Components.h"
#include "Uge/Scene/SceneSerializer.h"

#include "Uge/Project/Project.h"

// ************* Renderer *****************
#include "Uge/Renderer/Renderer.h"
#include "Uge/Renderer/Renderer2D.h"
#include "Uge/Renderer/Renderer3D.h"
#include "Uge/Renderer/RenderCommand.h"

#include "Uge/Renderer/Buffer.h"
#include "Uge/Renderer/Shader.h"
#include "Uge/Renderer/Framebuffer.h"
#include "Uge/Renderer/Texture.h"
#include "Uge/Renderer/SubTexture2D.h"
#include "Uge/Renderer/VertexArray.h"
#include "Uge/Renderer/Mesh.h"
#include "Uge/Renderer/Model.h"
#include "Uge/Renderer/EditorCamera.h"

#include "Uge/Renderer/OrthographicCamera.h"
#include "Uge/Renderer/PerspectiveCamera.h"


// ****************************************

#include "Uge/Utils/PlatformUtils.h"

// Math
#include "Uge/Math/Math.h"

