cmake_minimum_required(VERSION 3.15)
project(tcp-client VERSION 1.0.0 LANGUAGES CXX)

add_library(tcp-client STATIC
    src/tcp_client.cpp
)
target_include_directories(tcp-client PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(tcp-client PUBLIC cxx_std_20)

if(WIN32)
    target_link_libraries(tcp-client PUBLIC ws2_32)
endif()

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS tcp-client EXPORT tcp-client-config
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(EXPORT tcp-client-config NAMESPACE tcp-client:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tcp-client)