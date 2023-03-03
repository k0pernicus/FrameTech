# Get all C/CPP source files in the ftstd directory
file(GLOB_RECURSE SRC
    "*.h"
    "*.c"
)

add_library("ftstd" ${SRC})
set_target_properties("ftstd" PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features("ftstd" PRIVATE cxx_std_17)
