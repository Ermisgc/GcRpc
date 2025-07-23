set(SRC_LIST user_caller.cc ../login_service.cc ../user.pb.cc)
add_executable(caller ${SRC_LIST})
target_link_libraries(caller gcrpc gchipe ${Protobuf_LIBRARIES})