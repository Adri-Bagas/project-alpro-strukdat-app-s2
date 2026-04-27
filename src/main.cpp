#include "app-window.h"
#include "utils/media_scan.hpp"
#include "utils/metadata_extract.hpp"
#include "engine/video_engine.hpp"
#include <vlcpp/vlc.hpp>
#include <memory>

int main(int argc, char **argv)
{
    auto ui = AppWindow::create();

    // 1. Initialize Engines
    VLC::Instance vlcInstance(0, nullptr);
    auto vlcEngine = std::make_shared<VlcEngine>();

    // 2. Initialize Models and Data Structures
    auto mediaModel = std::make_shared<slint::VectorModel<MediaItem>>();
    auto queueModel = std::make_shared<slint::VectorModel<MediaItem>>();
    MediaQueue playbackQueue;
    MediaStack playbackHistory;

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

        vlcEngine->load_file(path);
        vlcEngine->play();
        ui->set_current_track(item);
        ui->set_track_progress(0.0f); // Reset progress on new track

        // Switch view based on format
        std::string format = std::string(item.format.data());
        if (format == ".mp4" || format == ".mkv" || format == ".avi" || format == ".mov") {
            ui->set_current_view(2); // Video View
        } else {
            ui->set_current_view(1); // Now Playing View
        }
    };

    // 3. Connect VLC Engine to UI
    auto currentLength = std::make_shared<float>(0.0f);

    vlcEngine->on_time_changed = [ui, currentLength, vlcEngine](float time) {
        slint::invoke_from_event_loop([ui, time, currentLength, vlcEngine]() {
            if (*currentLength <= 0.0f) {
                *currentLength = vlcEngine->get_length();
            }
            if (*currentLength > 0.0f) {
                ui->set_track_progress(time / (*currentLength));
            }
        });
    };

    vlcEngine->on_length_changed = [currentLength](float length) {
        printf("VLC Length Changed: %f seconds\n", length);
        *currentLength = length;
    };

    ui->on_player_seek([vlcEngine, currentLength](float progress) {
        if (*currentLength <= 0.0f) {
            *currentLength = vlcEngine->get_length();
        }
        printf("Seek requested to %f%%. Current length: %f\n", progress * 100.0f, *currentLength);
        if (*currentLength > 0.0f) {
            vlcEngine->set_time(progress * (*currentLength));
        }
    });

    vlcEngine->on_frame_ready = [ui](slint::SharedPixelBuffer<slint::Rgba8Pixel> buffer) {
        slint::invoke_from_event_loop([ui, buffer]() {
            ui->set_video_frame(slint::Image(buffer));
        });
    };

    // 4. Scan for Media Files
    MediaScanner mediaScanner;
    ui->set_root_path(mediaScanner.getRootPath().string().c_str());
    MediaLinkedList mediaList = mediaScanner.scanToLinkedList();

    MediaNode* current = mediaList.head;
    printf("Found %zu media files. Starting metadata extraction...\n", mediaList.getSize());
    while (current != nullptr) {
        parse_media_vlcpp(vlcInstance, current->path, mediaModel);
        current = current->next;
    }

    // 5. Setup UI Callbacks
    ui->on_play_media([&](int index) {
        if (index >= 0 && index < mediaModel->row_count()) {
            auto item = mediaModel->row_data(index).value();
            std::string path = std::string(item.path.data());
            
            // Clear existing queue logic as requested
            while (!playbackQueue.isEmpty()) playbackQueue.dequeue();
            while (queueModel->row_count() > 0) queueModel->erase(0);

            // Add the new song as the ONLY item in the queue
            playbackHistory.push(path);
            playbackQueue.enqueue(path);
            queueModel->push_back(item);

            playPath(path, item);
        }
    });

    ui->on_add_to_queue([&](int index) {
        if (index >= 0 && index < mediaModel->row_count()) {
            auto item = mediaModel->row_data(index).value();
            std::string path = std::string(item.path.data());
            
            // Just add to queue, don't clear or play
            playbackQueue.enqueue(path);
            queueModel->push_back(item);
            
            printf("Added to queue (manual): %s\n", path.c_str());
        }
    });

    ui->on_player_next([&]() {
        if (!playbackQueue.isEmpty()) {
            std::string path = playbackQueue.dequeue();
            
            // Find item in queueModel to get metadata, then remove it
            for (int i = 0; i < queueModel->row_count(); ++i) {
                auto item = queueModel->row_data(i).value();
                if (std::string(item.path.data()) == path) {
                    playbackHistory.push(path); // Add to history before playing next
                    playPath(path, item);
                    queueModel->erase(i);
                    break;
                }
            }
        }
    });

    ui->on_player_prev([&]() {
        if (!playbackHistory.isEmpty()) {
            // Pop the CURRENT song first (since it was the last pushed)
            std::string currentPath = playbackHistory.pop();
            
            // Now pop the PREVIOUS song to play it
            if (!playbackHistory.isEmpty()) {
                std::string prevPath = playbackHistory.pop();
                
                // Find metadata in main mediaModel
                for (int i = 0; i < mediaModel->row_count(); ++i) {
                    auto item = mediaModel->row_data(i).value();
                    if (std::string(item.path.data()) == prevPath) {
                        playbackHistory.push(prevPath); // Re-push as it's now "current"
                        playPath(prevPath, item);
                        break;
                    }
                }
            }
        }
    });

    ui->on_clear_queue([&]() {
        while (!playbackQueue.isEmpty()) playbackQueue.dequeue();
        while (queueModel->row_count() > 0) queueModel->erase(0);
    });

    ui->on_queue_move_up([&](int index) {
        if (index > 0 && index < queueModel->row_count()) {
            playbackQueue.swapNodes(index, index - 1);
            auto item1 = queueModel->row_data(index).value();
            auto item2 = queueModel->row_data(index - 1).value();
            queueModel->set_row_data(index, item2);
            queueModel->set_row_data(index - 1, item1);
        }
    });

    ui->on_queue_move_down([&](int index) {
        if (index >= 0 && index < queueModel->row_count() - 1) {
            playbackQueue.swapNodes(index, index + 1);
            auto item1 = queueModel->row_data(index).value();
            auto item2 = queueModel->row_data(index + 1).value();
            queueModel->set_row_data(index, item2);
            queueModel->set_row_data(index + 1, item1);
        }
    });

    ui->on_queue_delete([&](int index) {
        if (index >= 0 && index < queueModel->row_count()) {
            playbackQueue.removeAt(index);
            queueModel->erase(index);
        }
    });

    ui->on_queue_jump([&](int index) {
        if (index >= 0 && index < queueModel->row_count()) {
            // 1. All previous songs moved to history and removed from queue
            for (int i = 0; i < index; ++i) {
                std::string path = playbackQueue.dequeue();
                playbackHistory.push(path);
                queueModel->erase(0);
            }

            // 2. Play the current target without removing it from the queue
            // We just peek the path since we want it to stay in the queue
            std::string path = playbackQueue.peek(); 
            auto item = queueModel->row_data(0).value();
            
            // Note: We don't push to history here because it's now the "current" track
            // It will be pushed to history when the NEXT track starts.
            playPath(path, item);
        }
    });

    ui->on_player_play([vlcEngine]() { vlcEngine->play(); });
    ui->on_player_pause([vlcEngine]() { vlcEngine->pause(); });
    ui->on_player_stop([vlcEngine]() { 
        // We'll treat stop as pause + reset time for now
        vlcEngine->pause();
        vlcEngine->set_time(0);
    });

    ui->run();
    return 0;
}
