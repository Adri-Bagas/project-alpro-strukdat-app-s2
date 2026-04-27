#include <filesystem>
#include <vlcpp/vlc.hpp>
#include <slint.h>
#include <string>
#include <iomanip>
#include <sstream>
#include "app-window.h"

namespace fs = std::filesystem;

// Helper: Format milliseconds to mm:ss
std::string format_duration(int64_t ms) {
    if (ms <= 0) return "0:00";
    int total_seconds = ms / 1000;
    int minutes = total_seconds / 60;
    int seconds = total_seconds % 60;
    std::ostringstream oss;
    oss << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;
    return oss.str();
}

// Helper: Format bytes to MB
std::string format_size(uintmax_t bytes) {
    double mb = bytes / (1024.0 * 1024.0);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << mb << " MB";
    return oss.str();
}

// Main parser function
void parse_media_vlcpp(VLC::Instance& instance, const std::string& filepath, std::shared_ptr<slint::VectorModel<MediaItem>> ui_model) {
    fs::path p(filepath);
    
    // 1. OS-Level Extraction (Synchronous)
    std::string filename = p.filename().string();
    std::string format = p.extension().string();
    uintmax_t size_bytes = fs::exists(p) ? fs::file_size(p) : 0;
    std::string size_str = format_size(size_bytes);

    // 2. Media-Level Extraction (Asynchronous)
    VLC::Media media(instance, filepath, VLC::Media::FromPath);

    media.eventManager().onParsedChanged([media, filepath, filename, format, size_str, ui_model](VLC::Media::ParsedStatus status) mutable {
        if (status == VLC::Media::ParsedStatus::Done) {
            
            // Extract raw metadata
            std::string title = media.meta(libvlc_meta_Title);
            std::string artist = media.meta(libvlc_meta_Artist);
            std::string album = media.meta(libvlc_meta_Album);
            std::string track_num = media.meta(libvlc_meta_TrackNumber);
            std::string art = media.meta(libvlc_meta_ArtworkURL);
            int64_t duration_ms = media.duration();

            // Apply fallbacks for missing data
            if (title.empty()) title = filename;
            if (artist.empty()) artist = "Unknown Artist";
            if (album.empty()) album = "Unknown Album";
            if (track_num.empty()) track_num = "-";

            std::string dur_str = format_duration(duration_ms);

            // 3. UI Thread Dispatch
            slint::invoke_from_event_loop([title, artist, album, filepath, dur_str, size_str, format, track_num, art, ui_model]() {
                printf("Pushing item to UI: %s\n", title.c_str());
                MediaItem item;
                item.name = title.c_str();
                item.details = album.c_str();
                item.artist = artist.c_str();
                item.album = album.c_str();
                item.path = filepath.c_str();
                item.duration = dur_str.c_str();
                item.size_str = size_str.c_str();
                item.format = format.c_str();
                item.track_num = track_num.c_str();
                item.has_icon = !art.empty();
                item.is_playing = false;

                ui_model->push_back(item);
            });
        } else {
            printf("Media parsing status for %s: %d\n", filename.c_str(), (int)status);
        }
    });

    media.parseWithOptions(VLC::Media::ParseFlags::Local, -1);
}
