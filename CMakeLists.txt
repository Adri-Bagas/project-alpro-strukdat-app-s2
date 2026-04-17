cmake_minimum_required(VERSION 3.21)
project(my_application LANGUAGES CXX)

# Always generate the map for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- 1. SLINT SETUP ---
find_package(Slint QUIET)
if (NOT Slint_FOUND)
  message("Slint could not be located in the CMake module search path. Downloading it from Git and building it locally")
  include(FetchContent)
  FetchContent_Declare(
    Slint
    GIT_REPOSITORY https://github.com/slint-ui/slint.git
    GIT_TAG release/1
    SOURCE_SUBDIR api/cpp
  )
  FetchContent_MakeAvailable(Slint)
endif (NOT Slint_FOUND)

# --- 2. VLC & LIBVLCPP SETUP (Added) ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(VLC REQUIRED IMPORTED_TARGET libvlc)

include(FetchContent) # Safe to call even if Slint already called it
FetchContent_Declare(
    libvlcpp
    GIT_REPOSITORY https://code.videolan.org/videolan/libvlcpp.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(libvlcpp)

# --- 3. BUILD THE APP ---
add_executable(my_application
  src/main.cpp
  src/utils/media_scan.cpp
)
slint_target_sources(my_application ui/app-window.slint)

# --- 4. FIX CLANGD AUTOCOMPLETE (Added) ---
# This forces the compiler to look in the build folder for generated headers
target_include_directories(my_application PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${libvlcpp_SOURCE_DIR})

# --- 5. LINK ALL LIBRARIES (Updated) ---
# Notice we added PkgConfig::VLC and libvlcpp here!
target_link_libraries(my_application PRIVATE Slint::Slint PkgConfig::VLC)

# On Windows, copy the Slint DLL next to the application binary so that it's found.
if (WIN32)
    add_custom_command(TARGET my_application POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:my_application> $<TARGET_FILE_DIR:my_application> COMMAND_EXPAND_LISTS)
endif()