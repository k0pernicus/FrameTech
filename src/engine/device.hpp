//
//  device.hpp
//  FrameTech
//
//  Created by Antonin on 26/09/2022.
//

#ifndef device_h
#define device_h

#include "../result.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace FrameTech
{
    enum QueueState
    {
        UNSUPPORTED,
        READY,
        USED,
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
        Result<int> listDevices();
        /// @brief Returns if the device is suitable for graphical needs or not.
        /// @param device The device to check.
        /// To be suitable, the device must:
        /// 1. Be an Apple Silicon chip (the internal name should be APPLE_M1_NAME, no Apple M1 Pro / Max / M2 chips allowed),
        /// 2. Be a discrete GPU,
        /// 3. Supports geometry shader.
        /// The device **must** follows the rule 1, or (2 and 3).
        /// @return If the device follows the previously explicited rules.
        inline bool isInitialized() const;
        /// @brief Find supported queues on the device
        Result<uint32_t> getQueueFamilies();
        /// @brief Returns if any queue is available to use
        bool isAnyQueueAvailable() const;
        Result<int> createLogicalDevice();

    private:
        /// @brief The physical device that has been picked
        VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
        /// @brief The state associated to each queue, for the stored
        /// physical device
        std::vector<QueueState> m_queue_states;
        /// @brief The logical device associated to the physical device
        VkDevice m_logical_device;
    };
} // namespace FrameTech

#endif // device_h
