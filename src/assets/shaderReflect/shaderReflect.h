#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <unordered_map>

#include <external/SPIRV_cross/spirv_cross.hpp>

constexpr bool config_Validate =
#ifdef _VALIDATE
	true;
#else
	false;
#endif

struct VertexLayoutDesc {
	std::vector<VkVertexInputAttributeDescription> attributes;
	std::vector<VkVertexInputBindingDescription> bindings;
};

struct BindingKey {
	uint32_t set;
	uint32_t binding;

	bool operator==(const BindingKey& other) const noexcept {
		return set == other.set && binding == other.binding;
	}
};
struct BindingKeyHash {
	size_t operator()(const BindingKey& k) const noexcept {
		return (size_t(k.set) << 32) | k.binding;
	}
};
using BindingMap = std::unordered_map<BindingKey, VkDescriptorSetLayoutBinding, BindingKeyHash>;

class ShaderReflect {
private:
	std::unique_ptr<spirv_cross::Compiler> m_compiler;

	VkFormat mapTypeToFormat(const spirv_cross::SPIRType& type) const noexcept(!config_Validate) {
		if (type.basetype == spirv_cross::SPIRType::Float) {
			switch (type.vecsize) {
			case 1: return VK_FORMAT_R32_SFLOAT;
			case 2: return VK_FORMAT_R32G32_SFLOAT;
			case 3: return VK_FORMAT_R32G32B32_SFLOAT;
			case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
		}

		if (type.basetype == spirv_cross::SPIRType::Int) {
			switch (type.vecsize) {
			case 1: return VK_FORMAT_R32_SINT;
			case 2: return VK_FORMAT_R32G32_SINT;
			case 3: return VK_FORMAT_R32G32B32_SINT;
			case 4: return VK_FORMAT_R32G32B32A32_SINT;
			}
		}

		if (type.basetype == spirv_cross::SPIRType::UInt) {
			switch (type.vecsize) {
			case 1: return VK_FORMAT_R32_UINT;
			case 2: return VK_FORMAT_R32G32_UINT;
			case 3: return VK_FORMAT_R32G32B32_UINT;
			case 4: return VK_FORMAT_R32G32B32A32_UINT;
			}
		}

		return VK_FORMAT_UNDEFINED;
	}

	uint32_t getFormatSize(VkFormat format) const noexcept(!config_Validate) {
		switch (format) {

		case VK_FORMAT_R8_UNORM:
		case VK_FORMAT_R8_SNORM:
		case VK_FORMAT_R8_UINT:
		case VK_FORMAT_R8_SINT:
			return 1;

		case VK_FORMAT_R16_UNORM:
		case VK_FORMAT_R16_SNORM:
		case VK_FORMAT_R16_UINT:
		case VK_FORMAT_R16_SINT:
		case VK_FORMAT_R16_SFLOAT:
			return 2;

		case VK_FORMAT_R8G8_UNORM:
		case VK_FORMAT_R8G8_SNORM:
		case VK_FORMAT_R8G8_UINT:
		case VK_FORMAT_R8G8_SINT:
			return 2;

		case VK_FORMAT_R32_UINT:
		case VK_FORMAT_R32_SINT:
		case VK_FORMAT_R32_SFLOAT:
			return 4;

		case VK_FORMAT_R16G16_UNORM:
		case VK_FORMAT_R16G16_SNORM:
		case VK_FORMAT_R16G16_UINT:
		case VK_FORMAT_R16G16_SINT:
		case VK_FORMAT_R16G16_SFLOAT:
			return 4;

		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_R8G8B8A8_UINT:
		case VK_FORMAT_R8G8B8A8_SINT:
			return 4;

		case VK_FORMAT_R32G32_UINT:
		case VK_FORMAT_R32G32_SINT:
		case VK_FORMAT_R32G32_SFLOAT:
			return 8;

		case VK_FORMAT_R16G16B16A16_UNORM:
		case VK_FORMAT_R16G16B16A16_SNORM:
		case VK_FORMAT_R16G16B16A16_UINT:
		case VK_FORMAT_R16G16B16A16_SINT:
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return 8;

		case VK_FORMAT_R32G32B32_UINT:
		case VK_FORMAT_R32G32B32_SINT:
		case VK_FORMAT_R32G32B32_SFLOAT:
			return 12;

		case VK_FORMAT_R32G32B32A32_UINT:
		case VK_FORMAT_R32G32B32A32_SINT:
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return 16;

		default:
#ifdef _VALIDATE
			throw std::runtime_error("Unsupported VkFormat in getFormatSize");
#else
			return 0;
#endif
		}
	}

public:
	ShaderReflect() = default;
	ShaderReflect::ShaderReflect(const std::vector<uint32_t>& bin) {
		compile(bin);
	}
	ShaderReflect(const ShaderReflect&) = delete;
    ShaderReflect(ShaderReflect&&) = delete;

    ShaderReflect& operator=(const ShaderReflect&) = delete;
    ShaderReflect& operator=(ShaderReflect&&) = delete;

	void compile(const std::vector<uint32_t>& bin) {
		if (bin.empty())
			throw std::runtime_error("INVALID: bin_empty!");

		m_compiler = std::make_unique<spirv_cross::Compiler>(bin);
	}

	void compile(const sgb::span<const uint32_t> bin) {
		if (bin.empty())
			throw std::runtime_error("INVALID: bin_empty!");

		m_compiler = std::make_unique<spirv_cross::Compiler>(bin.data(), bin.size());
	}

	VertexLayoutDesc reflectVertexLayout() const noexcept(!config_Validate) {
		VertexLayoutDesc layout{};

		if (!m_compiler)
			return layout;

		const auto& res = m_compiler->get_shader_resources();

		struct TempAttr {
			uint32_t location;
			VkFormat format;
			uint32_t size;
		};

		std::vector<TempAttr> temp;
		temp.reserve(res.stage_inputs.size());

		for (const spirv_cross::Resource& input : res.stage_inputs) {
			uint32_t location = m_compiler->get_decoration(input.id, spv::DecorationLocation);
			const auto& type = m_compiler->get_type(input.type_id);

			VkFormat format = mapTypeToFormat(type);
			uint32_t size = getFormatSize(format);

			temp.push_back({ location, format, size });
		}

		std::sort(temp.begin(), temp.end(), [](const TempAttr& a, const TempAttr& b) {
			return a.location < b.location;
			});

		layout.attributes.reserve(temp.size());

		uint32_t offset = 0;
		for (const auto& attr : temp) {
			VkVertexInputAttributeDescription desc{};

			desc.location = attr.location;
			desc.binding = 0;
			desc.format = attr.format;
			desc.offset = offset;

			layout.attributes.push_back(desc);

			offset += attr.size;
		}

		VkVertexInputBindingDescription binding{};
		binding.binding = 0;
		binding.stride = offset;
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		layout.bindings.push_back(binding);

		return layout;
	}

	BindingMap reflectDescriptorBindings(VkShaderStageFlags stage) const noexcept(!config_Validate) {
		BindingMap merged;

		if (!m_compiler)
			return merged;

		const auto& res = m_compiler->get_shader_resources();

		size_t _tdSz =
			res.uniform_buffers.size()
			+ res.storage_buffers.size()
			+ res.sampled_images.size()
			+ res.separate_images.size()
			+ res.separate_samplers.size()
			+ res.storage_images.size();

		merged.reserve(_tdSz);

		auto process = [&](const spirv_cross::SmallVector<spirv_cross::Resource>& resources, VkDescriptorType type) {
			for (const auto& r : resources) {
				uint32_t set = m_compiler->get_decoration(r.id, spv::DecorationDescriptorSet);
				uint32_t binding = m_compiler->get_decoration(r.id, spv::DecorationBinding);

				const auto& spirType = m_compiler->get_type(r.type_id);

				uint32_t count = 1;
				for (size_t dim = 0; dim < spirType.array.size(); ++dim) {
					if (spirType.array_size_literal[dim])
						count *= spirType.array[dim];
					else
						count *= 1;
				}

				BindingKey key{ set, binding };

				auto [it, inserted] = merged.try_emplace(key, VkDescriptorSetLayoutBinding{ binding, type, count, stage, nullptr });

				if (!inserted) {
					it->second.stageFlags |= stage;

#ifdef _VALIDATE
					if (it->second.descriptorType != type) {
						throw std::logic_error("LOGIC: descriptor_type_mismatch!");
					}
#endif
				}
			}
			};

		process(res.uniform_buffers, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		process(res.storage_buffers, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		process(res.sampled_images, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		process(res.separate_images, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
		process(res.separate_samplers, VK_DESCRIPTOR_TYPE_SAMPLER);
		process(res.storage_images, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

		return merged;
	}

	std::vector<VkPushConstantRange> reflectPushConstants(VkShaderStageFlags stage) const noexcept(!config_Validate) {
		std::vector<VkPushConstantRange> result;

		if (!m_compiler)
			return result;

		spirv_cross::ShaderResources res = m_compiler->get_shader_resources();

		for (const spirv_cross::Resource& pc : res.push_constant_buffers) {
			const spirv_cross::SPIRType& type = m_compiler->get_type(pc.base_type_id);
			uint32_t size = static_cast<uint32_t>(m_compiler->get_declared_struct_size(type));

			VkPushConstantRange range{};
			range.stageFlags = stage;
			range.size = size;
			range.offset = 0;

			result.push_back(range);
		}

		return result;
	}

};
