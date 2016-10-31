#include "player.h"

Player::CameraNavigationMode Player::cameraNavMode = FREE_FLY;
double Player::scrollY = 0.0;

Player::Player(const glm::mat4 &matrix_, Camera *camera_, GLFWwindow *window_, const std::string &filePath)
    : Geometry(matrix_, filePath)
    , camera(camera_)    
    , window(window_)
{
	// set glfw callbacks
	glfwSetScrollCallback(window, onScroll);

	camera->setTransform(glm::translate(glm::mat4(1.0f), getLocation()+glm::vec3(0,2,6)));  //move camera back a bit
	lastCamTransform = camera->getMatrix();
	camDirection = glm::normalize(camera->getLocation() - getLocation());
	camUp = glm::vec3(0, 1, 0);
	camRight = glm::normalize(glm::cross(camUp, camDirection));

}

Player::~Player()
{
	delete camera; camera = nullptr;
	window = nullptr;
}

void Player::update(float timeDelta)
{

	// note: camera navigation mode is toggled on tab key press, look for keyCallback
	handleNavModeChange();

	if (cameraNavMode == FOLLOW_PLAYER) {
		handleInputFreeCamera(window, timeDelta);
		glm::vec3 v = glm::normalize(getLocation() - camera->getLocation()) * 5.0f;
		viewMat = glm::lookAt(getLocation()-v, getLocation() + glm::vec3(0, 1, 0), camUp);
	}
	else {
		handleInputFreeCamera(window, timeDelta);
		viewMat = camera->getViewMatrix();
	}

	projMat = camera->getProjectionMatrix();

}

void Player::draw(Shader *shader, bool useFrustumCulling, Texture::FilterType filterType, const glm::mat4 &viewMat)
{
	Geometry::draw(shader, camera, useFrustumCulling, filterType, viewMat);
}


void Player::handleInputFreeCamera(GLFWwindow *window, float timeDelta)
{

	float moveSpeed = 10.0f;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		moveSpeed = 50.0f;
	}
	
	//////////////////////////
	/// CAMERA MOVEMENT
	//////////////////////////

	// camera movement
	// note: we apply rotation before translation since we dont want the distance from the origin
	// to affect how we rotate
    if (glfwGetKey(window, 'W')) {
		camera->translate(camera->getMatrix()[2].xyz * -timeDelta * moveSpeed, SceneObject::LEFT);
    }
	else if (glfwGetKey(window, 'S')) {
		camera->translate(camera->getMatrix()[2].xyz * timeDelta * moveSpeed, SceneObject::LEFT);
	}

	if (glfwGetKey(window, 'A')) {
		camera->translate(camera->getMatrix()[0].xyz * -timeDelta * moveSpeed, SceneObject::LEFT);
    }
	else if (glfwGetKey(window, 'D')) {
		camera->translate(camera->getMatrix()[0].xyz * timeDelta * moveSpeed, SceneObject::LEFT);
    }

	if (glfwGetKey(window, 'Q')) {
	    camera->translate(glm::vec3(0,1,0) * timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, 'E')) {
	    camera->translate(glm::vec3(0,1,0) * -timeDelta * moveSpeed, SceneObject::LEFT);
	}

	// rotate camera based on mouse movement
	// the mouse pointer is reset to (0, 0) every frame, and we just take the displacement of that frame
	const float mouseSensitivity = 0.01f;
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	camera->rotateX(-mouseSensitivity * (float)mouseY, SceneObject::RIGHT); // rotate around local x axis (tilt up/down)
	glm::vec3 location = camera->getLocation();
	camera->translate(-location, SceneObject::LEFT);
	camera->rotateY(-mouseSensitivity * (float)mouseX, SceneObject::LEFT); // rotate around global y at local position
	camera->translate(location, SceneObject::LEFT);
	glfwSetCursorPos(window, 0, 0); // reset the mouse, so it doesn't leave the window

	// handle camera zoom by changing the field of view depending on mouse scroll since last frame
	float zoomSensitivity = -0.1f;
	float fieldOfView = camera->getFieldOfView() + zoomSensitivity * (float)scrollY;
	if (fieldOfView < glm::radians(ZOOM_MIN)) fieldOfView = glm::radians(ZOOM_MIN);
	if (fieldOfView > glm::radians(ZOOM_MAX)) fieldOfView = glm::radians(ZOOM_MAX);
	camera->setFieldOfView(fieldOfView);
	scrollY = 0.0;

}

void Player::onScroll(GLFWwindow *window, double deltaX, double deltaY)
{
	scrollY += deltaY;
}

void Player::toggleNavMode()
{
	if (cameraNavMode == FOLLOW_PLAYER) {
		cameraNavMode = FREE_FLY;
	}
	else if (cameraNavMode == FREE_FLY) {
		cameraNavMode = FOLLOW_PLAYER;
	}
}

void Player::handleNavModeChange()
{
	if (cameraNavMode == lastNavMode) {
		return;
	}

	glm::mat4 temp = camera->getMatrix();
	camera->setTransform(lastCamTransform);
	lastCamTransform = temp;

	lastNavMode = cameraNavMode;
}

glm::mat4 Player::getViewMat()
{
	return viewMat;
}

glm::mat4 Player::getProjMat()
{
	return projMat;
}
