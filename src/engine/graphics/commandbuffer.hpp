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

namespace FrameTech
{
    namespace Graphics
    {
        /// @brief Commands buffer
        class CommandBuffer
        {
        public:
            static CommandBuffer* getInstance();
            ~CommandBuffer();
            /// @brief Returns the VkCommandPool object
            /// @return the VkCommandPool object
            VkCommandPool* getPool();
            /// @brief Returns the VkCommandBuffer object
            /// @return the VkCommandBuffer object
            VkCommandBuffer* getBuffer();
            /// @brief Creates the command pool
            /// @return A VResult type
            VResult createPool();
            /// @brief Creates the command buffer
            /// @return A VResult type
            VResult createBuffer();
            /// @brief Writes the commands we want to execute into a command buffer
            VResult record(VkCommandBuffer command_buffer, uint32_t swapchain_index);

        private:
            /// @brief Private constructor in order to create one and only one
            /// CBP for the moment
            CommandBuffer();
            /// @brief CommandBuffer should not be cloneable
            CommandBuffer(CommandBuffer& other) = delete;
            /// @brief CommandBuffer should not be assignable
            void operator=(const CommandBuffer& other) = delete;
            /// @brief The internal instance (singleton) of
            /// the CommandBuffer object
            static CommandBuffer* m_instance;
            VkCommandPool* m_pool = nullptr;
            VkCommandBuffer* m_buffer = nullptr;
        };
    } // namespace Graphics
} // namespace FrameTech

#endif commandbuffer_h // commandbuffer_h
