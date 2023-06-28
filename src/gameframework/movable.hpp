//
//  movable.hpp
//  FrameTech
//
//  Created by Antonin on 21/05/2023.
//

#pragma once
#ifndef _movable_hpp
#define _movable_hpp

#include "../engine/inputs/inputs.hpp"

namespace frametech
{
    namespace gameframework
    {
        /// @brief Pure abstract class (interface) to let the developer
        /// easily handle movements (frametech's inputs), for any object
        class MovableInterface
        {
        public:
            /// @brief Handles move(ment) key
            /// @param key A key handled by frametech's inputs
            /// TODO: check if necessary to set, as parameter, if the input needs to propagate to children / parents
            /// or not
            virtual void handleKeyEvent(frametech::engine::inputs::Key& key) noexcept = 0;
        };
    } // namespace gameframework
} // namespace frametech

#endif // _movable_hpp
