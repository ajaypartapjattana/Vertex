#include "transfer.h"

namespace rndr {

	int TransferStage::commitChunk(const DataSource* const pSource, const size_t* const pGranularities, const size_t granularityCount, StageChunk* const pChunk) noexcept {
		assert(pSource && pGranularities && granularityCount && pChunk);

		const size_t availableSize = pAllocHead >= pAllocTail ? static_cast<size_t>(stageSpan.pEnd - pAllocHead) : static_cast<size_t>(pAllocTail - pAllocHead);

		if (pSource->size > availableSize) {
			if(pAllocHead < pAllocTail)
				return VK_NOT_READY;

			size_t transferSize = 0, granularityIndex = 0;

			for (const size_t* pGranularity = pGranularities + granularityCount; pGranularity != pGranularities; --pGranularity) {
				transferSize = mem::alignDown(availableSize, *pGranularity);
				
				if (!transferSize)
					continue;

				granularityIndex = static_cast<size_t>(pGranularity - pGranularities);
				break;
			}

			if (transferSize) {
				memcpy(pAllocHead, pSource->pData, transferSize);

				pChunk->offset = static_cast<VkDeviceSize>(pAllocHead - stageSpan.pBegin);
				pChunk->size = static_cast<VkDeviceSize>(transferSize);
				pChunk->granularityIndex = granularityIndex;

				pAllocHead += transferSize;

				return 1;
			}

			if (pSource->size > static_cast<size_t>(pAllocTail - stageSpan.pBegin))
				return -1;

			pAllocHead = stageSpan.pBegin;
		}

		memcpy(pAllocHead, pSource->pData, pSource->size);

		pChunk->offset = static_cast<VkDeviceSize>(pAllocHead - stageSpan.pBegin);
		pChunk->size = static_cast<VkDeviceSize>(pSource->size);
		pChunk->granularityIndex = 0ull;

		pAllocHead += pSource->size;

		return 0;
	}

	int TransferStage::streamBufferUpload(const DataSource* const pSource, VkDeviceSize* const pOffset, VkBufferCopy* const pRegion) noexcept {
		assert(pSource && pOffset && pRegion && pSource->pData);

		if (!pSource->size)
			return VK_SUCCESS;

		DataSource source = { reinterpret_cast<const uint8_t*>(pSource->pData) + pRegion->srcOffset, pSource->size - pRegion->srcOffset };

		constexpr size_t bufferGranularity = 1ull;

		StageChunk chunk;
		int result = commitChunk(&source, &bufferGranularity, 1, &chunk);

		switch (result) {
		case -1:
			stats.failures++;

			return -1;
		
		case 0:
			pRegion->srcOffset = chunk.offset;
			pRegion->dstOffset = *pOffset;
			pRegion->size = chunk.size;

			*pOffset += chunk.size;

			return 0;

		case 1:
			pRegion->srcOffset = chunk.offset;
			pRegion->dstOffset = *pOffset;
			pRegion->size = chunk.size;

			*pOffset += chunk.size;

			return 1;
			
		}

		return -1;
	}

	int TransferStage::streamImageUpload(const DataSource* const pSource, const TransferImageAttributes* const pImageInfo, VkOffset3D* const pOffset, VkBufferImageCopy* const pRegion) noexcept {
		assert(pSource && pImageInfo && pOffset && pRegion);

		if (!pSource->size)
			return VK_SUCCESS;

		size_t granularity[3]{};
		granularity[0] = pImageInfo->texelSize;
		granularity[1] = pImageInfo->extent.width * granularity[0];
		granularity[2] = pImageInfo->extent.height * granularity[1];

		const size_t sourceOffset = pOffset->z * granularity[2] + pOffset->y * granularity[1] + pOffset->x * granularity[0];

		size_t pending, granularityLimit;

		if (pOffset->x) {
			pending = granularity[0] * ((size_t)pImageInfo->extent.width - pOffset->x);
			granularityLimit = 1;
		}
		else if (pOffset->y) {
			pending = granularity[1] * ((size_t)pImageInfo->extent.height - pOffset->y);
			granularityLimit = 2;
		}
		else {
			pending = granularity[2] * ((size_t)pImageInfo->extent.depth - pOffset->z);
			granularityLimit = 3;
		}

		DataSource source = { reinterpret_cast<const uint8_t*>(pSource->pData) + sourceOffset, pending };
		
		StageChunk chunk;
		int result = commitChunk(&source, granularity, granularityLimit, &chunk);

		switch (result) {
		case -1:
			stats.failures++;

			return -1;

		case 0:
			pRegion->bufferOffset = chunk.offset;
			pRegion->imageOffset = *pOffset;
			pRegion->imageExtent = pImageInfo->extent;

			*pOffset = { 0, 0, (int32_t)pImageInfo->extent.depth };

			return 0;

		case 1: {
			const uint32_t units = static_cast<uint32_t>(chunk.size / granularity[chunk.granularityIndex]);

			VkExtent3D transferExtent{};

			switch (chunk.granularityIndex) {
			case 0:
				transferExtent.width = units;
				transferExtent.height = 1;
				transferExtent.depth = 1;

				break;

			case 1:
				transferExtent.width = pImageInfo->extent.width - pOffset->x;
				transferExtent.height = units;
				transferExtent.depth = 1;

				break;

			case 2:
				transferExtent.width = pImageInfo->extent.width - pOffset->x;
				transferExtent.height = pImageInfo->extent.height - pOffset->y;
				transferExtent.depth = units;

				break;
			}

			pRegion->bufferOffset = chunk.offset;
			pRegion->imageOffset = *pOffset;
			pRegion->imageExtent = transferExtent;

			switch (chunk.granularityIndex) {
			case 0:
				pOffset->x += transferExtent.width;
				
				if (pOffset->x == pImageInfo->extent.width) {
					pOffset->x = 0;
					pOffset->y++;
				}

				if (pOffset->y == pImageInfo->extent.height) {
					pOffset->y = 0;
					pOffset->z++;
				}

				break;

			case 1:
				pOffset->y += transferExtent.height;
				if (pOffset->y == pImageInfo->extent.height) {
					pOffset->y = 0;
					pOffset->z++;
				}

				break;

			case 2:
				pOffset->z += transferExtent.depth;

				break;
			}
		}

			return 1;


		}

		return -1;
	}

}
