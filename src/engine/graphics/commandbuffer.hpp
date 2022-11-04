//
//  pool.hpp
//  FrameTech
//
//  Created by Antonin on 26/10/2022.
//

#pragma once
#ifndef commandbuffer_h
#define commandbuffer_h

#include "../../ftstd/result.h"
#include <vulkan/vulkan.h>

namespace frametech
{
    namespace graphics
    {
        /// @brief Commands buffer
        class CommandBuffer
        {
        public:
            /// @brief Public constructor
            CommandBuffer();
            ~CommandBuffer();
            /// @brief Returns the VkCommandPool object
            /// @return the VkCommandPool object
            VkCommandPool* getPool();
            /// @brief Returns the VkCommandBuffer object
            /// @return the VkCommandBuffer object
            VkCommandBuffer* getBuffer();
            /// @brief Creates the command pool
            /// @return A VResult type
            ftstd::VResult createPool();
            /// @brief Creates the command buffer
            /// @return A VResult type
            ftstd::VResult createBuffer();
            /// @brief Writes the commands we want to execute into a command buffer
            ftstd::VResult record();

        private:
            /// @brief CommandBuffer should not be cloneable
            CommandBuffer(CommandBuffer& other) = delete;
            /// @brief CommandBuffer should not be assignable
            void operator=(const CommandBuffer& other) = delete;
            VkCommandPool m_pool;
            VkCommandBuffer m_buffer;
        };
    } // namespace graphics
} // namespace frametech

#endif // commandbuffer_h
