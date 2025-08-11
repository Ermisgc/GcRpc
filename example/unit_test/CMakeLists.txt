add_executable(test_logger test_logger.cc)
add_executable(myuring test_uring.cc)
add_executable(test_proto test_protobuf.cc)
add_executable(etcd_client_test test_etcd_client.cc)

target_link_libraries(test_logger gcspdlog)
target_link_libraries(myuring uring)
target_link_libraries(test_proto gcrpc protobuf)
target_link_libraries(etcd_client_test gcrpc curl)
