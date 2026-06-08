cmake_minimum_required(VERSION 3.15)
project(base64 VERSION 1.0.0 LANGUAGES CXX)

add_library(base64 INTERFACE)
target_include_directories(base64 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(base64 INTERFACE cxx_std_17)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS base64 EXPORT base64-config)
install(EXPORT base64-config DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/base64)
