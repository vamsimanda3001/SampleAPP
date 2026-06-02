cmake_minimum_required(VERSION 3.20)

project(RemoteDesktopProviderSample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SYSTEM_VERSION "10.0")

# ── Locate Windows SDK cppwinrt.exe ──────────────────────────────────────────
# The Windows 10/11 SDK ships cppwinrt.exe. We use it to generate projection
# headers into the build tree so #include <winrt/...> works.
find_program(CPPWINRT_EXE cppwinrt
    PATHS "C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x64"
          "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22621.0/x64"
)

if(NOT CPPWINRT_EXE)
    message(FATAL_ERROR "cppwinrt.exe not found in Windows SDK. Install the Windows 10/11 SDK.")
endif()

# Generate C++/WinRT projection headers
set(CPPWINRT_OUTPUT_DIR "${CMAKE_BINARY_DIR}/cppwinrt")
file(MAKE_DIRECTORY "${CPPWINRT_OUTPUT_DIR}")

add_custom_command(
    OUTPUT "${CPPWINRT_OUTPUT_DIR}/winrt/base.h"
    COMMAND "${CPPWINRT_EXE}" -input sdk -output "${CPPWINRT_OUTPUT_DIR}"
    COMMENT "Generating C++/WinRT projection headers..."
)
add_custom_target(cppwinrt_projection DEPENDS "${CPPWINRT_OUTPUT_DIR}/winrt/base.h")

# ── Application target ──────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    src/app.rc
)

add_dependencies(${PROJECT_NAME} cppwinrt_projection)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        "${CPPWINRT_OUTPUT_DIR}"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        windowsapp
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        UNICODE
        _UNICODE
        WINRT_LEAN_AND_MEAN
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        /EHsc
)

# Disable linker-generated manifest — we embed our own via app.rc
# (contains the <msix> element needed for sparse package identity)
target_link_options(${PROJECT_NAME}
    PRIVATE
        /MANIFEST:NO
)

# ── Copy sparse package files to build output ────────────────────────────────
# The AppxManifest.xml and Assets must live alongside the exe for sparse
# package registration to work (InstallLocation must match the exe path).
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/sparse/AppxManifest.xml"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/AppxManifest.xml"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/sparse/Assets/StoreLogo.png"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets/StoreLogo.png"
    COMMENT "Copying sparse package manifest and assets to output directory..."
)
