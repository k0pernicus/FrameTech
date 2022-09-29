//
//  swapchain.hpp
//  FrameTech
//
//  Created by Antonin on 29/09/2022.
//

#pragma once
#ifndef swapchain_h
#define swapchain_h

#include "../../result.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

namespace FrameTech
{
    namespace Graphics
    {

        /// @brief The maximum number of buffers / images
        /// that can be store, waiting to be presenting
        static constexpr uint8_t MAX_BUFFERS = 3;

        /// @brief Structure to get any detail about a swapchain
        /// object.
        /// There are three things to check, in order to know if the
        /// swapchain (required) object matches the window surface:
        /// 1. basic surface capabilities (min/max images, height/width of images, ...),
        /// 2. surface formats (pixel and color formats),
        /// 3. available presentation ('present' for short) modes
        struct SwapChainSupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> present_modes;
        };

        /// @brief An abstraction around the VK queue of images
        ///  that are waiting to be presenting on the screen
        class SwapChain
        {
        public:
            /// @brief Returns the static instance (singleton) of the object
            /// @return A static SwapChain pointer
            static SwapChain* getInstance();
            /// @brief Destructor
            ~SwapChain();
            /// @brief Get details about the swapchain
            void queryDetails();
            /// @brief Check if the number of images (or buffers)
            /// set up in the engine is between the minimum and maximum
            /// supported number of images for the swapchain, and if there
            /// is at least one supported image format and one
            /// presentation mode
            Result<int> checkDetails();
            /// @brief Creates the Vulkan swapchain object
            /// This function should not be called before `queryDetails`
            /// and `checkDetails` functions!
            Result<int> create();

        private:
            SwapChain();
            /// @brief SwapChain should not be cloneable
            SwapChain(SwapChain& other) = delete;
            /// @brief SwapChain should not be assignable
            void operator=(const SwapChain& other) = delete;
            /// @brief The internal instance (singleton) of
            /// the SwapChain object
            static SwapChain* m_instance;
            /// @brief The swapchain Vulkan object
            VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
            /// @brief Any stored details about the SwapChain object
            SwapChainSupportDetails m_details;
        };
    } // namespace Graphics
} // namespace FrameTech

#endif // swapchain_h
