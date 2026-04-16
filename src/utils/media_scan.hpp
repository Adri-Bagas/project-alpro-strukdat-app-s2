#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class MediaNode {
private:
    std::string path;
    MediaNode* next;

    friend class MediaLinkedList;

public:
    explicit MediaNode(const std::string& p);

    std::string getPath() const;
};

class MediaLinkedList {
private:
    MediaNode* head;
    MediaNode* tail;
    size_t count;

public:
    MediaLinkedList();
    ~MediaLinkedList();

    void pushBack(const std::string& path);
    void clear();
    size_t getSize() const;
    bool isEmpty() const;
    void printAll() const;
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
