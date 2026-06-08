cmake_minimum_required(VERSION 3.15)
project(crc-hash VERSION 1.0.0 LANGUAGES CXX)

add_library(crc-hash INTERFACE)
target_include_directories(crc-hash INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(crc-hash INTERFACE cxx_std_17)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS crc-hash EXPORT crc-hash-config)
install(EXPORT crc-hash-config DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/crc-hash)
