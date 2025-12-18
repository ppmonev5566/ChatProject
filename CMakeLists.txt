cmake_minimum_required(VERSION 3.16)

project(command_line_messenger CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ------------------------------------------------------------------------------
# Find packages from vcpkg
# ------------------------------------------------------------------------------
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

message(STATUS "Using Protobuf ${Protobuf_VERSION}")
message(STATUS "Using gRPC via vcpkg")

# ------------------------------------------------------------------------------
# Proto configuration
# ------------------------------------------------------------------------------
set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(PROTO_FILE "${PROTO_DIR}/messenger.proto")

set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_DIR}")

# Generated file paths
set(GENERATED_SRCS
    "${GENERATED_DIR}/messenger.pb.cc"
    "${GENERATED_DIR}/messenger.grpc.pb.cc"
)

set(GENERATED_HDRS
    "${GENERATED_DIR}/messenger.pb.h"
    "${GENERATED_DIR}/messenger.grpc.pb.h"
)

# Get protoc and grpc_cpp_plugin locations
# Protobuf's config usually defines this variable:
#   Protobuf_PROTOC_EXECUTABLE
set(GRPC_CPP_PLUGIN_EXECUTABLE
    "C:/Work/Coding/gRPC-projects/VCPKG/vcpkg/installed/x64-windows/tools/grpc/grpc_cpp_plugin.exe"
)

add_custom_command(
    OUTPUT ${GENERATED_SRCS} ${GENERATED_HDRS}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS
        --grpc_out="${GENERATED_DIR}"
        --cpp_out="${GENERATED_DIR}"
        -I "${PROTO_DIR}"
        --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${PROTO_FILE}"
    DEPENDS "${PROTO_FILE}"
    COMMENT "Generating gRPC and Protobuf sources from ${PROTO_FILE}"
)

# Group generated code into a library
add_library(messenger_protos ${GENERATED_SRCS})
target_include_directories(messenger_protos PUBLIC "${GENERATED_DIR}")

target_link_libraries(messenger_protos
    PUBLIC
        gRPC::grpc++
        protobuf::libprotobuf
)

# Server executable
add_executable(grpc_server
    src/server.cpp
)
target_link_libraries(grpc_server
    PRIVATE messenger_protos
)

# Client executable
add_executable(grpc_client
    src/client.cpp
)
target_link_libraries(grpc_client
    PRIVATE messenger_protos
)