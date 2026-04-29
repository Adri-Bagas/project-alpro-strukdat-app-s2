# Slint Media Player

A media player application built with C++ and [Slint](https://slint.dev) for the user interface. It utilizes LibVLC for media playback and Native File Dialog Extended (NFDe) for native file and folder selection.

## Features

- **Media Library Scanning**: Scan folders for media files and automatically extract metadata.
- **Playback Control**: Play, pause, stop, and seek tracks.
- **Queue Management**: Add to queue, remove, reorder, and jump to tracks.
- **Sorting**: Sort your media library by name, artist, duration, or size.
- **Video Support**: Seamlessly switch between audio and video views with full-screen support.

## Prerequisites

In order to build and run this application, you need to install a few tools:

- **[CMake](https://cmake.org/download/)** (3.21 or newer)
- A C++ compiler that supports **C++ 20**
- **libvlc**: 
  - For Windows users, please download libvlc manually from:
    [https://www.nuget.org/api/v2/package/VideoLAN.LibVLC.Windows/3.0.23.1](https://www.nuget.org/api/v2/package/VideoLAN.LibVLC.Windows/3.0.23.1)
  - Change the downloaded `.nupkg` extension to `.zip` and extract it. 
  - Copy the contents of `build\x64` to `C:/libvlc` so that `C:/libvlc/include` and `C:/libvlc/libvlc.lib` exist.
  - For Linux/macOS users, libvlc can be installed via your system's package manager.

*Note: The project uses CMake FetchContent to automatically download and build Slint, libvlcpp, and Native File Dialog Extended.*

## Usage

1. Clone or download this repository.
2. Change into the project directory:
   ```sh
   cd project-alpro-strukdat-app-s2
   ```
3. Configure with CMake:
   ```sh
   mkdir build
   cmake -B build
   ```
4. Build with CMake:
   ```sh
   cmake --build build
   ```
5. Run the application:
   - **Linux/macOS**:
     ```sh
     ./build/my_application
     ```
   - **Windows**:
     ```sh
     build\my_application.exe
     ```

## IDE Integration

We recommend using an IDE for development, along with the [LSP-based IDE integration for `.slint` files](https://github.com/slint-ui/slint/blob/master/tools/lsp/README.md). You can also load this project directly in [Visual Studio Code](https://code.visualstudio.com) and install the [Slint extension](https://marketplace.visualstudio.com/items?itemName=Slint.slint).
