//
//  timer.h
//  FrameTech
//
//  Created by Antonin on 09/10/2022.
//

#pragma once
#ifndef timer_h
#define timer_h

#include "debug_tools.h"
#include <chrono>
#include <ctime>
#include <optional>
#ifdef WIN32
#include <ctime>
#else // APPLE
#include <sys/time.h>
#endif

/// The current time, in ms
#define NOW_MS std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

namespace ftstd
{
    /// @brief An helper class to play with a Timer / Timer object.
    /// This class might be useful in order to pause / resume the engine drawing
    /// a frame.
    class Timer
    {

    private:
        /// @brief The time, in ms, when the object has been created, or used
        std::optional<uint64_t> m_begin;
        /// @brief Stores if the timer object is being used or not
        bool m_is_running;

    public:
        /// @brief Instanciate a Timer object sets to NOW.
        Timer()
        {
            m_begin = (uint64_t)NOW_MS;
            m_is_running = false;
        }
        /// @brief Pause the CPU until the time limit (in ms) is reached
        /// This function is not thread safe on purpose, in order to let the
        /// developer stops the timer himself / herself, and cancel the
        /// CPU pause.
        /// @param time_limit The time limit to reach (in ms) to unblock the thread.
        void block_until(uint64_t time_limit)
        {
            m_is_running = true;
            while (m_is_running)
            {
                if (NOW_MS >= time_limit)
                    m_is_running = false;
            }
        }
        /// @brief Gets the time limit (in ms) based on the current time added
        /// to a limit (in ms)
        /// @param ms_to_add The ms to add to reach a limit, based on now
        static double get_time_limit(double ms_to_add)
        {
            return (double)NOW_MS + ms_to_add;
        }
        /// @brief Stops the timer
        void stop()
        {
            m_is_running = false;
        };
        uint64_t diff()
        {
            return (uint64_t)NOW_MS - m_begin.value();
        }
        /// @brief Resets the internal settings of the timer
        void reset()
        {
            m_is_running = false;
            m_begin = NOW_MS;
        };
        /// @brief Gets the information if the current timer object is being
        /// used or not.
        /// @return A boolean value: `true` if the time is being used, false otherwise.
        bool is_used()
        {
            return m_is_running;
        }
    };
} // namespace ftstd

#endif // timer_h
