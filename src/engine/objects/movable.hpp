//
//  movable.hpp
//  FrameTech
//
//  Created by Antonin on 21/05/2023.
//

#pragma once
#ifndef _movable_hpp
#define _movable_hpp

#include "../inputs.hpp"

namespace frametech
{
    namespace engine
    {
        /// @brief Pure abstract class (interface) to let the developer
        /// easily handle movements (frametech's inputs), for any object
        class MovableInterface
        {
        public:
            /// @brief Handles move(ment) key
            /// @param key A key handled by frametech's inputs
            virtual void handleKeyEvent(frametech::inputs::Key& key) noexcept = 0;
        };
    } // namespace engine
} // namespace frametech

#endif // _movable_hpp
