cmake_minimum_required(VERSION 3.15)
project(thread-pool VERSION 1.0.0 LANGUAGES CXX)

add_library(thread-pool INTERFACE)
target_include_directories(thread-pool INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(thread-pool INTERFACE cxx_std_20)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS thread-pool EXPORT thread-pool-config)
install(EXPORT thread-pool-config NAMESPACE thread-pool:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/thread-pool)