#include <ugpch.h>
#include "FrameProfiler.h"

#include <mutex>

namespace Uge
{

	namespace
	{
		// Keyed by the scope label. `UG_PROFILE_SCOPE` always passes a literal, so the
		// pointer would be stable, but hashing the text keeps identical labels from
		// different translation units on one row — which is what a reader expects.
		struct ProfilerState
		{
			std::mutex Mutex;
			std::unordered_map<std::string, ProfileEntry> Accumulating;
			std::vector<ProfileEntry> LastFrame;

			std::vector<float> FrameTimes;
			size_t FrameTimeHead = 0;
			float LastFrameMs = 0.0f;
		};

		/**
		* @brief Returns the process-wide profiler state.
		*
		* Deliberately leaked. Scope timers still fire from destructors that run after
		* `main` returns — a static `Ref<Font>` in the editor keeps an `OpenGLTexture2D`
		* alive, and `~OpenGLTexture2D` is profiled. A namespace-scope object would
		* already have been destroyed by then, so Submit() would touch freed memory.
		*/
		ProfilerState& State()
		{

			static ProfilerState* state = new ProfilerState();
			return *state;

		}

		/**
		 * @brief Reduces a `__FUNCSIG__` to a readable qualified name.
		 * @param name The raw scope label.
		 * @return `Uge::EditorLayer::OnUpdate` rather than
		 *         `void __cdecl Uge::EditorLayer::OnUpdate(class Uge::Timestep)`.
		 *
		 * `UG_PROFILE_FUNCTION` labels its scope with the full signature, which is right
		 * for a trace viewer but far too wide for a table column. Literal labels passed to
		 * `UG_PROFILE_SCOPE` contain no parenthesis and are returned unchanged.
		 */
		std::string ShortenScopeName(const char* name)
		{
			const std::string full = name;

			const size_t paren = full.find('(');
			if (paren == std::string::npos)
			{
				return full;
			}

			const size_t space = full.rfind(' ', paren);
			if (space == std::string::npos)
			{
				return full.substr(0, paren);
			}

			return full.substr(space + 1, paren - space - 1);
		}
	}

	void FrameProfiler::BeginFrame(float frameMs)
	{
		ProfilerState& state = State();
		std::lock_guard<std::mutex> lock(state.Mutex);

		state.LastFrame.clear();
		state.LastFrame.reserve(state.Accumulating.size());
		for (auto& [name, entry] : state.Accumulating)
		{
			state.LastFrame.push_back(entry);
		}
		state.Accumulating.clear();

		std::sort(state.LastFrame.begin(), state.LastFrame.end(),
			[](const ProfileEntry& lhs, const ProfileEntry& rhs) { return lhs.TotalMs > rhs.TotalMs; });

		state.LastFrameMs = frameMs;

		if (state.FrameTimes.size() < s_historySize)
		{
			state.FrameTimes.push_back(frameMs);
		}
		else
		{
			state.FrameTimes[state.FrameTimeHead] = frameMs;
			state.FrameTimeHead = (state.FrameTimeHead + 1) % s_historySize;
		}
	}

	void FrameProfiler::Submit(const char* name, float milliseconds)
	{
		if (!name)
		{
			return;
		}

#ifdef UG_DEBUG
		if (strcmp(name, "__cdecl Uge::OpenGLTexture2D::~OpenGLTexture2D(void)") == 0)
		{
			int a = 0;
		}

#endif
		ProfilerState& state = State();

		std::lock_guard<std::mutex> lock(state.Mutex);

		ProfileEntry& entry = state.Accumulating[name];
		if (entry.Calls == 0)
		{
			// Only on the frame's first call for this scope, so the string work does not
			// repeat for a scope entered hundreds of times.
			entry.Name = ShortenScopeName(name);
		}
		entry.TotalMs += milliseconds;
		entry.Calls++;
	}

	std::vector<ProfileEntry> FrameProfiler::GetLastFrame()
	{
		ProfilerState& state = State();

		std::lock_guard<std::mutex> lock(state.Mutex);
		return state.LastFrame;
	}

	std::vector<float> FrameProfiler::GetFrameTimeHistory()
	{
		ProfilerState& state = State();

		std::lock_guard<std::mutex> lock(state.Mutex);

		// Unroll the ring so the caller always gets oldest-first, ready to plot.
		std::vector<float> history;
		history.reserve(state.FrameTimes.size());
		for (size_t i = 0; i < state.FrameTimes.size(); i++)
		{
			history.push_back(state.FrameTimes[(state.FrameTimeHead + i) % state.FrameTimes.size()]);
		}

		return history;
	}

	float FrameProfiler::GetAverageFrameMs()
	{
		ProfilerState& state = State();

		std::lock_guard<std::mutex> lock(state.Mutex);

		if (state.FrameTimes.empty())
		{
			return 0.0f;
		}

		float total = 0.0f;
		for (float ms : state.FrameTimes)
		{
			total += ms;
		}

		return total / static_cast<float>(state.FrameTimes.size());
	}

	float FrameProfiler::GetLastFrameMs()
	{
		ProfilerState& state = State();

		std::lock_guard<std::mutex> lock(state.Mutex);
		return state.LastFrameMs;
	}

	void FrameProfiler::Reset()
	{
		ProfilerState& state = State();

		std::lock_guard<std::mutex> lock(state.Mutex);

		state.Accumulating.clear();
		state.LastFrame.clear();
		state.FrameTimes.clear();
		state.FrameTimeHead = 0;
		state.LastFrameMs = 0.0f;
	}

}
