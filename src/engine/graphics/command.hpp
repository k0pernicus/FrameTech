//
//  command.hpp
//  FrameTech
//
//  Created by Antonin on 26/10/2022.
//

#pragma once
#ifndef command_h
#define command_h

#include "../../ftstd/result.hpp"
#include <vulkan/vulkan.h>

namespace frametech
{
    namespace graphics
    {
        enum class CommandState
        {
            S_UNKNOWN,
            S_BEGAN,
            S_ENDED,
            S_ERROR,
        };
        /// @brief Command
        class Command
        {
        public:
            /// @brief Public constructor
            Command();
            /// @brief Public constructor
            /// @param command_pool The command pool to use
            Command(VkCommandPool command_pool);
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
            /// @brief Creates and initialize the internal buffer
            /// @return A VResult type
            ftstd::VResult createBuffer();
            /// @brief Initializes the command buffer
            /// @return A VResult type
            ftstd::VResult begin();
            /// @brief Ends the record of the current command buffer
            /// @return A VResult type
            ftstd::VResult end(const VkQueue& queue, const uint32_t submit_count);
            /// @brief Transition barrier for an image
            /// @param memory_barrier Image memory barrier for the memory transition
            void transition(
                const VkImage& image,
                const VkImageLayout new_layout,
                const VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
                const uint32_t src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
                const uint32_t dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED) const noexcept;
            /// @brief Writes the commands we want to execute into a command buffer
            ftstd::VResult record();
            /// @brief The queue family index the command pool
            /// has been created with
            uint32_t m_queue_family_index_created_with = 0;

        private:
            /// @brief Command should not be cloneable
            Command(Command& other) = delete;
            /// @brief Command should not be assignable
            void operator=(const Command& other) = delete;
            /// @brief The command pool
            VkCommandPool m_pool = nullptr;
            /// @brief The command buffer
            VkCommandBuffer m_buffer = nullptr;
            /// @brief State of the command buffer,
            /// debugging purposes
            CommandState m_state = CommandState::S_UNKNOWN;
        };
    } // namespace graphics
} // namespace frametech

#endif // command_h
