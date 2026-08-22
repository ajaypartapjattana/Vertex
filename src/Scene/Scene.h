#pragma once

#include "Signboard/Core/core.h"
#include "Signboard/Assets/io/io.h"

class SGBResources;

struct scnObj {
	uint32_t mesh;
};

struct scnView {
	sgb::vltView<scnObj> objView;
};

class SGBScene {
public:
	SGBScene();

	_NODISCARD scnView read_scene() const noexcept;	
	
	struct sceneObjectCraeteInfo {
		const char* modelPath;
	};
	void createSceneObject(const sceneObjectCraeteInfo& createInfo);

private:
	sgb::vault<scnObj> m_objects;

};