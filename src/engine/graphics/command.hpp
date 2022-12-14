//
//  command.hpp
//  FrameTech
//
//  Created by Antonin on 26/10/2022.
//

#pragma once
#ifndef command_h
#define command_h

#include "../../ftstd/result.h"
#include <vulkan/vulkan.h>

namespace frametech
{
    namespace graphics
    {
        /// @brief Command
        class Command
        {
        public:
            /// @brief Public constructor
            Command();
            /// @brief Public destructor
            ~Command();
            /// @brief Returns the VkCommandPool object
            /// @return the VkCommandPool object
            VkCommandPool* getPool();
            /// @brief Returns the VkCommandBuffer object
            /// @return the VkCommandBuffer object
            VkCommandBuffer* getBuffer();
            /// @brief Creates the command pool
            /// @param family_index The queue family index to bind
            /// @return A VResult type
            ftstd::VResult createPool(const uint32_t family_index);
            /// @brief Creates the command buffer
            /// @return A VResult type
            ftstd::VResult createBuffer();
            /// @brief Writes the commands we want to execute into a command buffer
            ftstd::VResult record();

        private:
            /// @brief Command should not be cloneable
            Command(Command& other) = delete;
            /// @brief Command should not be assignable
            void operator=(const Command& other) = delete;
            /// @brief The command pool
            VkCommandPool m_pool;
            /// @brief The command buffer
            VkCommandBuffer m_buffer;
        };
    } // namespace graphics
} // namespace frametech

#endif // command_h
