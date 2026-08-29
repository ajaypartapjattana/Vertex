#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

namespace Vertex {

	struct _2D {
		glm::vec2 position;
		glm::vec2 uv;
		unsigned char color[4];

		static constexpr std::array<VkVertexInputAttributeDescription, 3> attributes() noexcept {
			return { {
				{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(_2D, position)},
				{1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(_2D, uv)},
				{2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(_2D, color)}
			} };
		};

		static constexpr VkVertexInputBindingDescription binding() noexcept {
			return { 0, sizeof(_2D), VK_VERTEX_INPUT_RATE_VERTEX };
		};
	};

	struct _3D {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;

		static constexpr std::array<VkVertexInputAttributeDescription, 3> attributes() noexcept {
			return { {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(_3D, position)},
				{1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(_3D, normal)},
				{2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(_3D, uv)}
			} };
		};

		static constexpr VkVertexInputBindingDescription binding() noexcept {
			return { 0, sizeof(_3D), VK_VERTEX_INPUT_RATE_VERTEX };
		};
	};

	struct _3D_Tangent {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec4 tangent;

		static constexpr std::array<VkVertexInputAttributeDescription, 4> attributes() noexcept {
			return { {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(_3D_Tangent, position)},
				{1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(_3D_Tangent, normal)},
				{2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(_3D_Tangent, uv)},
				{3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(_3D_Tangent, tangent)}
			} };
		}

		static constexpr VkVertexInputBindingDescription binding() noexcept {
			return { 0, sizeof(_3D_Tangent), VK_VERTEX_INPUT_RATE_VERTEX };
		}
	};

	struct _3D_Skinned {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec4 tangent;

		uint32_t boneIndices[4];
		float boneWeights[4];

		static constexpr std::array<VkVertexInputAttributeDescription, 6> attributes() noexcept {
			return { {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(_3D_Skinned, position)},
				{1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(_3D_Skinned, normal)},
				{2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(_3D_Skinned, uv)},
				{3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(_3D_Skinned, tangent)},
				{4, 0, VK_FORMAT_R32G32B32A32_UINT, offsetof(_3D_Skinned, boneIndices)},
				{5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(_3D_Skinned, boneWeights)}
			} };
		}

		static constexpr VkVertexInputBindingDescription binding() noexcept {
			return { 0, sizeof(_3D_Skinned), VK_VERTEX_INPUT_RATE_VERTEX };
		}

	};

	struct _Debug {
		glm::vec3 position;
		uint32_t color;

		static constexpr std::array<VkVertexInputAttributeDescription, 2> attributes() noexcept {
			return { {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(_Debug, position)},
				{1, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(_Debug, color)}
			} };
		}

		static constexpr VkVertexInputBindingDescription binding() noexcept {
			return { 0, sizeof(_Debug), VK_VERTEX_INPUT_RATE_VERTEX };
		}

	};

}

