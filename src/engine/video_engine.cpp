#include "video_engine.hpp"
#include <cstring>

// --- C++ CLASS IMPLEMENTATION ---

VlcEngine::VlcEngine() 
    : video_width(1280), video_height(720) 
{
    // 1. Initialize VLC Engine
    instance = VLC::Instance(0, nullptr);
    player = VLC::MediaPlayer(instance);

    // 2. Set up Memory Rendering
    pixel_buffer.resize(video_width * video_height * 4);

    // Use lambdas that capture 'this' to handle callbacks in a C++ way
    player.setVideoCallbacks(
        [this](void** planes) -> void* {
            this->frame_mutex.lock();
            *planes = this->pixel_buffer.data();
            return nullptr;
        },
        [this](void* picture, void* const* planes) {
            if (this->on_frame_ready) {
                slint::SharedPixelBuffer<slint::Rgba8Pixel> slint_buffer(this->video_width, this->video_height);
                std::memcpy(slint_buffer.begin(), this->pixel_buffer.data(), this->pixel_buffer.size());

                // Fire the callback to send the pixels to main.cpp!
                this->on_frame_ready(slint_buffer);
            }
            this->frame_mutex.unlock();
        },
        nullptr
    );
    player.setVideoFormat("RGBA", video_width, video_height, video_width * 4);

    // 3. Attach Event Listeners
    m_em = std::make_unique<VLC::MediaPlayerEventManager>(player.eventManager());

    m_em->onTimeChanged([this](int64_t new_time_ms) {
        if (on_time_changed) {
            on_time_changed(new_time_ms / 1000.0f);
        }
    });

    m_em->onLengthChanged([this](int64_t new_length_ms) {
        if (on_length_changed) {
            on_length_changed(new_length_ms / 1000.0f);
        }
    });
}

VlcEngine::~VlcEngine() {
    player.stop();
    // libvlcpp automatically handles the rest of the cleanup!
}

void VlcEngine::load_file(const std::string& path) {
    current_media = VLC::Media(instance, path, VLC::Media::FromPath);
    player.setMedia(current_media);
}

void VlcEngine::play() {
    player.play();
}

void VlcEngine::pause() {
    player.pause();
}

void VlcEngine::set_time(float seconds) {
    player.setTime(static_cast<int64_t>(seconds * 1000));
}

float VlcEngine::get_length() {
    return player.length() / 1000.0f;
}

bool VlcEngine::is_playing() {
    return player.isPlaying();
}