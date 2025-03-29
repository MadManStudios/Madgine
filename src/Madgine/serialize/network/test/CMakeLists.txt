
project(NetworkSerialize_Test)

set(sources networkmanagertest.cpp ../../../../../test/Meta/Meta_gtest_main.cc)

add_gtest(NetworkSerialize_Test ${sources})

target_link_libraries(NetworkSerialize_Test PRIVATE NetworkSerialize)