#pragma once

#include <cstdint>

namespace DESCRIPTOR_SCHEMA {
	struct VIEW_STATE {
		static constexpr uint32_t SET = 0;
		static constexpr uint32_t VIEW_STATE_BINDING = 0;
		static constexpr uint32_t VIEW_STATE_COUNT = 1;
	};

	struct OBJECT_STATE {
		static constexpr uint32_t SET = 1;
		static constexpr uint32_t OBJECT_BUFFER_BINDING = 0;
		static constexpr uint32_t MAX_OBJECT_COUNT = 1024;
	};

	struct MATERIAL_VARIABLES {
		static constexpr uint32_t SET = 2;
		static constexpr uint32_t MATERIAL_BUFFER_BINDING = 0;
		static constexpr uint32_t MAX_MATERIAL_COUNT = 256;
	};

	struct BINDLESS_TEXTURES {
		static constexpr uint32_t SET = 3;
		static constexpr uint32_t TEXTURE_BINDING = 0;
		static constexpr uint32_t SAMPLER_BINDING = 1;
		static constexpr uint32_t MAX_TEXTURES = 512;
		static constexpr uint32_t MAX_SAMPLERS = 32;
		static constexpr uint32_t VARIABLE_COUNT[] = { MAX_TEXTURES, MAX_SAMPLERS };
	};
}
