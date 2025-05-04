set(SRC_LIST userservice.cc login_service.cc ../test.pb.cc)

# the name of executable is provider
add_executable(provider ${SRC_LIST})

target_link_libraries(provider gcrpc protobuf)

