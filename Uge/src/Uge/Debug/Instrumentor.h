/**
 * @file Instrumentor.h
 * @brief Chrome-tracing profiler and the `UG_PROFILE_*` macros.
 * @ingroup group_debug
 *
 * Scope timings feed two consumers. Uge::FrameProfiler always receives them and keeps a
 * live per-frame aggregate for the editor's Debug panel. Uge::Instrumentor additionally
 * writes them as a JSON trace — but only while a session is open, so tracing costs
 * nothing until it is deliberately started.
 *
 * The trace can be opened in `chrome://tracing` or at https://ui.perfetto.dev.
 *
 * @note `UG_PROFILE` is defined to `1` by premake in the Debug and Release
 * configurations and to `0` in Dist, where every macro compiles to nothing.
 */


#pragma once

#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <mutex>

#include <thread>

#include "Uge/Debug/FrameProfiler.h"

namespace Uge
{

    /**
     * @brief One completed timing, as written to the trace file.
     * @ingroup group_debug
     */
    struct ProfileResult
    {
        std::string Name; ///< Label of the timed scope.
        /** @brief Start and end timestamps, in microseconds. */
        long long Start, End; ///< Start and end timestamps, in microseconds.
        uint32_t ThreadID; ///< Hash of the thread that produced the timing.
    };

    /**
     * @brief A named profiling session, corresponding to one output file.
     * @ingroup group_debug
     */
    struct InstrumentationSession
    {
        std::string Name; ///< Session name.
    };

    /**
     * @brief Singleton that writes timing results to a Chrome-tracing JSON file.
     * @ingroup group_debug
     *
     * Use the macros rather than this class directly. `main` in EntryPoint.h opens a
     * session per phase, producing `UgeProfile-Startup.json`, `UgeProfile-Runtime.json`
     * and `UgeProfile-Shutdown.json`.
     *
     * @code
     * void Scene::OnUpdateRuntime(Timestep ts)
     * {
     *     UG_PROFILE_FUNCTION();          // times the whole function
     *     {
     *         UG_PROFILE_SCOPE("Physics");
     *         StepPhysics(ts);
     *     }
     * }
     * @endcode
     *
     * Thread-safe: every entry point takes an internal lock, so scopes timed on asset
     * loading threads interleave correctly with the main thread's.
     */
    class Instrumentor
    {
    private:
        InstrumentationSession* m_CurrentSession;
        std::ofstream m_OutputStream;
        int m_ProfileCount;
        std::mutex m_Mutex;
    public:
        /** @brief Constructs the profiler with no active session. */
        Instrumentor()
            : m_CurrentSession(nullptr), m_ProfileCount(0)
        {
        }

        /**
         * @brief Opens a trace file and starts a session.
         * @param name Session name.
         * @param filepath Output path for the JSON trace; overwritten if it exists.
         * @note Call EndSession() before starting another; sessions do not nest.
         */
        void BeginSession(const std::string& name, const std::string& filepath = "results.json")
        {
            std::lock_guard<std::mutex> lock(m_Mutex);

            if (m_CurrentSession)
                return;

            m_OutputStream.open(filepath);
            WriteHeader();
            m_CurrentSession = new InstrumentationSession{ name };
        }

        /** @brief Closes the trace file, completing the JSON document. */
        void EndSession()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);

            if (!m_CurrentSession)
                return;

