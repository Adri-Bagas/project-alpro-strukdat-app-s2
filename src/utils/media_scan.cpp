#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

class MediaNode {
private:
	std::string path;
	MediaNode* next;

	friend class MediaLinkedList;

public:
	MediaNode(const std::string& p) : path(p), next(nullptr) {}
	
	std::string getPath() const {
		return path;
	}
};

class MediaLinkedList {
private:
	MediaNode* head;
	MediaNode* tail;
	size_t count;

public:
	MediaLinkedList() : head(nullptr), tail(nullptr), count(0) {}

	void pushBack(const std::string& path) {
		MediaNode* node = new MediaNode(path);
		if (!head) {
			head = tail = node;
		} else {
			tail->next = node;
			tail = node;
		}
		count++;
	}

	void clear() {
		while (head) {
			MediaNode* temp = head;
			head = head->next;
			delete temp;
		}
		tail = nullptr;
		count = 0;
	}

	size_t getSize() const {
		return count;
	}

	bool isEmpty() const {
		return head == nullptr;
	}

	void printAll() const {
		const MediaNode* current = head;
		size_t index = 1;
		while (current) {
			std::cout << index << ". " << current->getPath() << '\n';
			current = current->next;
			index++;
		}
	}

	~MediaLinkedList() {
		clear();
	}
};

static std::string toLowerCopy(std::string text) {
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return text;
}

class MediaScanner {
private:
	fs::path rootPath;

	static bool isMediaFile(const fs::path& filePath) {
		if (!filePath.has_extension()) {
			return false;
		}

		const std::string ext = toLowerCopy(filePath.extension().string());
		return ext == ".mp3"  || ext == ".wav"  || ext == ".flac" || ext == ".aac" ||
			   ext == ".ogg"  || ext == ".m4a"  || ext == ".wma"  || ext == ".opus" ||
			   ext == ".mp4"  || ext == ".mkv"  || ext == ".avi"  || ext == ".mov"  ||
			   ext == ".wmv"  || ext == ".webm" || ext == ".m4v";
	}

	static fs::path getDefaultMediaRoot() {
#ifdef _WIN32
		if (const char* userProfile = std::getenv("USERPROFILE")) {
			return fs::path(userProfile) / "Music";
		}
		return fs::path("C:/Users/Default/Music");
#else
		if (const char* home = std::getenv("HOME")) {
			return fs::path(home) / "Musics";
		}
		return fs::path("/home") / "Musics";
#endif
	}

public:
	MediaScanner() : rootPath(getDefaultMediaRoot()) {}

	explicit MediaScanner(const fs::path& path) : rootPath(path) {}

	fs::path getRootPath() const {
		return rootPath;
	}

	void setRootPath(const fs::path& path) {
		rootPath = path;
	}

	bool isValidRoot() const {
		return !rootPath.empty() && fs::exists(rootPath) && fs::is_directory(rootPath);
	}

	MediaLinkedList scanToLinkedList() {
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
};

int main() {
	MediaScanner scanner;
	
	std::cout << "Root folder: " << scanner.getRootPath().string() << '\n';
	std::cout << "Status folder: " << (scanner.isValidRoot() ? "Valid" : "Tidak valid") << '\n';
	std::cout << '\n';

	MediaLinkedList mediaFiles = scanner.scanToLinkedList();
	
	std::cout << "Total file media ditemukan: " << mediaFiles.getSize() << '\n';
	std::cout << "Daftar file media:\n";
	mediaFiles.printAll();

	return 0;
}
