//
//  pipeline.hpp
//  FrameTech
//
//  Created by Antonin on 23/09/2022.
//

#pragma once
#ifndef pipeline_hpp
#define pipeline_hpp

#include "../result.h"
#include <cstdlib>
#include <optional>

namespace FrameTech
{

    /// Graphics pipeline representation
    class Pipeline
    {
    public:
        Pipeline();
        ~Pipeline();
        /// @brief Read each shader file passed as parameter, if those exist.
        /// **Warning**: this function is **not** data-race conditons bullet-proof.
        /// TODO: Real return type.
        Result<int> create(const char* vertex_shader_filepath, const char* fragment_shader_filepath);

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
        std::optional<uint64_t> readFile(const char* filepath, char* buffer, uint64_t buffer_length);
    };

} // namespace FrameTech

#endif /* pipeline_hpp */
