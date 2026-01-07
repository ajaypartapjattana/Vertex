#pragma once

#include "base/Flag_type.h"

enum class BufferUsage {
	TransferSource		= 1 << 0,
	TransferDestination = 1 << 1,
	Uniform				= 1 << 2,
	Storage				= 1 << 3,
	Vertex				= 1 << 4,
	Index				= 1 << 5
};

using BufferUsageFlags = Flags<BufferUsage>;

enum class MemoryProperty {
	HostCached			= 1 << 0,
	HostVisible			= 1 << 1,
	HostCoherent		= 1 << 2,
	DeviceLocal			= 1 << 3
};

using MemoryPropertyFlags = Flags<MemoryProperty>;

struct BufferDesc {
	uint64_t size;
	BufferUsageFlags usageFlags{};
	MemoryPropertyFlags memoryFlags{};
};
