#include "video_engine.hpp"
#include "app-window.h"
#include <cstring>

namespace Playback {

    void init(VlcEngine& engine) {
        engine.video_width = 1280;
        engine.video_height = 720;

        // 1. Initialize VLC Engine
        engine.instance = VLC::Instance(0, nullptr);
        engine.player = VLC::MediaPlayer(engine.instance);

        // 2. Set up Memory Rendering
        engine.pixel_buffer.resize(engine.video_width * engine.video_height * 4);

        // Use lambdas that capture '&engine' to handle callbacks
        engine.player.setVideoCallbacks(
            [&engine](void** planes) -> void* {
                engine.frame_mutex.lock();
                *planes = engine.pixel_buffer.data();
                return nullptr;
            },
            [&engine](void* picture, void* const* planes) {
                if (engine.on_frame_ready) {
                    slint::SharedPixelBuffer<slint::Rgba8Pixel> slint_buffer(engine.video_width, engine.video_height);
                    
                    // Copy from VLC buffer to Slint buffer
                    // VLC's RV32 is typically compatible with Slint's RGBA8 on standard systems
                    std::memcpy(slint_buffer.begin(), engine.pixel_buffer.data(), engine.pixel_buffer.size());
                    
                    engine.on_frame_ready(slint_buffer);
                }
                engine.frame_mutex.unlock();
            },
            nullptr
        );
        engine.player.setVideoFormat("RGBA", engine.video_width, engine.video_height, engine.video_width * 4);

        // 3. Attach Event Listeners
        engine.m_em = std::make_unique<VLC::MediaPlayerEventManager>(engine.player.eventManager());

        engine.m_em->onTimeChanged([&engine](int64_t new_time_ms) {
            if (engine.on_time_changed) {
                engine.on_time_changed(new_time_ms / 1000.0f);
            }
        });

        engine.m_em->onLengthChanged([&engine](int64_t new_length_ms) {

            update_resolution(engine);

            if (engine.on_length_changed) {
                engine.on_length_changed(new_length_ms / 1000.0f);
            }
        });
    }

    void destroy(VlcEngine& engine) {
        engine.player.stop();
    }

    void loadFile(VlcEngine& engine, const std::string& path) {
        engine.current_media = VLC::Media(engine.instance, path, VLC::Media::FromPath);
        engine.player.setMedia(engine.current_media);
    }

    void loadFile(VlcEngine& engine, const std::string& path, const MediaItem& item) {



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

    void update_resolution(VlcEngine& engine) {
    unsigned int width = 0;
    unsigned int height = 0;

    // libvlc_video_get_size mengembalikan 0 jika berhasil mendapatkan resolusi
    if (libvlc_video_get_size(engine.player, 0, &width, &height) == 0) {
        // Cek apakah ukurannya valid dan berbeda dari ukuran bawaan (1280x720)
        if (width > 0 && height > 0 && (width != engine.video_width || height != engine.video_height)) {
            
            engine.frame_mutex.lock();
            
            engine.video_width = width;
            engine.video_height = height;
            
            // Resize buffer Slint sesuai dengan dimensi video asli
            engine.pixel_buffer.resize(engine.video_width * engine.video_height * 4);
            
            // Beri tahu VLC untuk menggunakan resolusi yang baru
            engine.player.setVideoFormat("RGBA", engine.video_width, engine.video_height, engine.video_width * 4);
            
            printf("Resolusi video diperbarui secara dinamis: %u x %u\n", engine.video_width, engine.video_height);
            
            engine.frame_mutex.unlock();
        }
    }
}
}
