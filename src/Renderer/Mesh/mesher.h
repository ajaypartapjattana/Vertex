#pragma once

#include <memory>

#include "Signboard/Core/Interfaces/renderer/model/model.h"

enum class MESH_PRIMITIVE {
	CUBE, NGON, ICOSPHERE, CYLINDER
};

class Mesher {
public:
	Mesher() = default;

	struct PrimitiveInfo {
		float size;
		uint32_t res;
		bool floatingMesh;
	};

	std::unique_ptr<Model> createPrimitive(const MESH_PRIMITIVE primitive, const PrimitiveInfo& info);

private:

};