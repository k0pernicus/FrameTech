//
//  pipeline.hpp
//  FrameTech
//
//  Created by Antonin on 23/09/2022.
//

#pragma once
#ifndef pipeline_hpp
#define pipeline_hpp

#include "common.hpp"
#include "../shaders.h"
#include "../../ftstd/result.hpp"
#include "mesh.hpp"
#include "ubo.hpp"
#include <cstdlib>
#include <optional>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace frametech
{
    namespace graphics
    {
        namespace Shader
        {
            /// @brief The stage of shader
            enum struct Type
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
            ftstd::Result<std::vector<Shader::Module>> createGraphicsApplication(
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
            ftstd::VResult create();
            /// @brief Pre-configures the graphics pipeline:
            /// 1. Creates the shader module,
            /// 2. Configure the fixed functions,
            /// 3. TODO: ...
            /// @return A Result type to know if the function succeeded
            /// or not.
            ftstd::VResult preconfigure();
            /// @brief Setup the framebuffer attachments that will be used
            /// while rendering, like color and depth buffers, how many
            /// samples do we want to use, etc...
            /// @return A VResult type to know if the function succeeded
            /// or not.
            ftstd::VResult setupRenderPass();
            /// @brief Returns the registered render pass object
            /// @return A VkRenderPass object
            VkRenderPass& getRenderPass();
            /// @brief Returns the bounded vertices
            /// @return The bounded vertices (may be an empty vector)
            const std::vector<frametech::shaders::Vertex>& getVertices() noexcept;
            /// @brief Returns the vertices to draw, through their index
            /// @return The indices of the vertices to draw (may be an empty vector)
            const std::vector<uint32_t>& getIndices() noexcept;
            /// @brief Returns a reference to the current vertex buffer
            /// @return A reference to the current vertex buffer
            const VkBuffer& getVertexBuffer() noexcept;
            /// @brief Returns a reference to the current index buffer
            /// @return A reference to the current index buffer
            const VkBuffer& getIndexBuffer() noexcept;
            /// @brief Returns the pipeline of this object
            /// @return A VkPipeline object
            VkPipeline getPipeline();
            /// @brief Creates a Vertex Buffer object to use for our shaders
            /// @return A VResult type to know if the function succeeded or not
            ftstd::VResult createVertexBuffer() noexcept;
            /// @brief Creates an index buffer object to store the vertices to use to display our objects
            /// @return A VResult type to know if the function succeeded or not
            ftstd::VResult createIndexBuffer() noexcept;
            /// @brief Creates the uniform buffers to use in our pipeline - should corresponds to the
            /// maximum number of frames in flight possible
            /// @return A VResult type to know if the function succeeded or not
            ftstd::VResult createUniformBuffers() noexcept;
            /// @brief Updates, if created, the uniform buffer bounded to the current frame,
            /// in our current graphics pipeline.
            /// This function should be call every frame to get the latest / current transformation.
            /// @param current_frame_index A uint32_t value that represents the current frame index, in order
            /// to update **only** the right array value
            void updateUniformBuffer(const uint32_t current_frame_index, UniformBufferObject& ubo) noexcept;
            /// @brief Creates a descriptor set layout (data layout) to let the shaders
            /// access to any resource (buffer / image / ...)
            /// @param descriptor_count The number of descriptors to pass (specify 1 for a single data, or X to
            /// specify an array of data). **This number should be greater than 0**.
            /// @param shader_stages The stages to associate the newly (in creation) descriptor set layout (flags).
            /// @param descriptor_type The associated type to the newly (in creation) descriptor set layout.
            /// VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER by default.
            /// @param samplers Optional samplers to associate, for image sampling related descriptors.
            /// nullptr by default.
            /// @return A VResult type to know if the function succeeded or not.
            ftstd::VResult createDescriptorSetLayout(
                const uint32_t descriptor_count,
                const VkShaderStageFlags shader_stages,
                const VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                const VkSampler* samplers = nullptr) noexcept;
            /// @brief Creates the descriptor sets
            /// @param descriptor_count The number of descriptors to pass (specify 1 for a single data, or X to
            /// specify an array of data). **This number should be greater than 0**.
            /// @param descriptor_type The associated type to the newly (in creation) descriptor set layout.
            /// VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER by default.
            /// @return A VResult type to know if the function succeeded or not.
            ftstd::VResult createDescriptorSets(
                const uint32_t descriptor_count,
                const VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) noexcept;
            /// @brief Returns the pipeline layout
            /// @return a VkPipelineLayout type
            VkPipelineLayout getPipelineLayout() noexcept;
            /// @brief Returns the full descriptor sets, as a std::Vector type
            /// @return The vector of descriptor sets
            std::vector<VkDescriptorSet>& getDescriptorSets() noexcept;
            /// @brief Returns the descriptor set at a given index, if it exists
            /// @param index The index to get the descriptor set
            /// @return A descriptor set at index `index` as an optional: nullopt means the index is greater
            /// of the maximum number of frames in flight possible
            std::optional<VkDescriptorSet*> getDescriptorSet(const uint32_t index) noexcept;
            /// @brief Returns the Mesh object stored in the object
            /// @return A reference to the stored Mesh object
            const frametech::graphics::Mesh& getMesh() noexcept;
            /// @brief Updates the Mesh object to render on screen
            /// WARNING: this function is absolutely not safe as it destroys
            /// the entire buffers before creating new ones...
            /// @param new_mesh a Mesh2D enum
            void setMesh2D(frametech::graphics::Mesh2D new_mesh) noexcept;
            /// @brief Returns the selected transformation to apply to vertices
            /// @return A Transformation enum
            frametech::graphics::Transformation getTransform() noexcept {
                return m_transform;
            }
            /// @brief Updates the Transformation enum stored in the pipeline
            /// @param new_transform The new transformation to apply, as an enum
            void setTransform(frametech::graphics::Transformation new_transform) noexcept {
                m_transform = new_transform;
            }
            /// @brief Performs the acquire image call
            void acquireImage();
            /// @brief Draw the current frame
            /// @return A result type that corresponds to the error status
            /// of the draw function
            ftstd::Result<int> draw();
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
            ftstd::VResult createSyncObjects();
            /// @brief The shader stages in the pipeline
            std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
            /// @brief Stores the shader modules to create the pipeline object later
            std::vector<VkShaderModule> m_shader_modules;
            /// @brief The pipeline layout created for our renderer
            VkPipelineLayout m_layout = VK_NULL_HANDLE;
            /// @brief A descriptor set layout to bind & pass information to shaders
            VkDescriptorSetLayout m_descriptor_set_layout = VK_NULL_HANDLE;
            /// @brief Stores the descriptor sets per frame
            std::vector<VkDescriptorSet> m_descriptor_sets;
            /// @brief The render pass object
            VkRenderPass m_render_pass = VK_NULL_HANDLE;
            /// @brief The pipeline object
            VkPipeline m_pipeline = VK_NULL_HANDLE;
            /// @brief The vertex buffer
            VkBuffer m_vertex_buffer = VK_NULL_HANDLE;
            /// @brief The vertex buffer allocation object
            VmaAllocation m_vertex_buffer_allocation = {};
            /// @brief The index buffer
            VkBuffer m_index_buffer = VK_NULL_HANDLE;
            /// @brief The index buffer allocation object
            VmaAllocation m_index_buffer_allocation = {};
            /// @brief Uniform buffers
            std::vector<VkBuffer> m_uniform_buffers;
            /// @brief Memory addresses of the uniform buffers
            std::vector<VmaAllocation> m_uniform_buffers_allocation;
            /// @brief To know if / which uniform buffers are currently used
            /// TODO: boolean type instead ?
            std::vector<void*> m_uniform_buffers_data;
            /// @brief The default mesh to display
            frametech::graphics::Mesh m_mesh = frametech::graphics::MeshUtils::getMesh2D(frametech::graphics::Mesh2D::BASIC_TRIANGLE);
            /// @brief The selected transformation
            frametech::graphics::Transformation m_transform = frametech::graphics::Transformation::Constant;
            /// @brief Sync object to signal that an image is ready to
            /// be displayed
            VkSemaphore* m_sync_image_ready = nullptr;
            /// @brief Sync object to signal that the rendering
            /// is done for the current frame
            VkSemaphore* m_sync_present_done = nullptr;
            /// @brief Sync object for CPU / GPU
            VkFence* m_sync_cpu_gpu = nullptr;
        };
    } // namespace graphics
} // namespace frametech

#endif /* pipeline_hpp */
