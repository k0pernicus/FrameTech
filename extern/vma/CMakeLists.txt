# Get all C/CPP source files in the vma directory
file(GLOB_RECURSE SRC
    "*.h"
)

add_library(vma ${SRC})
set_target_properties(vma PROPERTIES LINKER_LANGUAGE CXX)
include_directories(".")
target_compile_features(vma PRIVATE cxx_std_17)
