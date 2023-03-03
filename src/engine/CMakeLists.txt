# Get all C/CPP source files in the engine directory
file(GLOB_RECURSE SRC
    "*.cpp"
    "*.hpp"
)

add_library("engine" ${SRC})
include_directories("extern/glm")
include_directories("extern/imgui")
include_directories("extern/vma")
include_directories("src/ftstd")
target_compile_features("engine" PRIVATE cxx_std_17)
