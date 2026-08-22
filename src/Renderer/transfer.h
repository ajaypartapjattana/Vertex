#pragma once

#include <utility>
#include <cstddef>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>
#include <core/Memory/memory.h>

namespace rndr {

	struct DataSource {
		const void* pData;
		size_t size;
	};

	struct TransferImageAttributes {
		VkExtent3D extent;
		uint32_t texelSize;
	};

	struct flushRanges {
		VkDeviceSize offset[2];
		VkDeviceSize size[2];
		size_t count;
	};

	class TransferStage {
	private:
		mem::span<uint8_t> stageSpan;

		uint8_t* pAllocBase = nullptr;
		uint8_t* pAllocTail = nullptr;
		uint8_t* pAllocHead = nullptr;

		struct {
			size_t failures = 0;
		} stats;
		
		struct StageChunk {
			VkDeviceSize offset;
			VkDeviceSize size;
			size_t granularityIndex;
		};

		inline int commitChunk(const DataSource* const pSource, const size_t* const pGranularities, const size_t granularityCount, StageChunk* const pChunk) noexcept;

	public:
		TransferStage() noexcept = default;
		TransferStage(const mem::span<uint8_t>& _Span) noexcept 
			:
			stageSpan(_Span),
			pAllocBase(mem::alignUp<uint8_t>(_Span.pBegin, alignof(uint8_t*))),
			pAllocTail(pAllocBase),
			pAllocHead(pAllocBase + sizeof(uint8_t*))
		{
			*reinterpret_cast<uint8_t**>(pAllocBase) = nullptr;
		}

		void restore() noexcept {
			uint8_t* const pNext = *reinterpret_cast<uint8_t**>(pAllocTail);

			if (!pNext)
				return;
			
			pAllocTail = pNext;
		}

		void flushStream(flushRanges* const pRanges) noexcept {
			assert(pRanges);

			uint8_t* pMarker = mem::alignUp<uint8_t>(pAllocHead, alignof(uint8_t*));
			
			if (pMarker + sizeof(void*) > stageSpan.pEnd)
				pMarker = stageSpan.pBegin;

			*reinterpret_cast<uint8_t**>(pAllocBase) = pMarker;

			pRanges->offset[0] = static_cast<VkDeviceSize>(pAllocBase - stageSpan.pBegin);
			
			if (pAllocHead > pAllocBase) {
				pRanges->size[0] = static_cast<VkDeviceSize>(pAllocHead - pAllocBase);
				pRanges->count = 1;

			}
			else {
				pRanges->size[0] = static_cast<VkDeviceSize>(stageSpan.pEnd - pAllocBase);
				
				pRanges->offset[1] = 0ull;
				pRanges->size[1] = static_cast<VkDeviceSize>(pAllocHead - stageSpan.pBegin);

				pRanges->count = 2;
			}

			pAllocBase = pMarker;
			*reinterpret_cast<uint8_t**>(pAllocBase) = nullptr;

			pAllocHead = pMarker + sizeof(uint8_t*);
		}

		int streamBufferUpload(const DataSource* const pSource, VkDeviceSize* const pOffset, VkBufferCopy* const pRegion) noexcept;
		int streamImageUpload(const DataSource* const pSource, const TransferImageAttributes* const pImageInfo, VkOffset3D* const pOffset, VkBufferImageCopy* const pRegion) noexcept;

		void reset() noexcept {
			pAllocHead = stageSpan.pBegin;
			pAllocBase = nullptr;
		}

	};

}
