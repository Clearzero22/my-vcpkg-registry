cmake_minimum_required(VERSION 3.15)
project(uuid VERSION 1.0.0 LANGUAGES CXX)

add_library(uuid INTERFACE)
target_include_directories(uuid INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(uuid INTERFACE cxx_std_17)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS uuid EXPORT uuid-config)
install(EXPORT uuid-config DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/uuid)
