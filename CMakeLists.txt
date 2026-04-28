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
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(VLC QUIET IMPORTED_TARGET libvlc)
endif()

if(NOT VLC_FOUND)
    find_path(VLC_INCLUDE_DIR NAMES vlc/vlc.h HINTS "C:/libvlc/include")
    find_library(VLC_LIBRARY NAMES vlc libvlc HINTS "C:/libvlc")
    
    if(VLC_INCLUDE_DIR AND VLC_LIBRARY)
        add_library(PkgConfig::VLC UNKNOWN IMPORTED)
        set_target_properties(PkgConfig::VLC PROPERTIES
            IMPORTED_LOCATION "${VLC_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${VLC_INCLUDE_DIR}"
        )
        set(VLC_FOUND TRUE)
        message(STATUS "Found libvlc manually at ${VLC_LIBRARY}")
    else()
        message(FATAL_ERROR "Could not find libvlc. Please ensure it is in C:/libvlc")
    endif()
endif()

include(FetchContent) # Safe to call even if Slint already called it
FetchContent_Declare(
    libvlcpp
    GIT_REPOSITORY https://code.videolan.org/videolan/libvlcpp.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(libvlcpp)

# --- 2.5 NFD SETUP ---
FetchContent_Declare(
    nfd
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(nfd)

# --- 3. BUILD THE APP ---
add_executable(my_application
  src/main.cpp
  src/utils/media_scan.cpp
  src/engine/video_engine.cpp
  src/utils/metadata_extract.cpp
  src/utils/file_dialog.cpp
)
slint_target_sources(my_application ui/app-window.slint)

# --- 4. FIX CLANGD AUTOCOMPLETE (Added) ---
# This forces the compiler to look in the build folder for generated headers
target_include_directories(my_application PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${libvlcpp_SOURCE_DIR})

# --- 5. LINK ALL LIBRARIES (Updated) ---
# Notice we added PkgConfig::VLC and libvlcpp here!
target_link_libraries(my_application PRIVATE Slint::Slint PkgConfig::VLC nfd)

# On Windows, copy the Slint DLL next to the application binary so that it's found.
if (WIN32)
    add_custom_command(TARGET my_application POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:my_application> $<TARGET_FILE_DIR:my_application> COMMAND_EXPAND_LISTS)
endif()