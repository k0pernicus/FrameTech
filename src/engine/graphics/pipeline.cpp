//
//  pipeline.cpp
//  FrameTech
//
//  Created by Antonin on 23/09/2022.
//

#include "pipeline.hpp"
#include "../../ftstd/debug_tools.h"
#include "../../ftstd/result.h"
#include <filesystem>
#include <fstream>
#include <vector>

FrameTech::Graphics::Pipeline::Pipeline() {}

FrameTech::Graphics::Pipeline::~Pipeline() {}

std::optional<uint64_t> FrameTech::Graphics::Pipeline::fileSize(const char* filepath)
{
    try
    {
        const auto file_size = std::filesystem::file_size(filepath);
        return file_size;
    }
    catch (const std::exception& e)
    {
        LogE("< Error getting the file size at path '%s'", filepath);
        return std::nullopt;
    }
}

std::optional<uint64_t> FrameTech::Graphics::Pipeline::readFile(const char* filepath,
                                                                char** buffer,
                                                                uint64_t buffer_length)
{
    std::ifstream file(filepath, std::ifstream::binary | std::ifstream::ate);
    assert(file);
    if (file)
    {
        std::optional<uint64_t> opt_length = fileSize(filepath);
        if (opt_length == std::nullopt)
        {
            return std::nullopt;
        }
        uint64_t length = opt_length.value();

        // do not read more than enough
        length = buffer_length < length ? (int)buffer_length : length;

        // read data as a block, close, and return
        // the length of the file
        file.seekg(0);
        file.read(*buffer, length);
        file.close();
        return length;
    }
    return std::nullopt;
}

Result<std::vector<FrameTech::Graphics::Shader::Module>> FrameTech::Graphics::Pipeline::createGraphicsApplication(const char* vertex_shader_filepath,
                                                                                                                  const char* fragment_shader_filepath)
{
    // Get the length
    const auto vs_file_size_opt = fileSize(vertex_shader_filepath);
    assert(vs_file_size_opt != std::nullopt);
    const auto fs_file_size_opt = fileSize(fragment_shader_filepath);
    assert(fs_file_size_opt != std::nullopt);
    // If one of them are empty, fail
    if (vs_file_size_opt == std::nullopt || fs_file_size_opt == std::nullopt)
    {
        LogE("< Cannot create the program");
        return Result<std::vector<FrameTech::Graphics::Shader::Module>>::Error((char*)"vertex or fragment shader is NULL");
    }
    // Get the content of the VS
    const auto vs_file_size = vs_file_size_opt.value();
    auto vs_buffer = new char[vs_file_size];
    readFile(vertex_shader_filepath, &vs_buffer, vs_file_size);
    Log("> For VS file '%s', read file ok (%d bytes)", vertex_shader_filepath, vs_file_size);
    // Get the content of the FS
    const auto fs_file_size = fs_file_size_opt.value();
    auto fs_buffer = new char[fs_file_size];
    readFile(fragment_shader_filepath, &fs_buffer, fs_file_size);
    Log("> For FS file '%s', read file ok (%d bytes)", fragment_shader_filepath, fs_file_size);
    std::vector<FrameTech::Graphics::Shader::Module> shader_modules(
        {FrameTech::Graphics::Shader::Module{
             .m_tag = (char*)fragment_shader_filepath,
             .m_code = fs_buffer,
             .m_size = fs_file_size,
             .m_type = FrameTech::Graphics::Shader::FRAGMENT_SHADER,
         },
         FrameTech::Graphics::Shader::Module{
             .m_tag = (char*)vertex_shader_filepath,
             .m_code = vs_buffer,
             .m_size = vs_file_size,
             .m_type = FrameTech::Graphics::Shader::VERTEX_SHADER,
         }});
    return Result<std::vector<FrameTech::Graphics::Shader::Module>>::Ok(shader_modules);
}

void FrameTech::Graphics::Pipeline::setShaderStages(const std::vector<VkPipelineShaderStageCreateInfo> stages)
{
    m_shader_stages = stages;
}
