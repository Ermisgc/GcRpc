# Details

Date : 2025-08-11 17:13:42

Directory /home/ermis/vs_project/GcRPC

Total : 88 files,  29968 codes, 7780 comments, 5471 blanks, all 43219 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [CMakeLists.txt](/CMakeLists.txt) | CMake | 31 | 0 | 13 | 44 |
| [README.md](/README.md) | Markdown | 35 | 0 | 22 | 57 |
| [autobuild.sh](/autobuild.sh) | Shell Script | 5 | 0 | 1 | 6 |
| [example/CMakeLists.txt](/example/CMakeLists.txt) | CMake | 3 | 0 | 0 | 3 |
| [example/callee/CMakeLists.txt](/example/callee/CMakeLists.txt) | CMake | 4 | 0 | 3 | 7 |
| [example/callee/userservice.cc](/example/callee/userservice.cc) | C++ | 12 | 3 | 4 | 19 |
| [example/caller/CMakeLists.txt](/example/caller/CMakeLists.txt) | CMake | 3 | 0 | 0 | 3 |
| [example/caller/user\_caller.cc](/example/caller/user_caller.cc) | C++ | 18 | 0 | 2 | 20 |
| [example/login\_service.cc](/example/login_service.cc) | C++ | 21 | 8 | 3 | 32 |
| [example/login\_service.h](/example/login_service.h) | C++ | 15 | 0 | 2 | 17 |
| [example/unit\_test/CMakeLists.txt](/example/unit_test/CMakeLists.txt) | CMake | 8 | 0 | 2 | 10 |
| [example/unit\_test/test\_etcd\_client.cc](/example/unit_test/test_etcd_client.cc) | C++ | 0 | 12 | 3 | 15 |
| [example/unit\_test/test\_logger.cc](/example/unit_test/test_logger.cc) | C++ | 8 | 4 | 2 | 14 |
| [example/unit\_test/test\_protobuf.cc](/example/unit_test/test_protobuf.cc) | C++ | 21 | 0 | 4 | 25 |
| [example/unit\_test/test\_uring.cc](/example/unit_test/test_uring.cc) | C++ | 95 | 1 | 17 | 113 |
| [example/user.pb.cc](/example/user.pb.cc) | C++ | 765 | 61 | 107 | 933 |
| [example/user.pb.h](/example/user.pb.h) | C++ | 752 | 70 | 119 | 941 |
| [include/Rpc/buffer.h](/include/Rpc/buffer.h) | C++ | 35 | 51 | 17 | 103 |
| [include/Rpc/channel.h](/include/Rpc/channel.h) | C++ | 28 | 6 | 7 | 41 |
| [include/Rpc/connection\_pool.h](/include/Rpc/connection_pool.h) | C++ | 76 | 37 | 44 | 157 |
| [include/Rpc/encoding.h](/include/Rpc/encoding.h) | C++ | 34 | 3 | 8 | 45 |
| [include/Rpc/etcd\_client.h](/include/Rpc/etcd_client.h) | C++ | 41 | 7 | 20 | 68 |
| [include/Rpc/gcrpcapplication.h](/include/Rpc/gcrpcapplication.h) | C++ | 19 | 0 | 6 | 25 |
| [include/Rpc/gcrpcconfig.h](/include/Rpc/gcrpcconfig.h) | C++ | 12 | 1 | 5 | 18 |
| [include/Rpc/generic\_rpc.pb.h](/include/Rpc/generic_rpc.pb.h) | C++ | 300 | 32 | 48 | 380 |
| [include/Rpc/load\_balancer.h](/include/Rpc/load_balancer.h) | C++ | 73 | 3 | 11 | 87 |
| [include/Rpc/net\_base.h](/include/Rpc/net_base.h) | C++ | 57 | 17 | 16 | 90 |
| [include/Rpc/pb\_encoding.h](/include/Rpc/pb_encoding.h) | C++ | 0 | 25 | 5 | 30 |
| [include/Rpc/rpc\_caller.h](/include/Rpc/rpc_caller.h) | C++ | 42 | 10 | 13 | 65 |
| [include/Rpc/rpc\_node\_info.pb.h](/include/Rpc/rpc_node_info.pb.h) | C++ | 316 | 38 | 49 | 403 |
| [include/Rpc/rpc\_protocal.h](/include/Rpc/rpc_protocal.h) | C++ | 39 | 17 | 15 | 71 |
| [include/Rpc/rpcprovider.h](/include/Rpc/rpcprovider.h) | C++ | 57 | 8 | 20 | 85 |
| [include/Rpc/service\_register.h](/include/Rpc/service_register.h) | C++ | 29 | 3 | 12 | 44 |
| [include/Rpc/service\_seeker.h](/include/Rpc/service_seeker.h) | C++ | 35 | 4 | 13 | 52 |
| [include/data\_structure/concurrentqueue.h](/include/data_structure/concurrentqueue.h) | C++ | 2,719 | 577 | 453 | 3,749 |
| [include/data\_structure/hashheap.hpp](/include/data_structure/hashheap.hpp) | C++ | 82 | 14 | 20 | 116 |
| [include/data\_structure/lockfreequeue.h](/include/data_structure/lockfreequeue.h) | C++ | 120 | 1 | 13 | 134 |
| [include/data\_structure/lru\_cache.h](/include/data_structure/lru_cache.h) | C++ | 125 | 3 | 14 | 142 |
| [include/data\_structure/rbtree.h](/include/data_structure/rbtree.h) | C++ | 332 | 24 | 40 | 396 |
| [include/data\_structure/skiplist.h](/include/data_structure/skiplist.h) | C++ | 113 | 8 | 17 | 138 |
| [include/hipe/dynamic\_threadpool.h](/include/hipe/dynamic_threadpool.h) | C++ | 74 | 3 | 13 | 90 |
| [include/hipe/hipe\_task.h](/include/hipe/hipe_task.h) | C++ | 48 | 4 | 10 | 62 |
| [include/logger/formatter.h](/include/logger/formatter.h) | C++ | 38 | 2 | 3 | 43 |
| [include/logger/log\_base.h](/include/logger/log_base.h) | C++ | 50 | 3 | 6 | 59 |
| [include/logger/logger.h](/include/logger/logger.h) | C++ | 39 | 0 | 4 | 43 |
| [include/logger/registry.h](/include/logger/registry.h) | C++ | 15 | 0 | 2 | 17 |
| [include/logger/sink.h](/include/logger/sink.h) | C++ | 27 | 2 | 4 | 33 |
| [include/thirdParty/curl/curl.h](/include/thirdParty/curl/curl.h) | C++ | 1,554 | 1,220 | 573 | 3,347 |
| [include/thirdParty/curl/curlver.h](/include/thirdParty/curl/curlver.h) | C++ | 13 | 58 | 8 | 79 |
| [include/thirdParty/curl/easy.h](/include/thirdParty/curl/easy.h) | C++ | 28 | 84 | 14 | 126 |
| [include/thirdParty/curl/header.h](/include/thirdParty/curl/header.h) | C++ | 42 | 24 | 9 | 75 |
| [include/thirdParty/curl/mprintf.h](/include/thirdParty/curl/mprintf.h) | C++ | 55 | 23 | 8 | 86 |
| [include/thirdParty/curl/multi.h](/include/thirdParty/curl/multi.h) | C++ | 166 | 294 | 66 | 526 |
| [include/thirdParty/curl/options.h](/include/thirdParty/curl/options.h) | C++ | 33 | 28 | 10 | 71 |
| [include/thirdParty/curl/stdcheaders.h](/include/thirdParty/curl/stdcheaders.h) | C++ | 8 | 23 | 5 | 36 |
| [include/thirdParty/curl/system.h](/include/thirdParty/curl/system.h) | C++ | 314 | 63 | 32 | 409 |
| [include/thirdParty/curl/typecheck-gcc.h](/include/thirdParty/curl/typecheck-gcc.h) | C++ | 681 | 118 | 69 | 868 |
| [include/thirdParty/curl/urlapi.h](/include/thirdParty/curl/urlapi.h) | C++ | 83 | 58 | 15 | 156 |
| [include/thirdParty/curl/websockets.h](/include/thirdParty/curl/websockets.h) | C++ | 33 | 42 | 11 | 86 |
| [include/thirdParty/nlohmann/json.hpp](/include/thirdParty/nlohmann/json.hpp) | C++ | 18,099 | 4,516 | 3,063 | 25,678 |
| [md\_resource/事件循环.svg](/md_resource/%E4%BA%8B%E4%BB%B6%E5%BE%AA%E7%8E%AF.svg) | XML | 3 | 0 | 0 | 3 |
| [protos/generic\_rpc.proto](/protos/generic_rpc.proto) | Protocol Buffers | 7 | 0 | 2 | 9 |
| [protos/rpc\_node\_info.proto](/protos/rpc_node_info.proto) | Protocol Buffers | 10 | 1 | 2 | 13 |
| [protos/user.proto](/protos/user.proto) | Protocol Buffers | 18 | 0 | 4 | 22 |
| [src/CMakeLists.txt](/src/CMakeLists.txt) | CMake | 3 | 0 | 0 | 3 |
| [src/Rpc/CMakeLists.txt](/src/Rpc/CMakeLists.txt) | CMake | 5 | 0 | 3 | 8 |
| [src/Rpc/buffer.cc](/src/Rpc/buffer.cc) | C++ | 94 | 11 | 18 | 123 |
| [src/Rpc/channel.cc](/src/Rpc/channel.cc) | C++ | 3 | 9 | 2 | 14 |
| [src/Rpc/connection\_pool.cc](/src/Rpc/connection_pool.cc) | C++ | 215 | 18 | 53 | 286 |
| [src/Rpc/etcd\_client.cc](/src/Rpc/etcd_client.cc) | C++ | 115 | 5 | 25 | 145 |
| [src/Rpc/gcrpcapplication.cc](/src/Rpc/gcrpcapplication.cc) | C++ | 44 | 3 | 9 | 56 |
| [src/Rpc/gcrpcconfig.cc](/src/Rpc/gcrpcconfig.cc) | C++ | 38 | 3 | 7 | 48 |
| [src/Rpc/generic\_rpc.pb.cc](/src/Rpc/generic_rpc.pb.cc) | C++ | 296 | 27 | 43 | 366 |
| [src/Rpc/load\_balancer.cc](/src/Rpc/load_balancer.cc) | C++ | 17 | 1 | 3 | 21 |
| [src/Rpc/net\_base.cc](/src/Rpc/net_base.cc) | C++ | 139 | 9 | 27 | 175 |
| [src/Rpc/rpc\_caller.cc](/src/Rpc/rpc_caller.cc) | C++ | 50 | 2 | 5 | 57 |
| [src/Rpc/rpc\_node\_info.pb.cc](/src/Rpc/rpc_node_info.pb.cc) | C++ | 349 | 36 | 49 | 434 |
| [src/Rpc/rpc\_protocal.cc](/src/Rpc/rpc_protocal.cc) | C++ | 142 | 4 | 18 | 164 |
| [src/Rpc/rpcprovider.cc](/src/Rpc/rpcprovider.cc) | C++ | 130 | 17 | 34 | 181 |
| [src/Rpc/service\_register.cc](/src/Rpc/service_register.cc) | C++ | 57 | 2 | 13 | 72 |
| [src/Rpc/service\_seeker.cc](/src/Rpc/service_seeker.cc) | C++ | 116 | 9 | 19 | 144 |
| [src/hipe/CMakeLists.txt](/src/hipe/CMakeLists.txt) | CMake | 2 | 0 | 1 | 3 |
| [src/hipe/dynamic\_threadpool.cc](/src/hipe/dynamic_threadpool.cc) | C++ | 62 | 5 | 10 | 77 |
| [src/logger/CMakeLists.txt](/src/logger/CMakeLists.txt) | CMake | 3 | 0 | 1 | 4 |
| [src/logger/formatter.cc](/src/logger/formatter.cc) | C++ | 109 | 3 | 12 | 124 |
| [src/logger/logger.cc](/src/logger/logger.cc) | C++ | 29 | 2 | 5 | 36 |
| [src/logger/registry.cc](/src/logger/registry.cc) | C++ | 0 | 0 | 1 | 1 |
| [src/logger/sink.cc](/src/logger/sink.cc) | C++ | 32 | 0 | 10 | 42 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)