cmake_minimum_required(VERSION 3.15)
project(string-utils VERSION 1.0.0 LANGUAGES CXX)

add_library(string-utils INTERFACE)
target_include_directories(string-utils INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(string-utils INTERFACE cxx_std_20)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS string-utils EXPORT string-utils-config)
install(EXPORT string-utils-config NAMESPACE string-utils:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/string-utils)