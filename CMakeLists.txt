cmake_minimum_required(VERSION 3.0.0)
project(otel-cpp-sample VERSION 0.1.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CTest)
enable_testing()

add_subdirectory(third-party/Simple-Web-Server)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS log filesystem REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/third-party
)

list(APPEND ${Boost_LIBRARIES})

add_executable(otel-cpp-sample main.cpp telemetry.cpp telemetry.h)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

find_package(Threads REQUIRED)

find_package(opentelemetry-cpp CONFIG REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)

target_link_libraries(otel-cpp-sample PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(otel-cpp-sample opentelemetry-cpp::api opentelemetry-cpp::sdk opentelemetry-cpp::trace opentelemetry-cpp::ostream_span_exporter opentelemetry_metrics opentelemetry_exporter_prometheus opentelemetry_resources prometheus-cpp::pull prometheus-cpp::core)