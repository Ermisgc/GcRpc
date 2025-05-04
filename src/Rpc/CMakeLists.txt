aux_source_directory(. SRC_LIST)

# generate a library
add_library(gcrpc SHARED ${SRC_LIST})

include_directories(./include)