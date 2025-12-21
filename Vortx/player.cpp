#include "player.h"

Player::Player(Camera* camera, Model* model) : camera(camera), model(model){

}

Player::~Player() {

}

void Player::update(float deltaTime, const InputState& input) {
	if (!camera)
		return;
	look(input);
	move(deltaTime, input);

	updateCamera();
	updateModelTransform();
}
void Player::render() {
	if (!model)
		return;
	if (viewMode == PlayerViewMode::FirstPerson && !renderModelInFirstPerson)
		return;

	/*model->;
	model->draw();*/
}

void Player::setModel(Model* model) {

}
Model* Player::getModel() const {

}

const glm::vec3& Player::getPosition() const {

}
void Player::setPosition(const glm::vec3& pos) {

}

void Player::setViewMode(PlayerViewMode mode) {

}
PlayerViewMode Player::getViewMode() const {

}

float Player::getYaw() const {

}
float Player::getPitch() const {

}

void Player::setMoveSpeed(float speed) {

}
void Player::setMouseSensitivity(float sensitivity) {

}