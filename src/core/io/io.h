#pragma once

#include <stdint.h>

#include <core/Memory/memory.h>

namespace io {

	struct ImageInfo {
		uint32_t width;
		uint32_t height;
		uint16_t channels;
		uint16_t bitDepth;
	};

	inline size_t getImageSize(const ImageInfo* const pImageInfo) noexcept {
		return (((size_t)pImageInfo->bitDepth * pImageInfo->channels * pImageInfo->width) >> 3) * pImageInfo->height;
	}

	int getBinarySize(const char* const _Path, size_t* const pSize) noexcept;
	int loadBinary(const char* const _Path, const size_t _LoadSize, void* const pDst) noexcept;

	int fetchPngInfo(const void* pBin, size_t _Size, ImageInfo* const pInfo) noexcept;

	struct InflatorCreateInfo {
		const void* pStreamSrc;
		size_t StreamSize;
		const ImageInfo* imageInfo;
		void* pDst;
		const void* pLimit;
	};

	struct Inflator_T;
	using Inflator = Inflator_T*;

	void getInflateBufferSize(const ImageInfo* const pImageInfo, size_t* const pSize) noexcept;
	int createInflator(InflatorCreateInfo* const pCreateInfo, mem::span<uint8_t> _ResolveMemory, Inflator* const pInflator) noexcept;

	void destroyInflator(Inflator const _Inflator) noexcept;

	int bindInflateSource(Inflator const _Inflator, const ImageInfo* const pNewImageInfo, const void* const pNewSrc, const size_t _NewSize) noexcept;
	void bindInflateTarget(Inflator const _Inflator, const void* const pDst, const void* const pLimit) noexcept;

	int decodePng(Inflator const _Inflator) noexcept;

}