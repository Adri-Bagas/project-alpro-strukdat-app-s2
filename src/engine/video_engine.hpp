#pragma once

#include <vlcpp/vlc.hpp>
#include <slint.h>
#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <memory>

// --- DATA STRUCTURE (Pure Data) ---

struct VlcEngine {
    VLC::Instance instance;
    VLC::MediaPlayer player;
    VLC::Media current_media;
    std::unique_ptr<VLC::MediaPlayerEventManager> m_em;

    unsigned int video_width;
    unsigned int video_height;
    std::vector<uint8_t> pixel_buffer;
    std::mutex frame_mutex;

    // Callbacks
    std::function<void(float)> on_time_changed;
    std::function<void(float)> on_length_changed;
    std::function<void()> on_end_reached;
    std::function<void(slint::SharedPixelBuffer<slint::Rgba8Pixel>)> on_frame_ready;
};

// --- LOGIC NAMESPACE ---

namespace Playback {
    void init(VlcEngine& engine);
    void destroy(VlcEngine& engine);
    void loadFile(VlcEngine& engine, const std::string& path, unsigned width, unsigned height);
    void play(VlcEngine& engine);
    void pause(VlcEngine& engine);
    void setTime(VlcEngine& engine, float seconds);
    float getLength(VlcEngine& engine);
    bool isPlaying(VlcEngine& engine);
}
