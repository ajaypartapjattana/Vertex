#pragma once

#include "base/Flag_type.h"
#include "DeviceTypes.h"

enum class BufferUsage {
	TransferSource		= 1 << 0,
	TransferDestination = 1 << 1,
	Uniform				= 1 << 2,
	Storage				= 1 << 3,
	Vertex				= 1 << 4,
	Index				= 1 << 5
};

using BufferUsageFlags = Flags<BufferUsage>;

struct BufferDesc {
	uint64_t size;
	BufferUsageFlags usageFlags{};
	MemoryPropertyFlags memoryFlags{};
};
