set(SRC_LIST userservice.cc ../login_service.cc ../user.pb.cc)

# the name of executable is provider
add_executable(provider ${SRC_LIST})

target_link_libraries(provider PRIVATE gcrpc gchipe ${Protobuf_LIBRARIES})  # 显式链接protobuf
