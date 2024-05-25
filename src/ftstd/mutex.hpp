//
//  mutex.hpp
//  FrameTech
//
//  Created by Antonin on 23/05/2023.
//

#pragma once
#ifndef _mutex_hpp
#define _mutex_hpp

#include "debug_tools.h"
#include <mutex>

namespace ftstd
{
    namespace mutex
    {
        /// @brief Maximum length of a scope label, for debugging purposes only
        constexpr uint32_t SCOPE_LABEL_MAX_LENGTH = 64;
        /// @brief Scope mutex class
        /// The lock is done as soon as the object has been created,
        /// and the unlock once the object is destroyed (scope end has been reached)
        /// WARNING: You should not use the object instance in the heap !
        class ScopedMutex
        {
        private:
            /// @brief The mutex to lock - copied for the destructor
            std::mutex* m_lk;
            /// @brief A label associated to the ScopedMutex object, for
            /// debugging purposes only
            // char m_label[SCOPE_LABEL_MAX_LENGTH];

        public:
            ScopedMutex(std::mutex* mx, const char* scope_label = "scope\0") : m_lk(mx)
            {
//                strncpy(m_label, scope_label, SCOPE_LABEL_MAX_LENGTH - 1);
//                m_label[SCOPE_LABEL_MAX_LENGTH - 1] = '\0';
//                Log("[SCOPE_MUTEX] locking '%s'", m_label);
                m_lk->lock();
            }
            ~ScopedMutex()
            {
//                Log("[SCOPE_MUTEX] unlocking '%s'", m_label);
                m_lk->unlock();
            }
        };
    } // namespace mutex
} // namespace ftstd

#endif // _mutex_hpp
