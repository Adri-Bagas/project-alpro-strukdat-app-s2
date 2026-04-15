#pragma once

#include <vlcpp/vlc.hpp>
#include <slint.h>
#include <string>
#include <vector>
#include <mutex>
#include <functional>

class VlcEngine {
public:
    VlcEngine();
    ~VlcEngine();

    // Core Controls
    void load_file(const std::string& path);
    void play();
    void pause();
    void set_time(float seconds);
    bool is_playing();

    // Callbacks to send data back to main.cpp
    std::function<void(float)> on_time_changed;
    std::function<void(float)> on_length_changed;
    std::function<void(slint::SharedPixelBuffer<slint::Rgba8Pixel>)> on_frame_ready;

    // Public memory for the C-callbacks to access
    std::vector<uint8_t> pixel_buffer;
    std::mutex frame_mutex;
    unsigned int video_width;
    unsigned int video_height;

private:
    VLC::Instance instance;
    VLC::MediaPlayer player;
    VLC::Media current_media;
};