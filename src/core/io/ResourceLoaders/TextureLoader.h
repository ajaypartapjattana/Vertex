#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>

namespace io::loader {

	class TextureLoader {
	public:
		TextureLoader();

		TextureLoader(const TextureLoader&) = delete;
		TextureLoader& operator=(const TextureLoader&) = delete;

		~TextureLoader();

		void requestLoad(const std::string& texPath);

		bool hasPendingLoad() const;

		struct Texture {
			std::string tex_FilePath;

			struct TextureAttributes {
				int width = 0;
				int height = 0;
				int channels = 0;
			} attributes{};

			std::vector<uint8_t> pixels;

			Texture(const std::string& path)
				: tex_FilePath(path) {
			}
		};

		std::vector<std::unique_ptr<Texture>> collectLoadedTextures();

	private:
		void load(Texture&);

		void loaderLoop();

	private:
		std::atomic<bool> running{ false };
		std::atomic<uint32_t> outstandingLoads{ 0 };

		ThreadSafeQueue<std::string> loadRequests;
		ThreadSafeQueue<std::unique_ptr<Texture>> loadedTextures;

		std::thread loaderThread;

		std::mutex wakeMutex;
		std::condition_variable wakeCV;
	};

}
