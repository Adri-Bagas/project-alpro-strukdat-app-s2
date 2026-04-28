#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// --- DATA STRUCTURES (Pure Data) ---

struct MediaNode {
    std::string path;
    MediaNode* next;
    MediaNode* prev; // Added for Doubly Linked List
};

struct MediaLinkedList {
    MediaNode* head;
    MediaNode* tail;
    size_t count;
};

struct MediaQueue {
    MediaNode* front;
    MediaNode* rear;
    size_t count;
};

struct MediaStack {
    MediaNode* top;
    size_t count;
};

struct MediaScanner {
    fs::path rootPath;
};

// --- NAMESPACES FOR LOGIC ---

namespace MediaList {
    void init(MediaLinkedList& list);
    void destroy(MediaLinkedList& list);
    void pushBack(MediaLinkedList& list, const std::string& path);
    void clear(MediaLinkedList& list);
    void printAll(const MediaLinkedList& list);
}

namespace MediaQueueOps {
    void init(MediaQueue& q);
    void destroy(MediaQueue& q);
    // Circular Doubly Linked List implementation for Queue
    void enqueue(MediaQueue& q, const std::string& path);
    std::string dequeue(MediaQueue& q);
    std::string peek(const MediaQueue& q);
    void swapNodes(MediaQueue& q, size_t index1, size_t index2);
    std::string removeAt(MediaQueue& q, size_t index);
    bool isEmpty(const MediaQueue& q);
    
    // Cycle helpers
    void rotateForward(MediaQueue& q); // Move front to next, for cycling
}

namespace MediaStackOps {
    void init(MediaStack& s);
    void destroy(MediaStack& s);
    void push(MediaStack& s, const std::string& path);
    std::string pop(MediaStack& s);
    std::string peek(const MediaStack& s);
    bool isEmpty(const MediaStack& s);
}

namespace MediaScannerOps {
    void init(MediaScanner& scanner, const fs::path& path = "");
    MediaLinkedList scanToLinkedList(MediaScanner& scanner);
    bool isMediaFile(const fs::path& filePath);
}
