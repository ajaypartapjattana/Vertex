#pragma once

#include <glm/glm.hpp>

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

struct AsyncLoader_T;
using AsyncLoader = AsyncLoader_T*;

struct AsyncLoaderCreateInfo {
	size_t stageSize;
	uint32_t maxLoadProcess;
};

int createAsyncLoader(Emulator const _Emulator, const AsyncLoaderCreateInfo* const pCreateInfo, AsyncLoader* const pAsyncLoader) noexcept;
void destroyAsyncLoader(AsyncLoader const _AsynLoader) noexcept;

struct ProcessCookie_T;
using ProcessCookie = ProcessCookie_T*;

int allocateProcessCookie(ProcessCookie* const pProcessCookie) noexcept;
void freeProcessCookie(ProcessCookie const _ProcessCookie) noexcept;

int waitProcess(Emulator const _Emulator, ProcessCookie const _Cookie) noexcept;

struct Scene_T;
using Scene = Scene_T*;

struct SceneCreateInfo {
	uint32_t modelCount;
};

int createScene(Emulator const _Emulator, const SceneCreateInfo* const pCreateInfo, Scene* const pScene) noexcept;
void destroyScene(Scene const _Scene) noexcept;

struct Model_T;
using Model = Model_T*;

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;
};

struct ModelCreateInfo {
	const Vertex* pVertex;
	size_t vertexCount;
};

int loadModel(AsyncLoader const _AsynLoader, Scene const _Scene, const ModelCreateInfo* const pCreateInfo, Model* const pModel, ProcessCookie const _ProcessCookie) noexcept;
void releaseModel(Scene const _Scene, Model const _Model) noexcept;

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
	uint32_t maxRenderProcess;
	uint32_t callDrawLimit;
};

int createRenderer(Emulator const _Emulator, const RendererCreateInfo* const pCreateInfo, Renderer* const pRenderer) noexcept;
void destroyRenderer(Renderer const _Renderer) noexcept;

int waitRenderer(Renderer const _Renderer) noexcept;

int draw(Canvas const _Canvas, Renderer const _Renderer, RenderBox const _RenderBox, Scene const _Scene) noexcept;

