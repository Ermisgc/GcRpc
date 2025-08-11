aux_source_directory(. SRC_LIST)

# generate a library
add_library(gcrpc SHARED ${SRC_LIST})

target_link_libraries(gcrpc PUBLIC uring gchipe ${Protobuf_LIBRARIES} ${ETCD_LIB} curl)

include_directories(./include)