cmake_minimum_required(VERSION 3.15)
project(vcpkg-registry-tests LANGUAGES CXX)

find_package(string-utils CONFIG REQUIRED)
find_package(file-utils CONFIG REQUIRED)
find_package(crc-hash CONFIG REQUIRED)
find_package(binary-serializer CONFIG REQUIRED)
find_package(object-pool CONFIG REQUIRED)
find_package(ini-parser CONFIG REQUIRED)
find_package(simple-logger CONFIG REQUIRED)
find_package(thread-pool CONFIG REQUIRED)
find_package(uuid CONFIG REQUIRED)
find_package(base64 CONFIG REQUIRED)
find_package(json-config CONFIG REQUIRED)
find_package(cpp-httplib CONFIG REQUIRED)

add_executable(test_all
    test_all.cpp
)

target_link_libraries(test_all PRIVATE
    string-utils::string-utils
    file-utils::file-utils
    crc-hash::crc-hash
    binary-serializer::binary-serializer
    object-pool::object-pool
    ini-parser::ini-parser
    simple-logger::simple-logger
    thread-pool::thread-pool
    uuid::uuid
    base64::base64
    json-config::json-config
    cpp-httplib
)

target_compile_features(test_all PRIVATE cxx_std_20)
