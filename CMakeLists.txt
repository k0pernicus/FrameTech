cmake_minimum_required(VERSION 3.18.1)
project(frametech)

if(NOT CMAKE_BUILD_TYPE)
    message("Setting build type as 'Debug' - please force 'Debug' to build with debug symbols and extra symbols")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	set(BUILD_NAME "${PROJECT_NAME}_Debug")
else()
	set(BUILD_NAME ${PROJECT_NAME})
endif()

message("FrameTech build for ${CMAKE_SYSTEM_NAME}")

add_subdirectory("extern/glm")
add_subdirectory("extern/glfw")
add_subdirectory("extern/imgui")
add_subdirectory("extern/libzip-1.10.0")

# NOTE: -Wno-implicit-int-conversion is required here as stb_image has issues with
# integer / float type conversions
if(WIN32)
set(CMAKE_CXX_FLAGS_DEBUG "/std:c++17 -Wconversion -Wno-sign-conversion -Werror -Wno-c++98-compat-pedantic -Wno-c++98-compat -Wno-implicit-int-conversion -Wno-deprecated-declarations -gcodeview -DDEBUG -DIMGUI -DENABLE_EXCEPTIONS -DUNSET_FPS_LIMIT")
set(CMAKE_CXX_FLAGS_RELEASE "/std:c++17 -O2 -Wno-sign-conversion -Wno-c++98-compat-pedantic -Wno-c++98-compat -Wno-implicit-int-conversion -Wno-deprecated-declarations -DNDEBUG -O2 -DNO_AVG_FPS_RECORDS -DIMGUI")
elseif(APPLE)
set(CMAKE_CXX_FLAGS_DEBUG "--std=c++17 -Wconversion -Wno-sign-conversion -Werror -Wno-nullability-completeness -Wno-c++98-compat-pedantic -Wno-c++98-compat -Wno-implicit-int-conversion -Wno-deprecated-declarations -g -DDEBUG -DIMGUI -DENABLE_EXCEPTIONS -DUNSET_FPS_LIMIT")
set(CMAKE_CXX_FLAGS_RELEASE "--std=c++17 -O2 -Wno-sign-conversion -Wno-nullability-completeness -Wno-c++98-compat-pedantic -Wno-c++98-compat -Wno-implicit-int-conversion -Wno-deprecated-declarations -DNDEBUG -O2 -DNO_AVG_FPS_RECORDS -DIMGUI")
endif()

add_subdirectory("src/engine")
add_subdirectory("src/gameframework")

# From https://github.com/PacktPublishing/Learning-Vulkan/ CMakeLists.txt
if(AUTO_LOCATE_VULKAN AND WIN32)
	message(STATUS "Attempting auto locate Vulkan using CMake......")

	# Find Vulkan Path using CMake's Vulkan Module
	# This will return Boolean 'Vulkan_FOUND' indicating the status of find as success(ON) or fail(OFF).
	# Include directory path - 'Vulkan_INCLUDE_DIRS' and 'Vulkan_LIBRARY' with required libraries.
	find_package(Vulkan)

	# Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
	if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
		set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
		STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
	endif()

	if(NOT Vulkan_FOUND)
		# CMake may fail to locate the libraries but could be able to
		# provide some path in Vulkan SDK include directory variable
		# 'Vulkan_INCLUDE_DIRS', try to extract path from this.
		message(STATUS "Failed to locate Vulkan SDK, retrying again...")
		if(EXISTS "${VULKAN_PATH}")
			message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
		else()
			message("Error: Unable to locate Vulkan SDK. Please turn off auto locate option by specifying 'AUTO_LOCATE_VULKAN' as 'OFF'")
			message("and specify manually path using 'VULKAN_SDK' and 'VULKAN_VERSION' variables in the CMakeLists.txt.")
			return()
		endif()
	endif()
else()
	message(STATUS "Attempting to locate Vulkan SDK using manual path......")

	if(WIN32)
	set(VULKAN_SDK "C:/VulkanSDK")
	elseif(APPLE)
	set(VULKAN_SDK "$ENV{HOME}/Devel/opt/VulkanSDK")
	endif()

	set(VULKAN_VERSION "1.3.211.0")
	set(VULKAN_PATH "${VULKAN_SDK}/${VULKAN_VERSION}")
	message(STATUS "Using manual specified path: ${VULKAN_PATH}")

	# Check if manual set path exists
	if(NOT EXISTS "${VULKAN_PATH}")
		message("Error: Unable to locate this Vulkan SDK path VULKAN_PATH: ${VULKAN_PATH}, please specify correct path.
		For more information on correct installation process, please refer to subsection 'Getting started with Lunar-G SDK'
		and 'Setting up first project with CMake' in Chapter 3, 'Shaking hands with the device' in this book 'Learning Vulkan', ISBN - 9781786469809.")
	   return()
	endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Include Vulkan header files from Vulkan SDK
	message("Include Vulkan headers for Windows")
	include_directories(AFTER ${VULKAN_PATH}/Include)

	# Link directory for vulkan-1
	message("Linking Vulkan libraries for Windows")
	link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)

	set(VULKAN_1_LIB ${VULKAN_PATH}/Lib/vulkan-1.lib)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# Include Vulkan header files from Vulkan SDK
	message("Include Vulkan headers for Darwin")
	include_directories(AFTER ${VULKAN_PATH}/macOS/include)

	# Link directory for vulkan-1
	message("Linking Vulkan libraries for Darwin")
	link_directories(${VULKAN_PATH}/macOS/bin;${VULKAN_PATH}/macOS/lib;)

	set(VULKAN_1_LIB ${VULKAN_PATH}/macOS/lib/libvulkan.1.dylib)
endif()

include_directories("${CMAKE_SOURCE_DIR}/extern")
include_directories("${CMAKE_SOURCE_DIR}/extern/imgui")
include_directories("${CMAKE_SOURCE_DIR}/extern/libzip-1.10.0/lib")
include_directories("${CMAKE_SOURCE_DIR}/extern/glfw/include")
include_directories("${CMAKE_SOURCE_DIR}/extern/vma")
include_directories("${CMAKE_SOURCE_DIR}/extern/stb")
include_directories("${CMAKE_SOURCE_DIR}/extern/tomlplusplus")
include_directories("${CMAKE_SOURCE_DIR}/src/engine")
include_directories("${CMAKE_SOURCE_DIR}/src/ftstd")
include_directories("${CMAKE_SOURCE_DIR}/src/gameframework")

#frametech
add_executable(${BUILD_NAME} "src/main.cpp" "src/application.cpp" "src/application.hpp" "src/project.hpp" ${CMAKE_CURRENT_BINARY_DIR}/shaders/)
target_compile_features(${BUILD_NAME} PRIVATE cxx_std_17)
target_link_libraries(${BUILD_NAME} zip engine gameframework glm imgui glfw ${VULKAN_1_LIB})

# Copy the shaders folder, as a custom POST command
message("Copying the shaders/ folder, from ${CMAKE_SOURCE_DIR} to ${CMAKE_BINARY_DIR}")
add_custom_command(TARGET ${BUILD_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
			${CMAKE_SOURCE_DIR}/shaders/
			${CMAKE_CURRENT_BINARY_DIR}/shaders/
)
# Add to resource bundle
if(APPLE)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/shaders/ PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()
