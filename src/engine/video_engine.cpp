#include "video_engine.hpp"
#include <cstring>

// --- THE C CALLBACKS (Hidden inside the .cpp file) ---
extern "C" {
    void* lock_frame(void* opaque, void** planes) {
        // Cast the opaque pointer back to our C++ VlcEngine class
        auto* engine = static_cast<VlcEngine*>(opaque);
        
        engine->frame_mutex.lock();
        *planes = engine->pixel_buffer.data();
        return nullptr;
    }

    void unlock_frame(void* opaque, void* picture, void* const* planes) {
        auto* engine = static_cast<VlcEngine*>(opaque);
        
        // Only process the frame if main.cpp has actually connected the callback
        if (engine->on_frame_ready) {
            slint::SharedPixelBuffer<slint::Rgba8Pixel> slint_buffer(engine->video_width, engine->video_height);
            std::memcpy(slint_buffer.begin(), engine->pixel_buffer.data(), engine->pixel_buffer.size());
            
            // Fire the callback to send the pixels to main.cpp!
            engine->on_frame_ready(slint_buffer);
        }
        
        engine->frame_mutex.unlock();
    }
}

// --- C++ CLASS IMPLEMENTATION ---

VlcEngine::VlcEngine() 
    : video_width(1280), video_height(720) 
{
    // 1. Initialize VLC Engine
    instance = VLC::Instance(0, nullptr);
    player = VLC::MediaPlayer(instance);

    // 2. Set up Memory Rendering
    pixel_buffer.resize(video_width * video_height * 4);
    
    // We pass 'this' as the final argument so the C-callbacks can find our class
    player.setVideoCallbacks(lock_frame, unlock_frame, nullptr);
    player.setVideoFormat("RGBA", video_width, video_height, video_width * 4);

    // 3. Attach Event Listeners
    auto em = player.eventManager();
    
    em.onTimeChanged([this](int64_t new_time_ms) {
        if (on_time_changed) {
            on_time_changed(new_time_ms / 1000.0f);
        }
    });

    em.onLengthChanged([this](int64_t new_length_ms) {
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

bool VlcEngine::is_playing() {
    return player.isPlaying();
}