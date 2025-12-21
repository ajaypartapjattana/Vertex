#pragma once

#include "entityHandlers/model.h"

class Camera;
class Model;

struct InputState {

};

enum PlayerViewMode {
	FirstPerson,
	ThirdPerson
};

class Player
{
public:
	Player(Camera* camera, Model* model);
	~Player();

	void update(float deltaTime, const InputState& input);
	void render();

	void setModel(Model* model);
	Model* getModel() const;

	const glm::vec3& getPosition() const;
	void setPosition(const glm::vec3& pos);

	void setViewMode(PlayerViewMode mode);
	PlayerViewMode getViewMode() const;

	float getYaw() const;
	float getPitch() const;

	void setMoveSpeed(float speed);
	void setMouseSensitivity(float sensitivity);

private:
	void move(float dt, const InputState& input);
	void look(const InputState& input);
	void updateCamera();
	void updateModelTransform();

	Camera* camera = nullptr;
	Model* model = nullptr;

	glm::vec3 position{ 0.0f };
	glm::vec3 velocity{ 0.0f };

	float yaw = -90.0f;
	float pitch = 0.0f;

	PlayerViewMode viewMode = PlayerViewMode::FirstPerson;

	glm::vec3 firstPersonOffset = { 0.0f, 1.7f, 0.0f };
	glm::vec3 thirdPersonOffset = { 0.0f, 1.6f, -3.5f };

	float moveSpeed = 5.0f;
	float mouseSensitvity = 0.1f;

	bool renderModelInFirstPerson = false;

	glm::mat4 modelMatrix{ 1.0f };
};
