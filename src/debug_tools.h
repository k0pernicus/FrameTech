//
//  debug_tools.h
//  FrameTech
//
//  Created by Antonin on 18/09/2022.
//

#ifndef debug_tools_h
#define debug_tools_h

#if defined(DEBUG) && DEBUG == 1
// DEBUG

#include <cassert>
#include <stdio.h>
#include <string>
#include <time.h>

// TODO: should be defined elsewhere
// (proper preprocessor for example)
#define ENABLE_EXCEPTIONS

/// @brief Build and prints a log statement.
/// The prefix argument is optional.
template <typename... Args>
void build_log(const char* prefix, Args... message)
{
    // Get timestamp
    time_t ltime;
    ltime = time(NULL);
    char* time = asctime(localtime(&ltime));
    assert(time != nullptr);
    // Remove the newline character
    if (time != nullptr)
        time[strlen(time) - 1] = '\0';
    // Print the date and the prefix
    if (prefix == nullptr)
        printf("[%s] ", time);
    else
        printf("[%s] %s: ", time, prefix);
    // Print the full message
    printf(message...);
    printf("\n");
}

#define Log(...) build_log(nullptr, __VA_ARGS__)
#define LogE(...) build_log("Error", __VA_ARGS__)
#define LogW(...) build_log("Warning", __VA_ARGS__)
#define WARN_RT_UNIMPLEMENTED assert(0)
#define WARN_CT_UNIMPLEMENTED static_assert(0)

#else
// PROFILE & RELEASE

#define Log(...)
#define LogE(...)
#define LogW(...)
#define WARN_RT_UNIMPLEMENTED ()
#define WARN_CT_UNIMPLEMENTED ()

#endif

#endif /* debug_tools_h */
