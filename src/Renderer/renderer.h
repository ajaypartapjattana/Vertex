#pragma once

#include <core/Memory/memory.h>

struct VulkanContext_T;
using VulkanContext = VulkanContext_T*;

int requestVulkanContext(mem::stack* const pScratch, VulkanContext* const pContext) noexcept;
void destroyVulkanContext(VulkanContext const _Context) noexcept;

int enumeratePhysicalDevices(VulkanContext const _Context, uint32_t* const pCount, const char** const pDeviceNames) noexcept;

struct EmulatorCreateInfo {
	void* windowContext;
	uintptr_t windowHandle;
	uint32_t physicalDevice;
};

struct Emulator_T;
using Emulator = Emulator_T*;

int createEmulator(VulkanContext const  _Context, const EmulatorCreateInfo* const pCreateInfo, mem::stack* const pScratch, Emulator* const pEmulator) noexcept;
void destroyEmulator(Emulator const _Emulator) noexcept;

int waitEmulator(Emulator const _Emulator) noexcept;

struct ResourceSet_T;
using ResourceSet = ResourceSet_T*;

struct ResourceSetAllocateInfo {
	uint32_t textureCount;
	uint32_t objectCount;
};

int allocateStaticResourceSet(Emulator const _Emulator, const ResourceSetAllocateInfo* const pAllocateInfo, ResourceSet* const pResourceSet) noexcept;
void resetStaticResourceSet(Emulator const _Emulator, ResourceSet const _ResourceSet) noexcept;

struct RenderObjectCreateInfo {
	const void* pVertex;
	size_t vertexBufferSize;
	const void* pIndex;
	size_t indexBufferSize;
};

struct AsyncLoader_T;
using AsyncLoader = AsyncLoader_T*;

struct AsyncLoaderCreateInfo {
	size_t stageSize;
	uint32_t maxAsyncLoadRate;
};

int createAsyncLoader(Emulator const _Emulator, const AsyncLoaderCreateInfo* const pCreateInfo, AsyncLoader* const pAsyncLoader) noexcept;
void destroyAsyncLoader(AsyncLoader const _AsynLoader) noexcept;

struct Canvas_T;
using Canvas = Canvas_T*;

struct CanvasCreateInfo {
	uint32_t minImageCount;
};

int createCanvas(Emulator const _Emulator, const CanvasCreateInfo* const pCreateInfo, mem::stack* const pScratch, Canvas* const pCanvas) noexcept;
void destroyCanvas(Canvas const _Canvas) noexcept;

int updateCanvas(Canvas const _Canvas) noexcept;

struct RenderBox_T;
using RenderBox = RenderBox_T*;

int createRenderBox(Emulator const _Emulator, Canvas const _Canvas, mem::stack* const pScratch, RenderBox* const pRenderBox) noexcept;
void destroyRenderBox(RenderBox const _RenderBox) noexcept;

struct Renderer_T;
using Renderer = Renderer_T*;

struct RendererCreateInfo {
	uint32_t maxFrameBuffering;
};

int createRenderer(Emulator const _Emulator, Canvas const _Canvas, RenderBox const _RenderBox, const RendererCreateInfo* const pCreateInfo, Renderer* const pRenderer) noexcept;
void destroyRenderer(Renderer const _Renderer) noexcept;

int waitRenderer(Renderer const _Renderer) noexcept;

int draw(Canvas const _Canvas, Renderer const _Renderer, RenderBox const _RenderBox) noexcept;

int defferedLoadObject(AsyncLoader const _AsyncLoader, ResourceSet const _ResourceSet, const RenderObjectCreateInfo* const pCreateInfo) noexcept;

