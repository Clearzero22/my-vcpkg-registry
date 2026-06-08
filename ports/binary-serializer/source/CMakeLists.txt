cmake_minimum_required(VERSION 3.15)
project(binary-serializer VERSION 1.0.0 LANGUAGES CXX)

add_library(binary-serializer INTERFACE)
target_include_directories(binary-serializer INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(binary-serializer INTERFACE cxx_std_17)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS binary-serializer EXPORT binary-serializer-config)
install(EXPORT binary-serializer-config DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/binary-serializer)
