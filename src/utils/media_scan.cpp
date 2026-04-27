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

MediaNode::MediaNode(const std::string& p) : path(p), next(nullptr) {}

std::string MediaNode::getPath() const {
	return path;
}

MediaLinkedList::MediaLinkedList() : head(nullptr), tail(nullptr), count(0) {}

MediaLinkedList::~MediaLinkedList() {
	clear();
}

void MediaLinkedList::pushBack(const std::string& path) {
	MediaNode* node = new MediaNode(path);
	if (!head) {
		head = tail = node;
	} else {
		tail->next = node;
		tail = node;
	}
	count++;
}

void MediaLinkedList::clear() {
	while (head) {
		MediaNode* temp = head;
		head = head->next;
		delete temp;
	}
	tail = nullptr;
	count = 0;
}

size_t MediaLinkedList::getSize() const {
	return count;
}

bool MediaLinkedList::isEmpty() const {
	return head == nullptr;
}

void MediaLinkedList::printAll() const {
	const MediaNode* current = head;
	size_t index = 1;
	while (current) {
		std::cout << index << ". " << current->getPath() << '\n';
		current = current->next;
		index++;
	}
}

// MediaQueue Implementation
MediaQueue::MediaQueue() : front(nullptr), rear(nullptr), count(0) {}

MediaQueue::~MediaQueue() {
	while (!isEmpty()) {
		dequeue();
	}
}

void MediaQueue::enqueue(const std::string& path) {
	MediaNode* newNode = new MediaNode(path);
	if (isEmpty()) {
		front = rear = newNode;
	} else {
		rear->next = newNode;
		rear = newNode;
	}
	count++;
}

std::string MediaQueue::dequeue() {
	if (isEmpty()) return "";
	MediaNode* temp = front;
	std::string path = temp->path;
	front = front->next;
	if (!front) rear = nullptr;
	delete temp;
	count--;
	return path;
}

std::string MediaQueue::peek() const {
	return isEmpty() ? "" : front->path;
}

void MediaQueue::swapNodes(size_t index1, size_t index2) {
	if (index1 >= count || index2 >= count || index1 == index2) return;
	
	MediaNode* node1 = front;
	for (size_t i = 0; i < index1; ++i) node1 = node1->next;
	
	MediaNode* node2 = front;
	for (size_t i = 0; i < index2; ++i) node2 = node2->next;
	
	std::string temp = node1->path;
	node1->path = node2->path;
	node2->path = temp;
}

std::string MediaQueue::removeAt(size_t index) {
	if (index >= count || isEmpty()) return "";
	
	MediaNode* toDelete = nullptr;
	std::string path;
	
	if (index == 0) {
		toDelete = front;
		front = front->next;
		if (!front) rear = nullptr;
	} else {
		MediaNode* prev = front;
		for (size_t i = 0; i < index - 1; ++i) prev = prev->next;
		toDelete = prev->next;
		prev->next = toDelete->next;
		if (toDelete == rear) rear = prev;
	}
	
	path = toDelete->path;
	delete toDelete;
	count--;
	return path;
}

bool MediaQueue::isEmpty() const {
	return front == nullptr;
}

size_t MediaQueue::getSize() const {
	return count;
}

// MediaStack Implementation
MediaStack::MediaStack() : top(nullptr), count(0) {}

MediaStack::~MediaStack() {
	while (!isEmpty()) {
		pop();
	}
}

void MediaStack::push(const std::string& path) {
	MediaNode* newNode = new MediaNode(path);
	newNode->next = top;
	top = newNode;
	count++;
}

std::string MediaStack::pop() {
	if (isEmpty()) return "";
	MediaNode* temp = top;
	std::string path = temp->path;
	top = top->next;
	delete temp;
	count--;
	return path;
}

std::string MediaStack::peek() const {
	return isEmpty() ? "" : top->path;
}

bool MediaStack::isEmpty() const {
	return top == nullptr;
}

size_t MediaStack::getSize() const {
	return count;
}

bool MediaScanner::isMediaFile(const fs::path& filePath) {
	if (!filePath.has_extension()) {
		return false;
	}

	const std::string ext = toLowerCopy(filePath.extension().string());
	return ext == ".mp3"  || ext == ".wav"  || ext == ".flac" || ext == ".aac" ||
		   ext == ".ogg"  || ext == ".m4a"  || ext == ".wma"  || ext == ".opus" ||
		   ext == ".mp4"  || ext == ".mkv"  || ext == ".avi"  || ext == ".mov"  ||
		   ext == ".wmv"  || ext == ".webm" || ext == ".m4v";
}

fs::path MediaScanner::getDefaultMediaRoot() {
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

MediaScanner::MediaScanner() : rootPath(getDefaultMediaRoot()) {}

MediaScanner::MediaScanner(const fs::path& path) : rootPath(path) {}

fs::path MediaScanner::getRootPath() const {
	return rootPath;
}

void MediaScanner::setRootPath(const fs::path& path) {
	rootPath = path;
}

bool MediaScanner::isValidRoot() const {
	return !rootPath.empty() && fs::exists(rootPath) && fs::is_directory(rootPath);
}

MediaLinkedList MediaScanner::scanToLinkedList() {
	MediaLinkedList list;

	if (!isValidRoot()) {
		std::cerr << "Error: Root path tidak valid atau tidak ada!\n";
		return list;
	}

	try {
		for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
			if (!entry.is_regular_file()) {
				continue;
			}

			if (isMediaFile(entry.path())) {
				list.pushBack(entry.path().string());
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error saat scan folder: " << e.what() << '\n';
	}

	return list;
}
