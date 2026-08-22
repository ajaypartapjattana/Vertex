#pragma once

#include <core/Memory/memory.h>

struct VulkanContext_T;
using VulkanContext = VulkanContext_T*;

int requestVulkanContext(VulkanContext* const pContext) noexcept;
void purgeVulkanContext(VulkanContext const _Context) noexcept;

int enumeratePhysicalDevices(VulkanContext const _Context, uint32_t* const pCount, const char** const pDeviceNames) noexcept;

struct RendererCreateInfo {
	void* windowContext;
	uintptr_t windowHandle;
	uint32_t physicalDevice;
};

struct Emulator_T;
using Emulator = Emulator_T*;

int createEmulator(VulkanContext const  _Context, const RendererCreateInfo* const pCreateInfo, Emulator* const pEmulator) noexcept;
void purgeEmulator(VulkanContext const _Context, Emulator const _Emulator) noexcept;

int createImage(Emulator const _Emulator, const void* const pData, size_t _Size, uint32_t _Width, uint32_t _Height) noexcept;
