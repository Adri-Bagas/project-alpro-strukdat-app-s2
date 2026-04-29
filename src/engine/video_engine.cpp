#include "video_engine.hpp"
#include <cstring>


namespace Playback {


    void init(VlcEngine& engine, int argc, const char* const* argv) {
        engine.video_width = 1280;
        engine.video_height = 720;

        // 1. Initialize VLC Engine
        engine.instance = VLC::Instance(argc, argv);
        engine.player = VLC::MediaPlayer(engine.instance);

        // 2. Set up Memory Rendering
        engine.pixel_buffer.resize(engine.video_width * engine.video_height * 4);

        // Use lambdas that capture '&engine' to handle callbacks
        engine.player.setVideoCallbacks(
            [&engine](void** planes) -> void* {
                engine.frame_mutex.lock();
                if (!engine.pixel_buffer.empty()) {
                    *planes = engine.pixel_buffer.data();
                } else {
                    *planes = nullptr;
                }
                return nullptr;
            },
            [&engine](void* picture, void* const* planes) {
                if (engine.on_frame_ready && !engine.pixel_buffer.empty()) {
                    slint::SharedPixelBuffer<slint::Rgba8Pixel> slint_buffer(engine.video_width, engine.video_height);
                    std::memcpy(slint_buffer.begin(), engine.pixel_buffer.data(), engine.pixel_buffer.size());
                    engine.on_frame_ready(slint_buffer);
                }
                engine.frame_mutex.unlock();
            },
            nullptr
        );
        
        // Use initial default format
        engine.player.setVideoFormat("RGBA", engine.video_width, engine.video_height, engine.video_width * 4);

        // 3. Attach Event Listeners
        engine.m_em = std::make_unique<VLC::MediaPlayerEventManager>(engine.player.eventManager());

        engine.m_em->onTimeChanged([&engine](int64_t new_time_ms) {
            if (engine.on_time_changed) {
                engine.on_time_changed(new_time_ms / 1000.0f);
            }
        });

        engine.m_em->onLengthChanged([&engine](int64_t new_length_ms) {
            if (engine.on_length_changed) {
                engine.on_length_changed(new_length_ms / 1000.0f);
            }
        });

        engine.m_em->onEndReached([&engine]() {
            if (engine.on_end_reached) {
                engine.on_end_reached();
            }
        });
    }

    void destroy(VlcEngine& engine) {
        engine.player.stop();
    }

    void loadFile(VlcEngine& engine, const std::string& path, unsigned width, unsigned height) {
        // Pre-configure video format before playing. This is the key to stutter-free 
        // playback while supporting native resolutions.
        if (width == 0 || height == 0) {
            // Default to 720p for audio files or files where parsing didn't yield a size
            width = 1280;
            height = 720;
        }

        engine.frame_mutex.lock();
        engine.video_width = width;
        engine.video_height = height;
        engine.pixel_buffer.resize(engine.video_width * engine.video_height * 4);
        engine.player.setVideoFormat("RGBA", engine.video_width, engine.video_height, engine.video_width * 4);
        engine.frame_mutex.unlock();

        engine.current_media = VLC::Media(engine.instance, path, VLC::Media::FromPath);
        engine.player.setMedia(engine.current_media);
    }

    void play(VlcEngine& engine) {
        engine.player.play();
    }

    void pause(VlcEngine& engine) {
        engine.player.pause();
    }

    void setTime(VlcEngine& engine, float seconds) {
        engine.player.setTime(static_cast<int64_t>(seconds * 1000));
    }

    float getLength(VlcEngine& engine) {
        return engine.player.length() / 1000.0f;
    }

    bool isPlaying(VlcEngine& engine) {
        return engine.player.isPlaying();
    }
}
