# Get all C/CPP source files in the vma directory
file(GLOB SRC *.h)

set(CMAKE_CXX_FLAGS_DEBUG "-Wno-sign-conversion")
set(CMAKE_CXX_FLAGS_RELEASE "-Wno-sign-conversion")

# if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
# 	# Include Vulkan header files from Vulkan SDK
# 	include_directories(AFTER ${VULKAN_PATH}/Include)

# 	# Link directory for vulkan-1
# 	link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)
# endif()

# set_target_properties(vma PROPERTIES LINKER_LANGUAGE CXX)
# add_library(vma ${SRC})
