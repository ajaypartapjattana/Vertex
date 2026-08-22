#include "Scene.h"

#include "Signboard/Assets/io/io.h"

SGBScene::SGBScene()
{

}

scnView SGBScene::read_scene() const noexcept {
	return { sgb::vltView<scnObj>{ m_objects } };
}

void SGBScene::createSceneObject(const sceneObjectCraeteInfo& createInfo) {
	
}
