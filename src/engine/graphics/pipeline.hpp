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
            /// @brief The shader stages in the pipeline
            std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
        };
    } // namespace Graphics
} // namespace FrameTech

#endif /* pipeline_hpp */
