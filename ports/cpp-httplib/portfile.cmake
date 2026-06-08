vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO yhirose/cpp-httplib
    REF "v${VERSION}"
    SHA512 6dd1054d0171594bf871a918b5540cc67981a48f7dbb82e9029a4877afc6d8c36e2f69411f5eec7b052bf0245554d01f33ef2180f42a15f6b8dd86ebb2427a41
    HEAD_REF master
)

file(INSTALL "${SOURCE_PATH}/httplib.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")

set(SSL_DEFINE "")
if("ssl" IN_LIST FEATURES)
    set(SSL_DEFINE "target_compile_definitions(cpp-httplib INTERFACE CPPHTTPLIB_OPENSSL_SUPPORT)")
endif()

file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/${PORT}-config.cmake"
"include(CMakeFindDependencyMacro)

add_library(cpp-httplib INTERFACE IMPORTED)
target_include_directories(cpp-httplib INTERFACE \"\${CMAKE_CURRENT_LIST_DIR}/../../include\")
target_compile_features(cpp-httplib INTERFACE cxx_std_17)
${SSL_DEFINE}

if(\"ssl\" IN_LIST cpp-httplib_FIND_COMPONENTS)
    find_dependency(OpenSSL REQUIRED)
    target_link_libraries(cpp-httplib INTERFACE OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(cpp-httplib INTERFACE CPPHTTPLIB_OPENSSL_SUPPORT)
endif()
")

file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/${PORT}-config-version.cmake"
"set(PACKAGE_VERSION \"${VERSION}\")
if(PACKAGE_FIND_VERSION VERSION_GREATER PACKAGE_VERSION)
    set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
    set(PACKAGE_VERSION_COMPATIBLE TRUE)
    if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)
        set(PACKAGE_VERSION_EXACT TRUE)
    endif()
endif()
")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
