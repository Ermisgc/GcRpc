add_executable(test_logger test_logger.cc)
add_executable(test_queue test_queue.cc)
add_executable(myuring test_uring.cc)

target_link_libraries(test_logger gcspdlog)
target_link_libraries(test_queue gcrpc)
target_link_libraries(myuring uring)