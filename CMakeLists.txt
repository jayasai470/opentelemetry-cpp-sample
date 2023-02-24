cmake_minimum_required(VERSION 3.0.0)
project(otel-cpp-sample VERSION 0.1.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CTest)
enable_testing()

add_subdirectory(third-party/Simple-Web-Server)

find_package(Boost COMPONENTS log filesystem system thread REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/third-party
)

list(APPEND ${Boost_LIBRARIES})

add_executable(otel-cpp-sample main.cpp telemetry.cpp telemetry.h xray_id_generator.h xray_id_generator.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

find_package(OpenSSL)
find_package(Threads REQUIRED)

find_package(opentelemetry-cpp CONFIG REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)

# find_package(Protobuf)
# find_package(gRPC)

target_link_libraries(otel-cpp-sample ${OPENSSL_LIBRARIES})
target_include_directories(otel-cpp-sample INTERFACE ${OPENSSL_INCLUDE_DIR})

target_link_libraries(otel-cpp-sample ${Boost_LIBRARIES})
target_include_directories(otel-cpp-sample INTERFACE ${Boost_INCLUDE_DIR})


target_link_libraries(otel-cpp-sample PRIVATE ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(otel-cpp-sample 
# protobuf::libprotobuf
opentelemetry_trace 
opentelemetry_common 
# opentelemetry_exporter_otlp_grpc
# opentelemetry_exporter_otlp_grpc_log
# opentelemetry_exporter_otlp_grpc_client 
# opentelemetry_proto 
# opentelemetry_otlp_recordable 
opentelemetry_resources 
# gRPC::grpc++
opentelemetry-cpp::api
opentelemetry-cpp::sdk
opentelemetry-cpp::trace
opentelemetry-cpp::logs
opentelemetry-cpp::ostream_span_exporter
opentelemetry-cpp::ostream_log_record_exporter
opentelemetry_metrics
opentelemetry_trace
opentelemetry_exporter_prometheus
opentelemetry_logs
prometheus-cpp::pull
prometheus-cpp::core
)
