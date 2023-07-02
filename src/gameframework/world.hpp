//
//  world.hpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#pragma once
#ifndef _world_hpp
#define _world_hpp

#include "../engine/graphics/texture.hpp"
#include "camera.hpp"
#include "movable.hpp"
#include <map> // For cache maps
#include <optional>

namespace frametech
{
    namespace gameframework
    {
        class World
        {
        public:
            ~World()
            {
                clean();
            }
            /// @brief Makes the main camera as the first selected object in the world
            void setup() noexcept;
            /// @brief Destroy the current object
            void clean() noexcept;
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
            /// @brief Returns if the world has been setup or not
            /// @return A boolean value
            bool hasBeenSetup() const noexcept;
            /// @brief Textures cache, attached to the world
            /// Each texture is a unique_ptr used by anywhere here
            std::map<std::string, frametech::engine::graphics::Texture*> m_textures_cache;

        private:
            /// @brief Main camera of the world
            /// The "Main" term should be considered here as the first implemented one
            /// TODO: should not consider only one camera here, move it to a smart system
            frametech::gameframework::Camera m_main_camera{};
            /// @brief Returns the current selected object in the world
            frametech::gameframework::MovableInterface* m_selected_object = nullptr;
            /// @brief Internal state to know if the world has been setup
            bool m_has_been_setup = false;
        };
    } // namespace gameframework
} // namespace frametech

#endif // _world_hpp
