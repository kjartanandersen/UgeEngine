#include <ugpch.h>
#include "FrameProfiler.h"

#include <mutex>

namespace Uge
{

	namespace
	{
		std::mutex s_mutex;

		// Keyed by the scope label. `UG_PROFILE_SCOPE` always passes a literal, so the
		// pointer would be stable, but hashing the text keeps identical labels from
		// different translation units on one row — which is what a reader expects.
		std::unordered_map<std::string, ProfileEntry> s_accumulating;
		std::vector<ProfileEntry> s_lastFrame;

		std::vector<float> s_frameTimes;
		size_t s_frameTimeHead = 0;
		float s_lastFrameMs = 0.0f;

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
		std::lock_guard<std::mutex> lock(s_mutex);

		s_lastFrame.clear();
		s_lastFrame.reserve(s_accumulating.size());
		for (auto& [name, entry] : s_accumulating)
		{
			s_lastFrame.push_back(entry);
		}
		s_accumulating.clear();

		std::sort(s_lastFrame.begin(), s_lastFrame.end(),
			[](const ProfileEntry& lhs, const ProfileEntry& rhs) { return lhs.TotalMs > rhs.TotalMs; });

		s_lastFrameMs = frameMs;

		if (s_frameTimes.size() < s_historySize)
		{
			s_frameTimes.push_back(frameMs);
		}
		else
		{
			s_frameTimes[s_frameTimeHead] = frameMs;
			s_frameTimeHead = (s_frameTimeHead + 1) % s_historySize;
		}
	}

	void FrameProfiler::Submit(const char* name, float milliseconds)
	{
		std::lock_guard<std::mutex> lock(s_mutex);

		ProfileEntry& entry = s_accumulating[name];
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
		std::lock_guard<std::mutex> lock(s_mutex);
		return s_lastFrame;
	}

	std::vector<float> FrameProfiler::GetFrameTimeHistory()
	{
		std::lock_guard<std::mutex> lock(s_mutex);

		// Unroll the ring so the caller always gets oldest-first, ready to plot.
		std::vector<float> history;
		history.reserve(s_frameTimes.size());
		for (size_t i = 0; i < s_frameTimes.size(); i++)
		{
			history.push_back(s_frameTimes[(s_frameTimeHead + i) % s_frameTimes.size()]);
		}

		return history;
	}

	float FrameProfiler::GetAverageFrameMs()
	{
		std::lock_guard<std::mutex> lock(s_mutex);

		if (s_frameTimes.empty())
		{
			return 0.0f;
		}

		float total = 0.0f;
		for (float ms : s_frameTimes)
		{
			total += ms;
		}

		return total / static_cast<float>(s_frameTimes.size());
	}

	float FrameProfiler::GetLastFrameMs()
	{
		std::lock_guard<std::mutex> lock(s_mutex);
		return s_lastFrameMs;
	}

	void FrameProfiler::Reset()
	{
		std::lock_guard<std::mutex> lock(s_mutex);

		s_accumulating.clear();
		s_lastFrame.clear();
		s_frameTimes.clear();
		s_frameTimeHead = 0;
		s_lastFrameMs = 0.0f;
	}

}
