//
//  inputs.hpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#pragma once
#ifndef _inputs_hpp
#define _inputs_hpp

#include <deque>
#include <mutex>

namespace frametech
{
    namespace engine
    {
        namespace inputs
        {
            /// We consider here that at most two keys can be hit at same time
            /// TODO : move from enum to proper constant expressions in a namespace ? -> uint8_t
            enum class KeyMask
            {
                ALT_UP_COMBINED     = 0x01,
                ALT_DOWN_COMBINED   = 0x02,
                ALT_LEFT_COMBINED   = 0x04,
                ALT_RIGHT_COMBINED  = 0x08,
                UP                  = 0x10,
                DOWN                = 0x20,
                LEFT                = 0x40,
                RIGHT               = 0x80,
            };
        
            /// Mask for mouse button in frametech
            /// TODO : move from enum to proper constant expressions in a namespace ? -> uint8_t
            enum class MouseButtonMask
            {
                LEFT    = 0x01,
                RIGHT   = 0x02,
                MIDDLE  = 0x04
            };
        
            class EventHandler
            {
            public:
                void poll(bool blank = false) noexcept;
                void addKey(const KeyMask mask) noexcept;

            private:
                std::deque<KeyMask> m_key_masks;
                std::mutex m_lock;
            };
        
            class CursorHandler
            {
            public:
                void poll(bool blank = false) noexcept;
                void addMove(const float xpos, const float ypos) noexcept;

            private:
                std::deque<std::tuple<float, float>> m_positions;
                std::mutex m_lock;
            };
        } // namespace inputs
    }     // namespace engine
} // namespace frametech

#endif // _inputs_hpp
