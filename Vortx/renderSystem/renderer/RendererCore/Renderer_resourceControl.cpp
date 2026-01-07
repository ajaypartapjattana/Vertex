#include "Renderer.h"

ModelID Renderer::createModel(ModelDesc& desc) {
	TextureHandle albedo = INVALID_TEXTURE;
	TextureHandle normal = INVALID_TEXTURE;

	if (!desc.albedoPath.empty()) {
		albedo = textureSystem.loadTexture(desc.albedoPath);
	}
	if (!desc.normalPath.empty()) {
		normal = textureSystem.loadTexture(desc.normalPath);
	}

	MaterialDesc matDesc{};
	matDesc.passID = forwardPass.passID;
	matDesc.name = desc.materialName;
	matDesc.pipeline = forwardPass.pipeID;
	matDesc.albedo = albedo;
	matDesc.normal = normal;
	matDesc.params = desc.materialParams;

	MaterialID material = materialSystem.registerMaterial(matDesc);

	MeshDesc meshDesc{};
	meshDesc.p_vertices = desc.vertices;
	meshDesc.vertexSize = desc.vertexSize;
	meshDesc.vertexCount = desc.vertexCount;
	meshDesc.p_indices = desc.indices;
	meshDesc.indexCount = desc.indexCount;

	MeshHandle mesh = meshSystem.createMesh(meshDesc);

	if (mesh == INVALID_MESH || material == INVALID_MATERIAL)
		return INVALID_MODEL;

	ObjectDesc objDesc{};
	objDesc.passID = forwardPass.passID;
	objDesc.mesh = mesh;
	objDesc.material = material;
	objDesc.transform = desc.transform;

	return objectSystem.createObject(objDesc);
}