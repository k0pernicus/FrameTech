//
//  device.hpp
//  FrameTech
//
//  Created by Antonin on 26/09/2022.
//

#pragma once
#ifndef device_h
#define device_h

#include "../../ftstd/result.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace frametech
{
    namespace graphics
    {
        /// @brief Stores the state of a Graphics queue
        enum QueueState
        {
            /// @brief The queue will not be used for the engine
            UNSUPPORTED,
            /// @brief The queue is ready to be used
            READY,
            /// @brief The queue is already being used
            USED,
        };

        /// @brief Enumerates the features supported, or requested, by FrameTech
        enum SupportFeatures
        {
            NOONE = 0x00000000,
            GRAPHICS = 0x00000001,
            PRESENTS = 0x00000002,
            TRANSFERT = 0x00000004,
        };

        class Device
        {
        public:
            /// @brief Destructor
            ~Device();
            /// @brief Returns the number of physical devices found in the running computer.
            /// @return The number of physical devices found that **may** be suitable for our needs.
            uint32_t getNumberDevices() const;
            /// @brief Lists the devices that **may** be suitable for our needs.
            /// @return `true` if the function founds a suitable physical device, otherwise `false`.
            ftstd::VResult listDevices();
            /// @brief Returns if the device is suitable for graphical needs or not.
            /// @param device The device to check.
            /// To be suitable, the device must:
            /// 1. Be an Apple Silicon chip (the internal name should be APPLE_M1_NAME, no Apple M1 Pro / Max / M2 chips allowed),
            /// 2. Be a discrete GPU,
            /// 3. Supports geometry shader.
            /// The device **must** follows the rule 1, or (2 and 3).
            /// @return If the device follows the previously explicited rules.
            bool isInitialized() const;
            /// @brief Find supported queues on the device
            ftstd::Result<uint32_t> getQueueFamilies();
            /// @brief Creates a logical device based on the setted physical device
            ftstd::VResult createLogicalDevice();
            /// @brief Returns the logical device
            VkDevice getLogicalDevice() const;
            /// @brief Returns the logical device
            VkPhysicalDevice getPhysicalDevice() const;
            /// @brief Clean and destroy the logical device, if it has been set
            void Destroy();
            /// @brief Store the index of the graphics queue family
            uint32_t m_graphics_queue_family_index = 0;
            /// @brief Store the index of the presents queue family
            uint32_t m_presents_queue_family_index = 0;
            /// @brief Store the index of the presents queue family
            uint32_t m_transfert_queue_family_index = 0;
            /// @brief Returns the Graphics queue of the logical device
            /// @return The Graphics queue of the logical device
            VkQueue& getGraphicsQueue();
            /// @brief Returns the Present queue of the logical device
            /// @return The Present queue of the logical device
            VkQueue& getPresentsQueue();
            /// @brief Returns the Transfert queue of the logical device
            /// @return The Transfert queue of the logical device
            VkQueue& getTransfertQueue();

        private:
            /// @brief The physical device that has been picked
            VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
            /// @brief The support state (SupportFeature) associated to each queue,
            /// for the physical device
            std::vector<int> m_queue_support;
            /// @brief To set and to get the state of the different family queues
            std::vector<QueueState> m_queue_states;
            /// @brief The logical device associated to the physical device
            VkDevice m_logical_device = VK_NULL_HANDLE;
            /// @brief Interface to the graphics queue
            VkQueue m_graphics_queue = VK_NULL_HANDLE;
            /// @brief Interface to the presents queue
            /// The presents queue **may** be the same than
            /// the graphics queue
            VkQueue m_presents_queue = VK_NULL_HANDLE;
            /// @brief Interface to the transfert queue
            /// The transfert queue can be the same queue than
            /// the presents / graphics one
            VkQueue m_transfert_queue = VK_NULL_HANDLE;
        };
    } // namespace graphics
} // namespace frametech

#endif // device_h
