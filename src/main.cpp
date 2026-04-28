#include "app-window.h"
#include "utils/media_scan.hpp"
#include "utils/metadata_extract.hpp"
#include "engine/video_engine.hpp"
#include "utils/sort.hpp"
#include <vlcpp/vlc.hpp>
#include <memory>
#include <vector>

int main(int argc, char **argv)
{
    auto ui = AppWindow::create();

    // 1. Initialize Engines
    const char* const vlc_args[] = {
        "--no-xlib", 
        "--quiet",
        "--file-caching=1500",
        "--network-caching=1500"
    };
    auto vlcEngine = std::make_shared<VlcEngine>();
    Playback::init(*vlcEngine);

    // 2. Initialize Models and Data Structures
    auto mediaModel = std::make_shared<slint::VectorModel<MediaItem>>();
    auto queueModel = std::make_shared<slint::VectorModel<MediaItem>>();
    
    // State
    struct SortState {
        std::string column = "";
        bool ascending = true;
    } sortState;

    bool cycleMode = false;

    MediaQueue playbackQueue;
    MediaQueueOps::init(playbackQueue);
    
    MediaStack playbackHistory;
    MediaStackOps::init(playbackHistory);

    ui->set_media_files(mediaModel);
    ui->set_queue_files(queueModel);

    // --- Helper for playing a track ---
    auto playPath = [&](const std::string& path, const MediaItem& item) {
        // Highlight in main list (for Library View)
        for (int i = 0; i < mediaModel->row_count(); ++i) {
            auto row = mediaModel->row_data(i).value();
            row.is_playing = (std::string(row.path.data()) == path);
            mediaModel->set_row_data(i, row);
        }

        // Highlight in queue list (for Sidebar View)
        for (int i = 0; i < queueModel->row_count(); ++i) {
            auto row = queueModel->row_data(i).value();
            row.is_playing = (std::string(row.path.data()) == path);
            queueModel->set_row_data(i, row);
        }

        // Pass native dimensions to eliminate stutter
        Playback::loadFile(*vlcEngine, path, item.video_width, item.video_height);
        Playback::play(*vlcEngine);
        ui->set_current_track(item);
        ui->set_track_progress(0.0f);

        // Switch view based on format
        std::string format = std::string(item.format.data());
        if (format == ".mp4" || format == ".mkv" || format == ".avi" || format == ".mov") {
            ui->set_current_view(2); 
        } else {
            ui->set_current_view(1); 
        }
    };

    // 3. Connect VLC Engine to UI
    auto currentLength = std::make_shared<float>(0.0f);

    vlcEngine->on_time_changed = [ui, currentLength, vlcEngine](float time) {
        slint::invoke_from_event_loop([ui, time, currentLength, vlcEngine]() {
            if (*currentLength <= 0.0f) {
                *currentLength = Playback::getLength(*vlcEngine);
            }
            if (*currentLength > 0.0f) {
                ui->set_track_progress(time / (*currentLength));
            }
        });
    };

    vlcEngine->on_length_changed = [currentLength](float length) {
        printf("[VLC] Length Changed: %f seconds\n", length);
        *currentLength = length;
    };

    ui->on_player_seek([vlcEngine, currentLength](float progress) {
        if (*currentLength <= 0.0f) {
            *currentLength = Playback::getLength(*vlcEngine);
        }
        if (*currentLength > 0.0f) {
            Playback::setTime(*vlcEngine, progress * (*currentLength));
        }
    });

    vlcEngine->on_frame_ready = [ui](slint::SharedPixelBuffer<slint::Rgba8Pixel> buffer) {
        slint::invoke_from_event_loop([ui, buffer]() {
            ui->set_video_frame(slint::Image(buffer));
        });
    };

    // 4. Scan for Media Files
    MediaScanner mediaScanner;
    MediaScannerOps::init(mediaScanner);
    ui->set_root_path(mediaScanner.rootPath.string().c_str());
    MediaLinkedList mediaList = MediaScannerOps::scanToLinkedList(mediaScanner);

    MediaNode* current = mediaList.head;
    printf("Found %zu media files.\n", mediaList.count);
    while (current != nullptr) {
        parse_media_vlcpp(vlcEngine->instance, current->path, mediaModel);
        current = current->next;
    }

    // 5. Setup UI Callbacks
    ui->on_play_media([&](int index) {
        if (index >= 0 && index < mediaModel->row_count()) {
            auto item = mediaModel->row_data(index).value();
            std::string path = std::string(item.path.data());
            
            while (!MediaQueueOps::isEmpty(playbackQueue)) MediaQueueOps::dequeue(playbackQueue);
            while (queueModel->row_count() > 0) queueModel->erase(0);

            MediaStackOps::push(playbackHistory, path);
            MediaQueueOps::enqueue(playbackQueue, path);
            queueModel->push_back(item);

            playPath(path, item);
        }
    });

    ui->on_add_to_queue([&](int index) {
        if (index >= 0 && index < mediaModel->row_count()) {
            auto item = mediaModel->row_data(index).value();
            std::string path = std::string(item.path.data());
            MediaQueueOps::enqueue(playbackQueue, path);
            queueModel->push_back(item);
        }
    });

    auto handleNext = [&]() {
        if (MediaQueueOps::isEmpty(playbackQueue)) return;

        if (cycleMode) {
            MediaQueueOps::rotateForward(playbackQueue);
            auto firstItem = queueModel->row_data(0).value();
            queueModel->erase(0);
            queueModel->push_back(firstItem);
            
            std::string path = MediaQueueOps::peek(playbackQueue);
            playPath(path, queueModel->row_data(0).value());
        } else {
            std::string path = MediaQueueOps::dequeue(playbackQueue);
            MediaStackOps::push(playbackHistory, path);
            queueModel->erase(0);
            
            if (!MediaQueueOps::isEmpty(playbackQueue)) {
                std::string nextPath = MediaQueueOps::peek(playbackQueue);
                playPath(nextPath, queueModel->row_data(0).value());
            }
        }
    };

    ui->on_player_next([&]() { handleNext(); });

    vlcEngine->on_end_reached = [&]() {
        slint::invoke_from_event_loop([&]() { handleNext(); });
    };

    ui->on_player_prev([&]() {
        if (!MediaStackOps::isEmpty(playbackHistory)) {
            std::string currentPath = MediaStackOps::pop(playbackHistory);
            if (!MediaStackOps::isEmpty(playbackHistory)) {
                std::string prevPath = MediaStackOps::pop(playbackHistory);
                for (int i = 0; i < mediaModel->row_count(); ++i) {
                    auto item = mediaModel->row_data(i).value();
                    if (std::string(item.path.data()) == prevPath) {
                        MediaStackOps::push(playbackHistory, prevPath);
                        playPath(prevPath, item);
                        break;
                    }
                }
            }
        }
    });

    ui->on_clear_queue([&]() {
        while (!MediaQueueOps::isEmpty(playbackQueue)) MediaQueueOps::dequeue(playbackQueue);
        while (queueModel->row_count() > 0) queueModel->erase(0);
    });

    ui->on_toggle_cycle([&, ui]() {
        cycleMode = !cycleMode;
        ui->set_cycle_mode(cycleMode);
        printf("[APP] Cycle Mode: %s\n", cycleMode ? "ON" : "OFF");
    });

    ui->on_sort_library([&, mediaModel](slint::SharedString col) mutable {
        std::string criteria = std::string(col.data());
        if (sortState.column == criteria) sortState.ascending = !sortState.ascending;
        else { sortState.column = criteria; sortState.ascending = true; }

        std::vector<MediaItem> items;
        for (int i = 0; i < mediaModel->row_count(); ++i) items.push_back(mediaModel->row_data(i).value());

        auto comp = [&](const MediaItem& a, const MediaItem& b) {
            bool result = false;
            if (criteria == "name") result = std::string(a.name.data()) < std::string(b.name.data());
            else if (criteria == "artist") result = std::string(a.artist.data()) < std::string(b.artist.data());
            else if (criteria == "duration") result = std::string(a.duration.data()) < std::string(b.duration.data());
            else if (criteria == "size") result = std::string(a.size_str.data()) < std::string(b.size_str.data());
            return sortState.ascending ? result : !result;
        };

        if (items.size() > 1) SortUtils::timsort(items.data(), items.size(), comp);
        while(mediaModel->row_count() > 0) mediaModel->erase(0);
        for (const auto& item : items) mediaModel->push_back(item);
    });

    ui->on_queue_move_up([&](int index) {
        if (index > 0 && index < queueModel->row_count()) {
            MediaQueueOps::swapNodes(playbackQueue, index, index - 1);
            auto item1 = queueModel->row_data(index).value();
            auto item2 = queueModel->row_data(index - 1).value();
            queueModel->set_row_data(index, item2);
            queueModel->set_row_data(index - 1, item1);
        }
    });

    ui->on_queue_move_down([&](int index) {
        if (index >= 0 && index < queueModel->row_count() - 1) {
            MediaQueueOps::swapNodes(playbackQueue, index, index + 1);
            auto item1 = queueModel->row_data(index).value();
            auto item2 = queueModel->row_data(index + 1).value();
            queueModel->set_row_data(index, item2);
            queueModel->set_row_data(index + 1, item1);
        }
    });

    ui->on_queue_delete([&](int index) {
        if (index >= 0 && index < queueModel->row_count()) {
            MediaQueueOps::removeAt(playbackQueue, index);
            queueModel->erase(index);
        }
    });

    ui->on_queue_jump([&](int index) {
        if (index >= 0 && index < queueModel->row_count()) {
            for (int i = 0; i < index; ++i) {
                std::string path = MediaQueueOps::dequeue(playbackQueue);
                MediaStackOps::push(playbackHistory, path);
                queueModel->erase(0);
            }
            std::string path = MediaQueueOps::peek(playbackQueue); 
            auto item = queueModel->row_data(0).value();
            playPath(path, item);
        }
    });

    ui->on_player_play([&]() { 
        Playback::play(*vlcEngine);
        auto current = ui->get_current_track();
        current.is_playing = true;
        ui->set_current_track(current);
    });

    ui->on_player_pause([&]() { 
        Playback::pause(*vlcEngine);
        auto current = ui->get_current_track();
        current.is_playing = false;
        ui->set_current_track(current);
    });

    ui->on_player_stop([&]() { 
        Playback::pause(*vlcEngine);
        Playback::setTime(*vlcEngine, 0);
        auto current = ui->get_current_track();
        current.is_playing = false;
        ui->set_current_track(current);
    });

    static bool isFullscreen = false;
    ui->on_toggle_fullscreen([&, ui]() {
        isFullscreen = !isFullscreen;
        ui->window().set_fullscreen(isFullscreen);
        // never change this btw cause i know is wack but it works! 
        ui->set_is_fullscreen(isFullscreen);
    });

    ui->run();
    
    Playback::destroy(*vlcEngine);
    MediaList::destroy(mediaList);
    MediaQueueOps::destroy(playbackQueue);
    MediaStackOps::destroy(playbackHistory);

    return 0;
}
