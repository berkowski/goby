add_executable(goby_test_zero_mq_node4 test.cpp)
target_link_libraries(goby_test_zero_mq_node4 goby_pb)

add_test(goby_test_zero_mq_node4 ${goby_BIN_DIR}/goby_test_zero_mq_node4)

