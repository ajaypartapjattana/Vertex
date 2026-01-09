#pragma once

#include "base/Flag_type.h"

enum class MemoryProperty {
	HostCached = 1 << 0,
	HostVisible = 1 << 1,
	HostCoherent = 1 << 2,
	DeviceLocal = 1 << 3
};

using MemoryPropertyFlags = Flags<MemoryProperty>;

enum class ShaderStageBit {
	VertexBit = 1 << 0,
	GeometryBit = 1 << 1,
	FragmentBit = 1 << 2,
	ComputeBit = 1 << 3,
	AllBit = 1 << 4
};

using ShaderStageFlags = Flags<ShaderStageBit>;

enum class PipelineStage {
	TopOfPipe = 1 << 0,
	Transfer = 1 << 1,
	VertexInput = 1 << 2,
	VertexShader = 1 << 3,
	FragmentShader = 1 << 4,
	EarlyDepthTest = 1 << 5,
	LateDepthTest = 1 << 6,
	ColorAttachmentOutput = 1 << 7,
	BottomOfPipe = 1 << 8
};

using PipelineStageFlags = Flags<PipelineStage>;