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

// Helper: Simple URL Decode (to handle %20 etc.)
std::string urlDecode(std::string str) {
    std::string ret;
    char ch;
    int i, ii;
    for (i=0; i<str.length(); i++) {
        if (str[i] == '%') {
            if (i + 2 < str.length()) {
                sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
                ch = static_cast<char>(ii);
                ret += ch;
                i = i + 2;
            }
        } else if (str[i] == '+') {
            ret += ' ';
        } else {
            ret += str[i];
        }
    }
    return ret;
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

            // Extract native resolution
            unsigned v_width = 0;
            unsigned v_height = 0;
            libvlc_media_track_t** tracks;
            unsigned count = libvlc_media_tracks_get(media, &tracks);
            for (unsigned i = 0; i < count; ++i) {
                if (tracks[i]->i_type == libvlc_track_video && tracks[i]->video) {
                    v_width = tracks[i]->video->i_width;
                    v_height = tracks[i]->video->i_height;
                    break;
                }
            }
            if (count > 0) libvlc_media_tracks_release(tracks, count);

            // Apply fallbacks for missing data
            if (title.empty()) title = filename;
            if (artist.empty()) artist = "Unknown Artist";
            if (album.empty()) album = "Unknown Album";
            if (track_num.empty()) track_num = "-";

            std::string dur_str = format_duration(duration_ms);

            // 3. UI Thread Dispatch
            slint::invoke_from_event_loop([title, artist, album, filepath, dur_str, size_str, format, track_num, art, v_width, v_height, ui_model]() {
                std::string cleaned_art = art;
                if (!art.empty()) {
                    if (art.compare(0, 7, "file://") == 0) {
                        cleaned_art = art.substr(7);
                    }
                    cleaned_art = urlDecode(cleaned_art);
                }

                bool actually_has_art = false;
                slint::Image art_img;
                if (!cleaned_art.empty() && fs::exists(cleaned_art)) {
                    art_img = slint::Image::load_from_path(cleaned_art.c_str());
                    actually_has_art = true;
                }

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
                item.art_url = art_img;
                item.has_icon = actually_has_art;
                item.is_playing = false;
                item.video_width = v_width;
                item.video_height = v_height;

                ui_model->push_back(item);
            });
        }
    });

    media.parseWithOptions(VLC::Media::ParseFlags::Local, -1);
}
