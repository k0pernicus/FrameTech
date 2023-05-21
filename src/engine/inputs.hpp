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
    namespace inputs
    {
        enum Key
        {
            ALT_UP_COMBINED,
            ALT_DOWN_COMBINED,
            ALT_LEFT_COMBINED,
            ALT_RIGHT_COMBINED,
            UP,
            DOWN,
            LEFT,
            RIGHT,
        };

        class EventHandler
        {
        public:
            void poll(bool blank = false) noexcept;
            void addKey(const Key key) noexcept;

        private:
            std::deque<Key> m_keys;
            std::mutex m_lock;
        };
    } // namespace inputs
} // namespace frametech

#endif // _inputs_hpp
