//
//  arg_parse.h
//  FrameTech
//
//  Created by Antonin on 04/11/2022.
//

#ifndef arg_parse_h
#define arg_parse_h

#include "debug_tools.h"
#include <any>
#include <map>
#include <optional>
#include <utility>

/// @brief The smaller prefix for a flag to parse
constexpr char* SMALL_FLAG_PREFIX = (char*)"-";

/// @brief The longest prefix for a flag to parse
constexpr char* LONG_FLAG_PREFIX = (char*)"--";

/// @brief Stores the length of the smaller prefix for a flag to parse
const int SMALL_FLAG_PREFIX_LEN = strlen(SMALL_FLAG_PREFIX);

/// @brief Stores the length of the longest prefix for a flag to parse
const int LONG_FLAG_PREFIX_LEN = strlen(LONG_FLAG_PREFIX);

/// @brief The maximum number of characters for a flag
const int MAX_FLAG_LEN = 32;

/// @brief Returns if the argument, passed as parameter, is a flag or not
/// @param arg A characters array that could represents a flag: a string with a
/// SMALL_FLAG_PREFIX or LONG_FLAG_PREFIX prefix
/// @return A boolean value: true if found, otherwise false
static bool foundFlag(const char* arg)
{
    if (strlen(arg) < LONG_FLAG_PREFIX_LEN)
        return false;
    char first_chars[LONG_FLAG_PREFIX_LEN];
    if (nullptr == strncpy(first_chars, arg, LONG_FLAG_PREFIX_LEN))
        return false;
    return strncmp(first_chars, SMALL_FLAG_PREFIX, SMALL_FLAG_PREFIX_LEN) == 0 ||
           strncmp(first_chars, LONG_FLAG_PREFIX, LONG_FLAG_PREFIX_LEN) == 0;
}

struct ArgParse
{
private:
    std::map<const char*, const char*> arguments;

public:
    /// @brief Parses the arguments, as parameters, and returns an ArgParse object
    /// Warning: Multiple values per argument / flag is not allowed.
    /// A flag must **not** exceed more than a certain number of characters
    /// (the prefix is not included) - please check MAX_FLAG_LEN for that.
    /// @param argc The number of strings
    /// @param argv The strings to parse
    /// @return An ArgParse object
    ArgParse(int argc, const char** argv)
    {
        if (argc == 0)
            return;
        bool found_flag = false;
        char flag_to_match[MAX_FLAG_LEN];
        strncpy(flag_to_match, "\0", MAX_FLAG_LEN);
        for (int i = 0; i < argc; ++i)
        {
            const bool is_flag = foundFlag(argv[i]);
            if (is_flag && found_flag)
            {
                LogW("Found flag '%s' at %dth position, but already found flag '%s' before...", argv[i], i, flag_to_match);
                strncpy(flag_to_match, argv[i], MAX_FLAG_LEN);
                continue;
            }
            if (is_flag)
            {
                found_flag = true;
                strncpy(flag_to_match, argv[i], MAX_FLAG_LEN);
                continue;
            }
            // Not a flag
            if (found_flag)
            {
                // Keep track of the string - the string will be freed
                // in the deallocator
                char* flag_name = new char[MAX_FLAG_LEN];
                strncpy(flag_name, flag_to_match, MAX_FLAG_LEN);
                // Insert and free the old name
                arguments.insert(std::pair<const char*, const char*>(flag_name, argv[i]));
                strncpy(flag_to_match, (char*)"\0", MAX_FLAG_LEN);
                found_flag = false;
            }
        }
        found_flag = false;
        strncpy(flag_to_match, (char*)"\0", MAX_FLAG_LEN);
    }

    ~ArgParse()
    {
        for (std::map<const char*, const char*>::iterator it = arguments.begin(); it != arguments.end(); ++it)
        {
            // We only need to free the key, as those were allocated on the heap (because 'char*' LUL)
            free((void*)it->first);
        }
        arguments.clear();
    }

    /// @brief Look for an argument in the existing / parsed arguments map
    /// @param arg The argument to look for
    /// @return A boolean value: true if already parsed, otherwise false
    bool exists(const char* arg) const
    {
        const auto search = arguments.find(arg);
        return search != arguments.end();
    }

    /// @brief Look for an argument in the existing / parsed arguments map
    /// @param arg The argument to look for
    /// @return An optional value: nullopt if the argument has not been found,
    /// otherwise the **value** of the argument
    std::optional<const char*> get(const char* arg) const
    {
        if (!exists(arg))
            return std::nullopt;
        return arguments.find(arg)->second;
    }
};

#endif /* arg_parse_h */
