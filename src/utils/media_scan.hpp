#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class MediaNode {
private:


    friend class MediaLinkedList;

public:
    std::string path;
    MediaNode* next;
    explicit MediaNode(const std::string& p);

    std::string getPath() const;
};

class MediaLinkedList {
private:
    size_t count;

public:
    MediaNode* head;
    MediaNode* tail;
    MediaLinkedList();
    ~MediaLinkedList();

    void pushBack(const std::string& path);
    void clear();
    size_t getSize() const;
    bool isEmpty() const;
    void printAll() const;
};

class MediaQueue {
private:
    MediaNode* front;
    MediaNode* rear;
    size_t count;

public:
    MediaQueue();
    ~MediaQueue();
    void enqueue(const std::string& path);
    std::string dequeue();
    std::string peek() const;
    void swapNodes(size_t index1, size_t index2);
    std::string removeAt(size_t index);
    bool isEmpty() const;
    size_t getSize() const;
};

class MediaStack {
private:
    MediaNode* top;
    size_t count;

public:
    MediaStack();
    ~MediaStack();
    void push(const std::string& path);
    std::string pop();
    std::string peek() const;
    bool isEmpty() const;
    size_t getSize() const;
};

class MediaScanner {
private:
    fs::path rootPath;

    static bool isMediaFile(const fs::path& filePath);
    static fs::path getDefaultMediaRoot();

public:
    MediaScanner();
    explicit MediaScanner(const fs::path& path);

    fs::path getRootPath() const;
    void setRootPath(const fs::path& path);
    bool isValidRoot() const;
    MediaLinkedList scanToLinkedList();
};
