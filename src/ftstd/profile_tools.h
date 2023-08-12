//
//  profile.h
//  FrameTech
//
//  Created by Antonin on 12/08/2023.
//

#ifndef profile_h
#define profile_h

#include <chrono>
#include <string>

namespace ftstd
{
    namespace profile {
#ifdef PROFILE
    const uint32_t MARKER_NAME_LENGTH = 256;
    static std::map<std::string, double> s_PROFILE_MARKERS;
    class ScopedProfileMarker {
        public:
        ScopedProfileMarker(char* name = (char*)"profile_event") {
            strncpy(m_name, name, MARKER_NAME_LENGTH);
            m_begin_time = std::chrono::high_resolution_clock::now();
        }
        ~ScopedProfileMarker() {
            std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
            double time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - m_begin_time).count();
            s_PROFILE_MARKERS.insert({std::string(m_name), time_spent});
        }
        private:
            char m_name[MARKER_NAME_LENGTH];
            std::chrono::high_resolution_clock::time_point m_begin_time;
    };
#else
    class ScopedProfileMarker {
        public:
        ScopedProfileMarker(char* name = (char*)"profile_event") {
        }
        ~ScopedProfileMarker() {
        }
        private:
    };
#endif
    } // namespace profile
} // namespace ftstd

#endif /* profile_h */
