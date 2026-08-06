/**
 * @file HeadlessImGui.h
 * @brief GoogleTest fixture that runs ImGui frames without a window or GPU.
 */

#pragma once

#include <imgui.h>

#include <gtest/gtest.h>

#include <utility>

namespace UgeTests
{

	/**
	 * @brief Fixture giving each test a private ImGui context it can draw into.
	 *
	 * The editor's panels are ordinary ImGui code: they need a current context and a frame
	 * in progress, but nothing about them needs pixels on a screen. Standing up a real
	 * window plus an OpenGL context in a test would make the suite dependent on a GPU and
	 * on Uge::Application's lifetime; instead this drives ImGui with no backend attached.
	 *
	 * That still exercises a lot. ImGui asserts on unbalanced `Begin`/`End`, on style and
	 * ID stacks left pushed, and on tables and child windows that are never closed, so a
	 * panel that renders a frame here is a panel whose widget structure is sound.
	 *
	 * What it cannot do is press anything: without a backend feeding input, every widget
	 * reports "not clicked", so a frame drawn here follows the panel's idle path. Behaviour
	 * behind a button press is not covered.
	 */
	class HeadlessImGuiTest : public ::testing::Test
	{
	protected:
		/** @brief Creates the ImGui context and configures it for headless use. */
		void SetUp() override
		{
			m_context = ImGui::CreateContext();
			ImGui::SetCurrentContext(m_context);

			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2(1280.0f, 720.0f);
			io.DeltaTime = 1.0f / 60.0f;

			// Claiming texture support keeps the font atlas unlocked and lets ImGui leave
			// its textures in WantCreate; nothing here ever uploads them.
			io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

			// A test binary should not drop imgui.ini next to itself, nor should one test's
			// window layout leak into the next.
			io.IniFilename = nullptr;
			io.LogFilename = nullptr;
		}

		/** @brief Destroys the context, so tests cannot see each other's ImGui state. */
		void TearDown() override
		{
			ImGui::DestroyContext(m_context);
			m_context = nullptr;
		}

		/**
		 * @brief Runs @p draw inside a complete ImGui frame.
		 * @tparam Fn Callable taking no arguments.
		 * @param draw Drawing code to run between `NewFrame` and `Render`.
		 *
		 * Render() is called rather than EndFrame() so the draw lists are actually built,
		 * which is where clipping and vertex generation would trip over a malformed widget.
		 */
		template<typename Fn>
		void DrawFrame(Fn&& draw)
		{
			ImGui::NewFrame();
			draw();
			ImGui::Render();
		}

		/**
		 * @brief Runs @p draw once per frame for @p frames frames.
		 * @tparam Fn Callable taking no arguments.
		 * @param frames Number of frames to draw.
		 * @param draw Drawing code to run each frame.
		 *
		 * Several things settle only on the second frame — auto-fitted window sizes, table
		 * column widths, and any widget whose state was created during the first one — so
		 * a panel is worth drawing more than once.
		 */
		template<typename Fn>
		void DrawFrames(int frames, Fn&& draw)
		{
			for (int i = 0; i < frames; i++)
			{
				DrawFrame(draw);
			}
		}

	private:
		ImGuiContext* m_context = nullptr;
	};

}
