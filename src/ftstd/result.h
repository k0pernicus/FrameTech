//
//  result.h
//  FrameTech
//
//  Created by Antonin on 25/09/2022.
//

#pragma once
#ifndef result_h
#define result_h

#include "debug_tools.h"
#include <assert.h>
#include <cstdint>

/// @brief Default value for an OK status code
constexpr int8_t RESULT_OK = 0;

/// @brief Default value for an ERROR status code
constexpr int8_t RESULT_ERROR = -1;

namespace ftstd
{
    /// @brief A very simple Result type for the Engine
    /// @tparam T The result that is contained in the container
    /// if and only if there was no error.
    template <typename T>
    struct Result
    {
    private:
        /// @brief The error code, if it exists.
        /// If there is no error, should be `RESULT_OK`.
        int8_t m_error;
        /// @brief The error message, if the container
        /// does contain an error.
        char* m_error_exp;
        /// @brief The value of the container, if the
        /// container should not contain any error.
        T m_result;

    public:
        /// @brief Returns if the container contains an error.
        /// @return A boolean value: `true` if the container contains
        /// an error, `false` elsewhere.
        inline bool IsError() const { return m_error != RESULT_OK; }
        /// @brief Returns if the container contains an error.
        /// @return A boolean value: `true` if the container contains
        /// an error, `false` elsewhere.
        inline bool IsError() { return m_error != RESULT_OK; }
        /// @brief Returns a copy of the internal value.
        /// @return The internal value of the container, as a copy.
        /// Check first if the container does not contain any error.
        T GetValue() const
        {
            assert(m_error == RESULT_OK);
            assert(m_error_exp == nullptr);
            return m_result;
        }
        /// @brief Returns a reference of the internal value.
        /// @return The internal value of the container, as a reference.
        /// Check first if the container does not contain any error.
        T* RefValue() const
        {
            assert(m_result != NULL);
            assert(m_error == RESULT_OK);
            assert(m_error_exp == nullptr);
            return &m_result;
        }
        /// @brief Returns the error value of the container, if there is one.
        /// @return The error value, as a characters array, if there is one.
        const char* GetError()
        {
            assert(m_error == RESULT_ERROR);
            assert(m_error_exp != nullptr);
            return m_error_exp;
        }
        /// @brief Initializes the Result type, as an error.
        /// Puts `RESULT_ERROR` as default error code.
        /// @param error_msg An error message.
        static Result Error(char* error_msg)
        {
            LogE("%s", error_msg);
            Result result;
            result.m_error = RESULT_ERROR;
            result.m_error_exp = error_msg;
            return result;
        }
        /// @brief Initializes the Result type, as an error.
        /// @param error_code An error code.
        /// @param error_msg An error message.
        static Result Error(int8_t error_code, char* error_msg)
        {
            LogE("%s", error_msg);
            Result result;
            result.m_error = error_code;
            result.m_error_exp = error_msg;
            return result;
        }
        /// @brief Initializes the Result type, as an Ok type.
        /// @param result The result value, which is not an error.
        static Result Ok(T c_result)
        {
            Result result;
            result.m_error = RESULT_OK;
            result.m_error_exp = nullptr;
            result.m_result = c_result;
            return result;
        }
    };

    /// @brief Basic type returned from a function to know
    /// if an error happened and when - should not contains
    /// any useful information (exception if the container
    /// contains an error).
    /// VResult stands for "Void Result".
    /// Very basic alternative to "exception tracing".
    struct VResult
    {
    private:
        /// @brief The error code, if it exists.
        /// If there is no error, should be `RESULT_OK`.
        int8_t m_error;
        /// @brief The error message, if the container
        /// does contain an error.
        char* m_error_exp;

    public:
        /// @brief Returns if the container contains an error.
        /// @return A boolean value: `true` if the container contains
        /// an error, `false` elsewhere.
        inline bool IsError() const { return m_error != RESULT_OK; }
        /// @brief Returns if the container contains an error.
        /// @return A boolean value: `true` if the container contains
        /// an error, `false` elsewhere.
        inline bool IsError() { return m_error != RESULT_OK; }
        /// @brief Returns the error value of the container, if there is one.
        /// @return The error value, as a characters array, if there is one.
        const char* GetError()
        {
            assert(m_error == RESULT_ERROR);
            assert(m_error_exp != nullptr);
            return m_error_exp;
        }
        /// @brief Initializes the Result type, as an error.
        /// Puts `RESULT_ERROR` as default error code.
        /// @param error_msg An error message.
        static VResult Error(char* error_msg)
        {
            LogE("%s", error_msg);
            VResult result;
            result.m_error = RESULT_ERROR;
            result.m_error_exp = error_msg;
            return result;
        }
        /// @brief Initializes the Result type, as an error.
        /// @param error_code An error code.
        /// @param error_msg An error message.
        static VResult Error(int8_t error_code, char* error_msg)
        {
            LogE("%s", error_msg);
            VResult result;
            result.m_error = error_code;
            result.m_error_exp = error_msg;
            return result;
        }
        /// @brief Initializes the Result type, as an Ok type.
        /// @param result The result value, which is not an error.
        static VResult Ok()
        {
            VResult result;
            result.m_error = RESULT_OK;
            result.m_error_exp = nullptr;
            return result;
        }
    };
} // namespace ftstd

#endif // result_h
