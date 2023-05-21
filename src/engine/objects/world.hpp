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
    namespace engine
    {
        class World
        {
        public:
            void init() noexcept;
            frametech::engine::MovableInterface* getSelectedObject() noexcept;
            void setSelectedObject(frametech::engine::MovableInterface* object) noexcept;
            frametech::engine::Camera& getMainCamera() noexcept;

        private:
            // TODO: should not consider only one camera here, move it to a smart system
            frametech::engine::Camera m_main_camera{};
            frametech::engine::MovableInterface* m_selected_object = nullptr;
        };
    } // namespace engine
} // namespace frametech

#endif // _world_hpp
