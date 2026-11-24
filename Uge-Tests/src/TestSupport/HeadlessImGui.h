/**
 * @file HeadlessImGui.h
 * @brief A fixture that runs ImGui frames without a window or GPU.
 */

#pragma once

#include <imgui.h>

#include <gtest/gtest.h>

#include <utility>

namespace UgeTests
{

	/**
	 * @brief Gives each test its own ImGui context, with no backend.
	 *
	 * ImGui asserts on unbalanced `Begin`/`End`, leaked style and ID pushes, and unclosed
	 * tables, so drawing a panel here checks its structure. Nothing can be clicked, so only
	 * the panel's idle path runs.
	 */
	class HeadlessImGuiTest : public ::testing::Test
	{
	protected:
		/** @brief Creates the context and sets it up for headless use. */
		void SetUp() override
		{
			m_context = ImGui::CreateContext();
			ImGui::SetCurrentContext(m_context);

			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2(1280.0f, 720.0f);
			io.DeltaTime = 1.0f / 60.0f;

			// Claiming texture support keeps the font atlas unlocked; its textures are never
			// uploaded.
			io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

			// No imgui.ini, so no layout is saved or carried between tests.
			io.IniFilename = nullptr;
			io.LogFilename = nullptr;
		}

		/** @brief Destroys the context. */
		void TearDown() override
		{
			ImGui::DestroyContext(m_context);
			m_context = nullptr;
		}

		/**
		 * @brief Runs @p draw between `NewFrame` and `Render`, so the draw lists are built.
		 * @tparam Fn Callable taking no arguments.
		 * @param draw The drawing code.
		 */
		template<typename Fn>
		void DrawFrame(Fn&& draw)
		{
			ImGui::NewFrame();
			draw();
			ImGui::Render();
		}

		/**
		 * @brief Runs @p draw in each of @p frames frames. Window sizes and table columns
		 * settle on the second frame.
		 * @tparam Fn Callable taking no arguments.
		 * @param frames Number of frames.
		 * @param draw The drawing code.
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
