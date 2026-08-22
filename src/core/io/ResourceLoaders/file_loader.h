#pragma once

#include <filesystem>
#include <cstdint>
#include <fstream>
#include <stdexcept>

namespace io::loader {

	static std::vector<uint32_t> load_SPIRV(const std::filesystem::path& path) {
		std::ifstream file{ path, std::ios::binary | std::ios::ate };

		if (!file)
			throw std::runtime_error("INVALID: file_path!");

		const std::streamsize size = file.tellg();

		if (size % 4 != 0)
			throw std::logic_error("INVALID: file_size!");

		std::vector<uint32_t> buffer(size / sizeof(uint32_t));

		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(buffer.data()), size);

		if (buffer.empty() || buffer[0] != 0x07230203)
			throw std::logic_error("INVALID: spirv_format!");

		return buffer;
	}

	static std::vector<char> load_BIN(const std::filesystem::path& path) {
		std::ifstream file{ path, std::ios::binary | std::ios::ate };
		const std::streamsize size = file.tellg();

		std::vector<char> buffer(size);

		file.seekg(0, std::ios::beg);
		file.read(buffer.data(), size);

		return buffer;
	}

}