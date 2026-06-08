cmake_minimum_required(VERSION 3.15)
project(json-config VERSION 1.0.0 LANGUAGES CXX)

add_library(json-config INTERFACE)
target_include_directories(json-config INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(json-config INTERFACE cxx_std_20)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS json-config EXPORT json-config-config)
install(EXPORT json-config-config NAMESPACE json-config:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/json-config)