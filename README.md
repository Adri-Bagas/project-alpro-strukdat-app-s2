# Slint Media Player

A media player application built with C++ and [Slint](https://slint.dev) for the user interface. It utilizes LibVLC for media playback and Native File Dialog Extended (NFDe) for native file and folder selection.

This repository serves as the **Mini Project: Algoritma Pemrograman & Struktur Data** untuk komponen penilaian Teori & Praktikum Alpro serta Teori & Praktikum Struktur Data.

## Tim Pengembang (Kelompok)

- Adri Bagas Witjaksono
- Muhammad Hafizh Hanifuddin
- Muhammad Rayhan Mubarok
- Dimas Reza Nugraha
- Dany Musyaffa

## Pemenuhan Kriteria Tugas

Aplikasi ini mengimplementasikan konsep Algoritma Pemrograman dan Struktur Data secara terintegrasi:

### 1. Implementasi Pemrograman C++ (Teori & Praktikum Alpro)

- **Struct & Pointer (\*)**: Digunakan untuk merepresentasikan entitas objek node pada struktur data.
  _[Lihat di `src/utils/media_scan.hpp`](src/utils/media_scan.hpp)_

  ```cpp
  struct MediaNode {
      std::string path;
      MediaNode* next;
  };
  ```

- **Reference (&)**: Diterapkan untuk menghindari _copy_ data dan manipulasi langsung pada memori.
  _[Lihat di `src/utils/media_scan.hpp`](src/utils/media_scan.hpp)_

  ```cpp
  void init(MediaLinkedList& list);
  void pushBack(MediaLinkedList& list, const std::string& path);
  ```

- **Namespace**: Fungsi logika inti dan operasi struktur data dibungkus agar lebih rapi dan modular.
  _[Lihat di `src/utils/media_scan.hpp`](src/utils/media_scan.hpp)_

  ```cpp
  namespace MediaQueueOps {
      void init(MediaQueue& q);
      void enqueue(MediaQueue& q, const std::string& path);
      std::string dequeue(MediaQueue& q);
  }
  ```

- **Callback Function**: Digunakan dalam algoritma pencarian (search) di mana kriteria pencarian dilempar sebagai fungsi (*match_fn*).
  _[Lihat di `src/utils/search.hpp`](src/utils/search.hpp)_

  ```cpp
  template <typename T, typename MatchFn>
  std::vector<T> linear_search_if(const std::vector<T>& data, MatchFn match_fn) {
      std::vector<T> result;
      for (const auto& item : data) {
          if (match_fn(item)) { // Memanggil fungsi callback
              result.push_back(item);
          }
      }
      return result;
  }
  ```

- **Default Argument**: Digunakan pada fungsi utilitas untuk memberikan nilai _default_ apabila argumen tidak dikirim.
  _[Lihat di `src/utils/media_scan.hpp`](src/utils/media_scan.hpp)_

  ```cpp
  void init(MediaScanner& scanner, const fs::path& path = "");
  ```

- **Template**: Penggunaan secara luas pada `slint::VectorModel` untuk model UI yang dinamis.
  _[Lihat di `src/main.cpp`](src/main.cpp)_
  ```cpp
  auto mediaModel = std::make_shared<slint::VectorModel<MediaItem>>();
  ```

### 2. Implementasi Struktur Data (Teori & Praktikum Strukdat)

- **Linked List**: Digunakan sebagai _Master Data_ penyimpan direktori hasil _scan_.
  _[Lihat di `src/utils/media_scan.hpp`](src/utils/media_scan.hpp)_

  ```cpp
  struct MediaLinkedList {
      MediaNode* head;
      MediaNode* tail;
      size_t count;
  };
  ```

- **Circular Linked List**: Diimplementasikan pada logika antrean lagu saat fitur _repeat/loop_ diaktifkan, agar perpindahan dari _tail_ langsung menyambung kembali ke _head_.

- **Stack (LIFO)**: Digunakan untuk fitur **Playback History** (Lagu Sebelumnya).
  _[Lihat di `src/utils/media_scan.hpp`](src/utils/media_scan.hpp)_

  ```cpp
  struct MediaStack {
      MediaNode* top;
      size_t count;
  };
  ```

- **Queue (FIFO)**: Digunakan untuk sistem **Playback Queue** (Antrean Lagu).
  _[Lihat di `src/utils/media_scan.hpp`](src/utils/media_scan.hpp)_
  ```cpp
  struct MediaQueue {
      MediaNode* front;
      MediaNode* rear;
      size_t count;
  };
  ```

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

_Note: The project uses CMake FetchContent to automatically download and build Slint, libvlcpp, and Native File Dialog Extended._

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
5. Run the application binary
    * Linux/macOS:
        ```
        ./build/smp
        ```
    * Windows:
        ```
        build\smp.exe
        ```

## IDE Integration

We recommend using an IDE for development, along with the [LSP-based IDE integration for `.slint` files](https://github.com/slint-ui/slint/blob/master/tools/lsp/README.md). You can also load this project directly in [Visual Studio Code](https://code.visualstudio.com) and install the [Slint extension](https://marketplace.visualstudio.com/items?itemName=Slint.slint).
