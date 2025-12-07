#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Input.h"

enum CAMERA_TRANSFORMATION_MODE {
    LOCAL_MODE, WORLD_MODE
};

class Camera
{
public:
    CAMERA_TRANSFORMATION_MODE cameraMode = LOCAL_MODE;
    Camera(glm::vec3 position, float aspectRatio);

    void handleCamera(GLFWwindow* window);

    void setPosition(const glm::vec3 pos);
    void makeCameraLookAt(const glm::vec3 pos);
    void setAspectRatio(float aspect);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getForward() const { return front; }

    float getCameraFov();

private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    glm::vec3 groundForward;
    glm::vec3 groundRight;

    float yaw;
    float pitch;
    float fov;
    float aspect;
    float nearPlane;
    float farPlane;

    struct MovementTable {
        void (Camera::* forward)(float);
        void (Camera::* backward)(float);
        void (Camera::* right)(float);
        void (Camera::* left)(float);
        void (Camera::* up)(float);
        void (Camera::* down)(float);
    };

    MovementTable localMovement;
    MovementTable worldMovement;
    MovementTable* camMode;

    void updateCameraVectors();

    //LOCAL 
    void moveForwardLocal(float delta);
    void moveBackwardLocal(float delta);
    void moveRightLocal(float delta);
    void moveLeftLocal(float delta);
    void moveUpLocal(float delta);
    void moveDownLocal(float delta);

    //WORLD
    void moveForwardWorld(float delta);
    void moveBackwardWorld(float delta);
    void moveRightWorld(float delta);
    void moveLeftWorld(float delta);
    void moveUpWorld(float delta);
    void moveDownWorld(float delta);

    void rotate(float yawDelta, float pitchDelta);
    void zoom(float yOffset);
};
