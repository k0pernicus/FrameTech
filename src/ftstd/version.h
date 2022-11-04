//
//  version.h
//  FrameTech
//
//  Created by Antonin on 18/09/2022.
//

#pragma once
#ifndef version_h
#define version_h

#include <regex>
#include <stdexcept>
#include <stdlib.h>

const std::regex VERSION_REGEX("(\\d+).(\\d+).(\\d+)");

namespace ftstd
{

    /**
     * Version formats the current version as:
     * 1. A major number,
     * 2. A minor number,
     * 3. A bug fix number.
     */
    class Version
    {
    private:
        u_int8_t m_major;
        u_int8_t m_minor;
        u_int8_t m_bug_fix;

    public:
        /// @brief The default constructor for a Version object.
        Version(u_int8_t bug_fix, u_int8_t minor, u_int8_t major)
        {
            this->m_major = major;
            this->m_minor = minor;
            this->m_bug_fix = bug_fix;
        }

        /// @brief A default Version class: bug fix number is set to 0.
        Version(u_int8_t minor, u_int8_t major)
        {
            Version(0, minor, major);
        }

        /// @brief A default Version class: bug fix, and minor numbers are set to 0.
        Version(u_int8_t major)
        {
            Version(0, 0, major);
        }

        /// @brief A default Version class: bug fix, minor and major numbers are set to 0.
        Version()
        {
            Version(0, 0, 0);
        }

        /// @brief Parses a string and returns a Version object.
        /// Throws an error if the string is NULL or empty, and another one if it does not match the criteria.
        /// @param from A string that represens a Version
        Version(std::string from)
        {
            if (from.empty())
            {
                throw std::invalid_argument("cannot build Version using a NULL 'from' parameter");
            }
            std::smatch matches;
            std::regex_match(from, matches, VERSION_REGEX, std::regex_constants::match_default);
            if (matches.size() < 4)
            {
                throw std::invalid_argument("expected VERSION with 3 matches");
            }
            this->m_major = 0;
            this->m_minor = 0;
            this->m_bug_fix = 0;
            for (unsigned int i = 1; i < matches.size(); i++)
            {
                if (i > 3)
                    break;
                const u_int8_t c = stoi(matches[i]);
                if (i == 1)
                    this->m_major = c;
                else if (i == 2)
                    this->m_minor = c;
                else
                    this->m_bug_fix = c;
            }
        }

        /// @brief Formats a Version class as a raw string.
        /// @param version A string to save the Version representation in it.
        void toString(char* version) const
        {
            snprintf(version, 8, "%d.%d.%d", this->m_major, this->m_minor, this->m_bug_fix);
        }
    };

} // namespace ftstd

#endif /* version_h */
