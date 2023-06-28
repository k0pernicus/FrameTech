//
//  taggable.hpp
//  FrameTech
//
//  Created by Antonin on 21/05/2023.
//

#pragma once
#ifndef _taggable_hpp
#define _taggable_hpp

namespace frametech
{
    namespace gameframework
    {
        /// @brief Pure abstract class for debugging purposes
        class TaggableInterface
        {
        public:
            /// @brief Sets a tag to the object instance that implements this interface
            /// @param tag A string
            virtual void setTag(const std::string& tag) noexcept;
            /// @brief Returns the tag of the instance that implements this interface
            /// @return A string
            virtual std::string& getTag() noexcept;
            /// @brief Function that implements the code for the debug UI
            virtual void debugUILogs() const noexcept;

        private:
            /// @brief Must represents the object that implements this interface, as a single string
            std::string m_tag;
        };
    } // namespace gameframework
} // namespace frametech

#endif // _taggable_hpp
