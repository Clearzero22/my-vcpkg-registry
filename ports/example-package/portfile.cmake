# Portfile for example-package
# This script defines how to download, build, and install the package

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO yourusername/example-package
    REF "v${VERSION}"
    SHA512 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
    HEAD_REF main
)

# Build the project
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTS=OFF
        -DBUILD_EXAMPLES=OFF
        -DUSE_FEATURE1=$<TARGET_EXISTS:some-other-package::some-other-package>
)

vcpkg_cmake_build()

# Install the project
vcpkg_cmake_install()

# Remove debug files that are not needed
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/example-package)

# Copy license file
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

# Handle any post-installation tasks
vcpkg_copy_pdbs()