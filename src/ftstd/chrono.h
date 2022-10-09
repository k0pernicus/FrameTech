//
//  chrono.h
//  FrameTech
//
//  Created by Antonin on 09/10/2022.
//

#pragma once
#ifndef chrono_h
#define chrono_h

#include "debug_tools.h"
#include <chrono>
#include <ctime>
#include <optional>
#include <sys/time.h>

#define NOW std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

class Chrono
{

private:
    std::optional<uint64_t> m_begin;
    bool m_is_running;

public:
    Chrono()
    {
        m_begin = NOW;
    }
    void block_until(uint64_t time_limit)
    {
        while (true)
        {
            auto now = NOW;
            if (now >= time_limit)
                break;
        }
    }
    static uint64_t get_time_limit(uint64_t ms_to_add)
    {
        auto now = NOW;
        return now + ms_to_add;
    }
    void stop()
    {
        m_is_running = false;
    };
    void reset()
    {
        m_is_running = false;
        m_begin = std::nullopt;
    };
};

#endif // chrono_h
