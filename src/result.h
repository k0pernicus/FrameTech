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

#define RESULT_OK 0
#define RESULT_ERROR 0

/// @brief A very simple Result type for the Engine
/// @tparam T The result that is contained in the container
/// if and only if there was no error.
template <typename T>
struct Result
{
private:
    /// @brief The error code, if it exists.
    /// If there is no error, should be `RESULT_OK`.
    int m_error;
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
    T GetValue() const { return m_result; }
    /// @brief Returns a reference of the internal value.
    /// @return The internal value of the container, as a reference.
    /// Check first if the container does not contain any error.
    T* RefValue() const { return &m_result; }
    /// @brief Returns the error value of the container, if there is one.
    /// @return The error value, as a characters array, if there is one.
    const char* GetError() { return m_error_exp; }
    /// @brief Initializes the Result type, as an error.
    /// @param error_code An error code.
    /// @param error_msg An error message.
    void Error(int error_code, char* error_msg)
    {
        LogE("%s", error_msg);
        m_error = error_code;
        m_error_exp = error_msg;
    }
    /// @brief Initializes the Result type, as an Ok type.
    /// @param result The result value, which is not an error.
    void Ok(T result)
    {
        m_error = 0;
        m_error_exp = nullptr;
        m_result = result;
    }
};

#endif // result_h
