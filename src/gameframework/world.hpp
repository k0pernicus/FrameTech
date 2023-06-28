//
//  world.hpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#pragma once
#ifndef _world_hpp
#define _world_hpp

#include "camera.hpp"
#include "movable.hpp"
#include <optional>

namespace frametech
{
    namespace gameframework
    {
        class World
        {
        public:
            /// @brief Makes the main camera as the first selected object in the world
            void init() noexcept;
            /// @brief Returns the current selected object
            /// @return A pointer to a MovableInterface inherited object
            frametech::gameframework::MovableInterface* getSelectedObject() noexcept;
            /// @brief Sets a new MovableInterface inherited object as the current
            /// selected object
            /// @param object The object to set
            void setSelectedObject(frametech::gameframework::MovableInterface* object) noexcept;
            /// @brief Returns the main camera object
            /// @return A reference to the main camera object
            frametech::gameframework::Camera& getMainCamera() noexcept;

        private:
            /// @brief Main camera of the world
            /// The "Main" term should be considered here as the first implemented one
            /// TODO: should not consider only one camera here, move it to a smart system
            frametech::gameframework::Camera m_main_camera{};
            /// @brief Returns the current selected object in the world
            frametech::gameframework::MovableInterface* m_selected_object = nullptr;
        };
    } // namespace gameframework
} // namespace frametech

#endif // _world_hpp
