#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

namespace io::loader {

	TextureLoader::TextureLoader()
		: running(true)
	{
		loaderThread = std::thread(&TextureLoader::loaderLoop, this);
	}

	TextureLoader::~TextureLoader() {
		running.store(false, std::memory_order_relaxed);
		wakeCV.notify_all();

		if (loaderThread.joinable())
			loaderThread.join();
	}

	void TextureLoader::requestLoad(const std::string& texPath) {
		outstandingLoads.fetch_add(1, std::memory_order_relaxed);
		loadRequests.push(texPath);

		wakeCV.notify_one();
	}

	bool TextureLoader::hasPendingLoad() const {
		return outstandingLoads.load(std::memory_order_acquire) > 0;
	}

	std::vector<std::unique_ptr<TextureLoader::Texture>> TextureLoader::collectLoadedTextures() {
		std::vector<std::unique_ptr<Texture>> texturesFound;

		std::optional<std::unique_ptr<Texture>> maybeTexture = loadedTextures.try_pop();
		while (maybeTexture.has_value()) {
			texturesFound.push_back(std::move(maybeTexture.value()));

			maybeTexture = loadedTextures.try_pop();
		}

		return texturesFound;
	}

	void TextureLoader::loaderLoop() {
		while (running) {
			{
				std::unique_lock lock(wakeMutex);

				wakeCV.wait(lock, [&] {return !running || !loadRequests.empty(); });
			}

			std::optional<std::string> req = loadRequests.try_pop();
			if (!req.has_value())
				continue;

			struct CounterGuard {
				std::atomic<uint32_t>& c;
				~CounterGuard() { c.fetch_sub(1, std::memory_order_release); }
			} gaurd{ outstandingLoads };

			std::unique_ptr<Texture> texture = std::make_unique<Texture>(req.value());

			try {
				load(*texture);
			}
			catch (const std::exception& e) {
				std::cerr << '\n' << e.what() << std::endl;
			}

			loadedTextures.push(std::move(texture));
		}
	}

	void TextureLoader::load(Texture& texture) {
		int width = 0;
		int height = 0;
		int channels = 0;

		constexpr int desiresChannels = 4;

		stbi_uc* data = stbi_load(texture.tex_FilePath.c_str(), &width, &height, &channels, desiresChannels);

		if (!data)
			throw std::runtime_error("Failed to load texture: " + texture.tex_FilePath);

		texture.attributes.width = width;
		texture.attributes.height = height;
		texture.attributes.channels = channels;

		size_t byteSize = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(desiresChannels);

		texture.pixels.resize(byteSize);
		std::memcpy(texture.pixels.data(), data, byteSize);

		stbi_image_free(data);
	}

}
