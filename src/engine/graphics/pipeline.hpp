//
//  pipeline.hpp
//  FrameTech
//
//  Created by Antonin on 23/09/2022.
//

#pragma once
#ifndef pipeline_hpp
#define pipeline_hpp

#include "../../ftstd/result.h"
#include <cstdlib>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace FrameTech
{
    namespace Graphics
    {
        namespace Shader
        {
            /// @brief The stage of shader
            enum Type
            {
                COMPUTE_SHADER,
                FRAGMENT_SHADER,
                GEOMETRY_SHADER,
                VERTEX_SHADER,
            };
            /// @brief Useful data for the shader
            struct Module
            {
            public:
                /// @brief The code of the SPIR-V shader.
                char* m_code;
                /// @brief The code size.
                size_t m_size;
                /// @brief The tag of the shader (e.g. its name / filepath).
                char* m_tag;
                /// @brief The stage of the shader, or type.
                Shader::Type m_type;
                /// @brief The entrypoint of the shader program.
                /// Default is 'main'
                char* m_entrypoint = (char*)"main";
            };
        } // namespace Shader
        /// Graphics pipeline representation
        class Pipeline
        {
        public:
            Pipeline();
            ~Pipeline();
            /// @brief Read each shader file passed as parameter, if those exist.
            /// **Warning**: this function is **not** data-race conditons bullet-proof.
            /// TODO: Real return type.
            Result<std::vector<Shader::Module>> createGraphicsApplication(
                const char* vertex_shader_filepath,
                const char* fragment_shader_filepath);
            /// @brief Set the shader stages
            /// @param stages The shader stages to register
            void setShaderStages(const std::vector<VkPipelineShaderStageCreateInfo> stages);
            /// @brief Set the shader modules
            /// @param shader_modules The shader modules to register
            void setShaderModules(const std::vector<VkShaderModule> shader_modules);
            /// @brief Finalizes the graphics pipeline setup, once everything
            /// has been created
            /// @return A VResult type to know if the function succeeded
            /// or not.
            VResult create();
            /// @brief Pre-configures the graphics pipeline:
            /// 1. Creates the shader module,
            /// 2. Configure the fixed functions,
            /// 3. TODO: ...
            /// @return A Result type to know if the function succeeded
            /// or not.
            VResult preconfigure();
            /// @brief Setup the framebuffer attachments that will be used
            /// while rendering, like color and depth buffers, how many
            /// samples do we want to use, etc...
            /// @return A VResult type to know if the function succeeded
            /// or not.
            VResult setupRenderPass();
            /// @brief Returns the registered render pass object
            /// @return A VkRenderPass object
            VkRenderPass& getRenderPass();
            /// @brief Returns the pipeline of this object
            /// @return A VkPipeline object
            VkPipeline getPipeline();
            /// @brief Draw the current frame
            /// @return A result type that corresponds to the error status
            /// of the draw function
            Result<int> draw();
            /// @brief Present the current image to the screen
            /// TODO: should return a VResult
            void present();

        private:
            /// @brief Returns the size, as a `uint64_t` type, of a file located at `filepath`.
            /// If the file does not exists, or can't be read, return a `nullopt` value.
            /// **Warning**: this function is **not** data-race conditons bullet-proof.
            std::optional<uint64_t> fileSize(const char* filepath);
            /// @brief Read the content of a file, located at `filepath`, and put the content of it
            /// in `buffer`.
            /// If `buffer_length` is greater than the real file size, there is a cap on the real file size.
            /// Returns the length that is read, or `nullopt` if an error happened.
            /// **Warning**: this function is **not** data-race conditons bullet-proof.
            std::optional<uint64_t> readFile(const char* filepath, char** buffer, uint64_t buffer_length);
            /// @brief Create all the sync objects (semaphores / fences) to use
            /// in our pipeline / renderer
            /// @return A VResult type to know if the creation has been successfuly
            /// executed or not
            VResult createSyncObjects();
            /// @brief The shader stages in the pipeline
            std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
            /// @brief Stores the shader modules to create the pipeline object later
            std::vector<VkShaderModule> m_shader_modules;
            /// @brief The pipeline layout created for our renderer
            VkPipelineLayout m_layout = VK_NULL_HANDLE;
            /// @brief The render pass object
            VkRenderPass m_render_pass = VK_NULL_HANDLE;
            /// @brief The pipeline object
            VkPipeline m_pipeline = VK_NULL_HANDLE;
            /// @brief Sync object to signal that an image is ready to
            /// be displayed
            VkSemaphore* m_sync_image_ready = nullptr;
            /// @brief Sync object to signal that the rendering
            /// is done for the current frame
            VkSemaphore* m_sync_present_done = nullptr;
            /// @brief Sync object for CPU / GPU
            VkFence* m_sync_cpu_gpu = nullptr;
        };
    } // namespace Graphics
} // namespace FrameTech

#endif /* pipeline_hpp */
