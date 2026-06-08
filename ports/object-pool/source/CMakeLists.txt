cmake_minimum_required(VERSION 3.15)
project(object-pool VERSION 1.0.0 LANGUAGES CXX)

add_library(object-pool INTERFACE)
target_include_directories(object-pool INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(object-pool INTERFACE cxx_std_17)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS object-pool EXPORT object-pool-config)
install(EXPORT object-pool-config DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/object-pool)
