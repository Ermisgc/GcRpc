# aux_source_directory(. SRC_LIST)
set(SRC_LIST formatter.cc logger.cc registry.cc sink.cc)

add_library(gcspdlog SHARED ${SRC_LIST})