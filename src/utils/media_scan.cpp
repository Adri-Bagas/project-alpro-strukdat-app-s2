#include "media_scan.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>

static std::string toLowerCopy(std::string text) {
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return text;
}

// --- MediaList Implementation ---
namespace MediaList {
    void init(MediaLinkedList& list) {
        list.head = nullptr;
        list.tail = nullptr;
        list.count = 0;
    }

    void clear(MediaLinkedList& list) {
        while (list.head) {
            MediaNode* temp = list.head;
            list.head = list.head->next;
            delete temp;
        }
        list.tail = nullptr;
        list.count = 0;
    }

    void destroy(MediaLinkedList& list) {
        clear(list);
    }

    void pushBack(MediaLinkedList& list, const std::string& path) {
        MediaNode* node = new MediaNode{path, nullptr};
        if (!list.head) {
            list.head = list.tail = node;
        } else {
            list.tail->next = node;
            list.tail = node;
        }
        list.count++;
    }

    void printAll(const MediaLinkedList& list) {
        const MediaNode* current = list.head;
        size_t index = 1;
        while (current) {
            std::cout << index << ". " << current->path << '\n';
            current = current->next;
            index++;
        }
    }
}

// --- MediaQueueOps Implementation ---
namespace MediaQueueOps {
    void init(MediaQueue& q) {
        q.front = nullptr;
        q.rear = nullptr;
        q.count = 0;
    }

    void destroy(MediaQueue& q) {
        while (!isEmpty(q)) {
            dequeue(q);
        }
    }

    void enqueue(MediaQueue& q, const std::string& path) {
        MediaNode* newNode = new MediaNode{path, nullptr};
        if (isEmpty(q)) {
            q.front = q.rear = newNode;
        } else {
            q.rear->next = newNode;
            q.rear = newNode;
        }
        q.count++;
    }

    std::string dequeue(MediaQueue& q) {
        if (isEmpty(q)) return "";
        MediaNode* temp = q.front;
        std::string path = temp->path;
        q.front = q.front->next;
        if (!q.front) q.rear = nullptr;
        delete temp;
        q.count--;
        return path;
    }

    std::string peek(const MediaQueue& q) {
        return isEmpty(q) ? "" : q.front->path;
    }

    bool isEmpty(const MediaQueue& q) {
        return q.front == nullptr;
    }

    void swapNodes(MediaQueue& q, size_t index1, size_t index2) {
        if (index1 >= q.count || index2 >= q.count || index1 == index2) return;
        
        MediaNode* node1 = q.front;
        for (size_t i = 0; i < index1; ++i) node1 = node1->next;
        
        MediaNode* node2 = q.front;
        for (size_t i = 0; i < index2; ++i) node2 = node2->next;
        
        std::string temp = node1->path;
        node1->path = node2->path;
        node2->path = temp;
    }

    std::string removeAt(MediaQueue& q, size_t index) {
        if (index >= q.count || isEmpty(q)) return "";
        
        MediaNode* toDelete = nullptr;
        std::string path;
        
        if (index == 0) {
            toDelete = q.front;
            q.front = q.front->next;
            if (!q.front) q.rear = nullptr;
        } else {
            MediaNode* prev = q.front;
            for (size_t i = 0; i < index - 1; ++i) prev = prev->next;
            toDelete = prev->next;
            prev->next = toDelete->next;
            if (toDelete == q.rear) q.rear = prev;
        }
        
        path = toDelete->path;
        delete toDelete;
        q.count--;
        return path;
    }
}

// --- MediaStackOps Implementation ---
namespace MediaStackOps {
    void init(MediaStack& s) {
        s.top = nullptr;
        s.count = 0;
    }

    void destroy(MediaStack& s) {
        while (!isEmpty(s)) {
            pop(s);
        }
    }

    void push(MediaStack& s, const std::string& path) {
        MediaNode* newNode = new MediaNode{path, s.top};
        s.top = newNode;
        s.count++;
    }

    std::string pop(MediaStack& s) {
        if (isEmpty(s)) return "";
        MediaNode* temp = s.top;
        std::string path = temp->path;
        s.top = s.top->next;
        delete temp;
        s.count--;
        return path;
    }

    std::string peek(const MediaStack& s) {
        return isEmpty(s) ? "" : s.top->path;
    }

    bool isEmpty(const MediaStack& s) {
        return s.top == nullptr;
    }
}

// --- MediaScannerOps Implementation ---
namespace MediaScannerOps {
    static fs::path getDefaultMediaRoot() {
    #ifdef _WIN32
        if (const char* userProfile = std::getenv("USERPROFILE")) {
            return fs::path(userProfile) / "Music";
        }
        return fs::path("C:/Users/Default/Music");
    #else
        if (const char* home = std::getenv("HOME")) {
            return fs::path(home) / "Music";
        }
        return fs::path("/home") / "Music";
    #endif
    }

    void init(MediaScanner& scanner, const fs::path& path) {
        if (path.empty()) {
            scanner.rootPath = getDefaultMediaRoot();
        } else {
            scanner.rootPath = path;
        }
    }

    bool isMediaFile(const fs::path& filePath) {
        if (!filePath.has_extension()) {
            return false;
        }

        const std::string ext = toLowerCopy(filePath.extension().string());
        return ext == ".mp3"  || ext == ".wav"  || ext == ".flac" || ext == ".aac" ||
               ext == ".ogg"  || ext == ".m4a"  || ext == ".wma"  || ext == ".opus" ||
               ext == ".mp4"  || ext == ".mkv"  || ext == ".avi"  || ext == ".mov"  ||
               ext == ".wmv"  || ext == ".webm" || ext == ".m4v";
    }

    MediaLinkedList scanToLinkedList(MediaScanner& scanner) {
        MediaLinkedList list;
        MediaList::init(list);

        if (scanner.rootPath.empty() || !fs::exists(scanner.rootPath) || !fs::is_directory(scanner.rootPath)) {
            std::cerr << "Error: Root path tidak valid atau tidak ada!\n";
            return list;
        }

        try {
            for (const auto& entry : fs::recursive_directory_iterator(scanner.rootPath)) {
                if (!entry.is_regular_file()) {
                    continue;
                }

                if (isMediaFile(entry.path())) {
                    MediaList::pushBack(list, entry.path().string());
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error saat scan folder: " << e.what() << '\n';
        }

        return list;
    }
}
