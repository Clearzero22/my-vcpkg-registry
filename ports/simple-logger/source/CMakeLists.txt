cmake_minimum_required(VERSION 3.15)
project(simple-logger VERSION 1.0.0 LANGUAGES CXX)

add_library(simple-logger STATIC
    src/logger.cpp
)
target_include_directories(simple-logger PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(simple-logger PUBLIC cxx_std_17)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS simple-logger EXPORT simple-logger-config
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(EXPORT simple-logger-config DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/simple-logger)
