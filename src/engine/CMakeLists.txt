# Get all C/CPP source files in the engine directory
file(GLOB_RECURSE SRC
    "*.cpp"
    "*.hpp"
)

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

add_library(engine ${SRC})

set_source_files_properties("allocator.cpp" PROPERTIES COMPILE_OPTIONS "-w") # for VMA

include_directories("${CMAKE_SOURCE_DIR}/extern")
include_directories("${CMAKE_SOURCE_DIR}/extern/glfw/include")
include_directories("${CMAKE_SOURCE_DIR}/extern/imgui")
include_directories("${CMAKE_SOURCE_DIR}/extern/vma")
include_directories("${CMAKE_SOURCE_DIR}/src/ftstd")

target_compile_features(engine PRIVATE cxx_std_17)
