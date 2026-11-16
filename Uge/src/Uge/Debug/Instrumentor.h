/**
 * @file Instrumentor.h
 * @brief Chrome-tracing profiler and the `UG_PROFILE_*` macros.
 * @ingroup group_debug
 *
 * Every scope timing goes to Uge::FrameProfiler. Uge::Instrumentor also writes it to a
 * JSON trace while a session is open; open the trace in `chrome://tracing` or
 * https://ui.perfetto.dev.
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
     * @brief One timing, as written to the trace.
     * @ingroup group_debug
     */
    struct ProfileResult
    {
        std::string Name; ///< Scope label.
        /** @brief Start and end, in microseconds. */
        long long Start, End; ///< Start and end, in microseconds.
        uint32_t ThreadID; ///< Hash of the thread's ID.
    };

    /**
     * @brief A profiling session, written to one file.
     * @ingroup group_debug
     */
    struct InstrumentationSession
    {
        std::string Name; ///< Session name.
    };

    /**
     * @brief Writes scope timings to a Chrome-tracing JSON file while a session is open.
     * @ingroup group_debug
     *
     * Thread-safe.
     */
    class Instrumentor
    {
    private:
        InstrumentationSession* m_CurrentSession;
        std::ofstream m_OutputStream;
        int m_ProfileCount;
        std::mutex m_Mutex;
    public:
        /** @brief A profiler with no session open. */
        Instrumentor()
            : m_CurrentSession(nullptr), m_ProfileCount(0)
        {
        }

        /**
         * @brief Opens a trace file and starts a session. Does nothing while one is open.
         * @param name Session name.
         * @param filepath The JSON trace file; overwritten if it exists.
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

        /** @brief Finishes the trace file and ends the session. */
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
         * @brief Whether a trace is being written.
         * @return `true` between BeginSession() and EndSession().
         */
        bool IsSessionActive()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_CurrentSession != nullptr;
        }

        /**
         * @brief Adds a timing to the trace. Does nothing without an open session.
         * @param result The timing.
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

            // Not flushed per event, which would cost more than the scopes being measured.
            // WriteFooter() flushes.
        }

        /**
         * @brief Writes the start of the trace document.
         * @note The caller must hold the lock.
         */
        void WriteHeader()
        {
            m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
        }

        /**
         * @brief Writes the end of the trace document and flushes the file.
         * @note The caller must hold the lock.
         */
        void WriteFooter()
        {
            m_OutputStream << "]}";
            m_OutputStream.flush();
        }

        /**
         * @brief The profiler.
         * @return The instance.
         */
        static Instrumentor& Get()
        {
            static Instrumentor* instance = new Instrumentor();
            return *instance;
        }
    };

    /**
     * @brief Times a scope and records the duration when destroyed.
     * @ingroup group_debug
     */
    class InstrumentationTimer
    {
    public:
        /**
         * @brief Starts timing.
         * @param name Scope label: a string literal or `__FUNCSIG__`.
         */
        InstrumentationTimer(const char* name)
            : m_Name(name), m_Stopped(false)
        {
            m_StartTimepoint = std::chrono::high_resolution_clock::now();
        }

        /** @brief Stops the timer unless Stop() already did. */
        ~InstrumentationTimer()
        {
            if (!m_Stopped)
                Stop();
        }

        /** @brief Ends the measurement and records it. The destructor then does nothing. */
        void Stop()
        {
            auto endTimepoint = std::chrono::high_resolution_clock::now();

            long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

            FrameProfiler::Submit(m_Name, (end - start) / 1000.0f);

            // Written to the trace only while a session is open.
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
 * @brief `1` turns profiling on. At `0`, the default, every profiling macro is empty.
 * @ingroup group_debug
 *
 * Premake defines it as `1` in Debug and Release.
 */
#ifndef UG_PROFILE
	#define UG_PROFILE 0
#endif

/**
 * @def UG_PROFILE_SCOPE_TIMER_NAME
 * @brief A timer variable name ending in @p line.
 * @param line Line number.
 * @ingroup group_debug
 */
#define UG_PROFILE_SCOPE_TIMER_NAME(line) ugProfileTimer##line

/**
 * @def UG_PROFILE_SCOPE_TIMER_NAME_EXPAND
 * @brief Expands @p line before pasting it.
 * @param line Line number.
 * @ingroup group_debug
 */
#define UG_PROFILE_SCOPE_TIMER_NAME_EXPAND(line) UG_PROFILE_SCOPE_TIMER_NAME(line)

/**
 * @def UG_PROFILE_BEGIN_SESSION
 * @brief Opens a trace session.
 * @param name Session name.
 * @param filepath Trace file.
 * @ingroup group_debug
 */

/**
 * @def UG_PROFILE_END_SESSION
 * @brief Closes the trace session.
 * @ingroup group_debug
 */

/**
 * @def UG_PROFILE_SCOPE
 * @brief Times the rest of the enclosing scope.
 * @param name Label, a string literal.
 * @ingroup group_debug
 */

/**
 * @def UG_PROFILE_FUNCTION
 * @brief Times the enclosing function, labelled with its signature.
 * @ingroup group_debug
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


