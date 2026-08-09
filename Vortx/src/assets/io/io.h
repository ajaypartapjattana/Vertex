#pragma once

#include <vector>
#include <stdint.h>

namespace io {

	enum Result{
		SUCCESS = 1,
		FAILURE = 0,
		INVALID_ARGUMENTS = -1,
		NOT_PERMITTED = -2,
		VALIDITY_CHECK_FAILURE = -3,
		UNSUPPORTED_FEATURE = -4
	};

	enum ImageType : uint16_t {
		IMAGE_TYPE_UNDEFINED,
		IMAGE_TYPE_GRAYSCALE,
		IMAGE_TYPE_GRAYSCALE_ALPHA,
		IMAGE_TYPE_INDEXED,
		IMAGE_TYPE_RGB,
		IMAGE_TYPE_RGB_ALPHA
	};

	struct ImageInfo {
		uint32_t width;
		uint32_t height;
		uint16_t channels;
		uint16_t bitDepth;
	};

	int loadBinary(const wchar_t* const _Path, size_t* const pSize, void* const pDst) noexcept;

	int fetchPngInfo(const void* pBin, size_t _Size, ImageInfo* const pInfo) noexcept;

	int decodePng(void* pDst, const ImageInfo* const pInfo, const void* const pSrc, size_t _SrcSize) noexcept;

	struct ImageFile {
		uint32_t width;
		uint32_t height;
		uint16_t bitDepth;
		ImageType type;

		std::vector<uint8_t> bin;

		Result loadPNGw(const wchar_t* _Path);

		const uint8_t* data() const noexcept {
			return bin.data();
		}

		size_t size() const noexcept {
			return bin.size();
		}

	};


}