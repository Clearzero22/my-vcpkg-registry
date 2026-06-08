cmake_minimum_required(VERSION 3.15)
project(ini-parser VERSION 1.0.0 LANGUAGES CXX)

add_library(ini-parser INTERFACE)
target_include_directories(ini-parser INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(ini-parser INTERFACE cxx_std_20)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS ini-parser EXPORT ini-parser-config)
install(EXPORT ini-parser-config NAMESPACE ini-parser:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ini-parser)