            WriteFooter();
            m_OutputStream.close();
            delete m_CurrentSession;
            m_CurrentSession = nullptr;
            m_ProfileCount = 0;
        }

        /**
         * @brief Reports whether a trace is currently being written.
         * @return `true` between BeginSession() and EndSession().
         *
         * Uge::InstrumentationTimer checks this so that, with no session open, timing a
         * scope costs only the FrameProfiler aggregation.
         */
        bool IsSessionActive()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_CurrentSession != nullptr;
        }

        /**
         * @brief Appends one timing entry to the open trace.
         * @param result The timing to record.
         * @note Does nothing when no session is open.
         */
        void WriteProfile(const ProfileResult& result)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);

            if (!m_CurrentSession)
                return;

            if (m_ProfileCount++ > 0)
                m_OutputStream << ",";

            std::string name = result.Name;
            std::replace(name.begin(), name.end(), '"', '\'');

            m_OutputStream << "{";
            m_OutputStream << "\"cat\":\"function\",";
            m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
            m_OutputStream << "\"name\":\"" << name << "\",";
            m_OutputStream << "\"ph\":\"X\",";
            m_OutputStream << "\"pid\":0,";
            m_OutputStream << "\"tid\":" << result.ThreadID << ",";
            m_OutputStream << "\"ts\":" << result.Start;
            m_OutputStream << "}";

            // Deliberately not flushed: a flush per event would cost more than the scopes
            // being measured. WriteFooter() flushes once, when the session closes.
        }

        /**
         * @brief Writes the opening JSON object of the trace document.
         * @note Caller must hold the internal lock.
         */
        void WriteHeader()
        {
            m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
        }

        /**
         * @brief Writes the closing JSON of the trace document and flushes it to disk.
         * @note Caller must hold the internal lock.
         */
        void WriteFooter()
        {
            m_OutputStream << "]}";
            m_OutputStream.flush();
        }

        /**
         * @brief Returns the singleton profiler.
         * @return Reference to the process-wide instance.
         */
        static Instrumentor& Get()
        {
            static Instrumentor instance;
            return instance;
        }
    };

    /**
     * @brief RAII scope timer that reports its duration on destruction.
     * @ingroup group_debug
     *
     * Created by `UG_PROFILE_SCOPE` and `UG_PROFILE_FUNCTION` rather than by hand.
     */
    class InstrumentationTimer
    {
    public:
        /**
         * @brief Starts timing.
         * @param name Label for this scope; must outlive the timer, so use a literal.
         */
        InstrumentationTimer(const char* name)
            : m_Name(name), m_Stopped(false)
        {
            m_StartTimepoint = std::chrono::high_resolution_clock::now();
        }

        /** @brief Stops the timer if Stop() has not already been called. */
        ~InstrumentationTimer()
        {
            if (!m_Stopped)
                Stop();
        }

        /**
         * @brief Ends the measurement and records it.
         *
         * Safe to call early to end a measurement before the scope closes; the destructor
         * will then do nothing.
         */
        void Stop()
        {
            auto endTimepoint = std::chrono::high_resolution_clock::now();

            long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

            // Always aggregated, so the editor's Debug panel has live numbers.
            FrameProfiler::Submit(m_Name, (end - start) / 1000.0f);

            // Only serialised when a trace is actually being captured.
            if (Instrumentor::Get().IsSessionActive())
            {
                uint32_t threadID = static_cast<uint32_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
                Instrumentor::Get().WriteProfile({ m_Name, start, end, threadID });
            }

            m_Stopped = true;
        }
    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
        bool m_Stopped;
    };


}



/**
 * @def UG_PROFILE
 * @brief Master switch: `1` enables profiling, `0` compiles every macro to nothing.
 * @ingroup group_debug
 *
 * Set per configuration by premake — `1` in Debug and Release, absent in Dist, where the
 * fallback below turns profiling off.
 */
#ifndef UG_PROFILE
	#define UG_PROFILE 0
#endif

/**
 * @def UG_PROFILE_SCOPE_TIMER_NAME
 * @brief Builds a unique timer variable name from a line number.
 * @param line The line number to append.
 * @ingroup group_debug
 * @note Implementation detail of #UG_PROFILE_SCOPE.
 */
#define UG_PROFILE_SCOPE_TIMER_NAME(line) ugProfileTimer##line

/**
 * @def UG_PROFILE_SCOPE_TIMER_NAME_EXPAND
 * @brief Expands @p line before pasting it, which `##` would otherwise prevent.
 * @param line The line number to append.
 * @ingroup group_debug
 * @note Implementation detail of #UG_PROFILE_SCOPE.
 */
#define UG_PROFILE_SCOPE_TIMER_NAME_EXPAND(line) UG_PROFILE_SCOPE_TIMER_NAME(line)

/**
 * @def UG_PROFILE_BEGIN_SESSION
 * @brief Opens a profiling session writing to @p filepath.
 * @param name Session name.
 * @param filepath Output trace path.
 * @ingroup group_debug
 * @note Expands to nothing unless #UG_PROFILE is `1`.
 */

/**
 * @def UG_PROFILE_END_SESSION
 * @brief Closes the current profiling session.
 * @ingroup group_debug
 * @note Expands to nothing unless #UG_PROFILE is `1`.
 */

/**
 * @def UG_PROFILE_SCOPE
 * @brief Times the enclosing scope under the label @p name.
 * @param name Literal label shown in the trace viewer and the editor's Debug panel.
 * @ingroup group_debug
 *
 * The timer variable is named after the source line, so several scopes may be timed
 * within one function without colliding.
 *
 * @note Expands to nothing unless #UG_PROFILE is `1`.
 */

/**
 * @def UG_PROFILE_FUNCTION
 * @brief Times the enclosing function, labelling it with its full signature.
 * @ingroup group_debug
 * @note Expands to nothing unless #UG_PROFILE is `1`.
 */

#if UG_PROFILE
    #define UG_PROFILE_BEGIN_SESSION(name, filepath) ::Uge::Instrumentor::Get().BeginSession(name, filepath)
    #define UG_PROFILE_END_SESSION() ::Uge::Instrumentor::Get().EndSession()
    #define UG_PROFILE_SCOPE(name) ::Uge::InstrumentationTimer UG_PROFILE_SCOPE_TIMER_NAME_EXPAND(__LINE__)(name);
    #define UG_PROFILE_FUNCTION() UG_PROFILE_SCOPE(__FUNCSIG__)
#else
    #define UG_PROFILE_BEGIN_SESSION(name, filepath)
    #define UG_PROFILE_END_SESSION()
    #define UG_PROFILE_SCOPE(name)
    #define UG_PROFILE_FUNCTION()
#endif


