#include "camera.h"

Camera::CameraNavigationMode Camera::cameraNavMode = FREE_FLY;
double Camera::scrollY = 0.0;


Camera::Camera(GLFWwindow *window_, const glm::mat4 &matrix_, float fieldOfView_, float aspectRatio_, float nearPlane_, float farPlane_)
    : SceneObject(matrix_)
    , window(window_)
    , fieldOfView(fieldOfView_)
    , aspectRatio(aspectRatio_)
    , nearPlane(nearPlane_)
    , farPlane(farPlane_)
{
	// set glfw callbacks
	glfwSetScrollCallback(window, onScroll);
	
	currentPathSegment = 0;
	pathSegmentLerpFactor = 0.0f;
	targetLookAtPos = glm::vec3(0, 5, 0);

	lastCamTransform = getMatrix();
}

Camera::Camera(GLFWwindow *window_, const glm::mat4 &matrix_)
    : Camera(window_, matrix_, glm::pi<float>()/3, 4.0f/3.0f, 0.2f, 100.0f)
{
}

Camera::~Camera()
{
	window = nullptr;
}


void Camera::update(float timeDelta, float cameraPathSpeed)
{
	// note: camera navigation mode is toggled on tab key press, look for keyCallback
	handleNavModeChange();

	if (cameraNavMode == FOLLOW_PATH) {
		/* no user movement in follow path mode */

		if (cameraPath.size() > 0)
			updateBezierPathCamTransform(timeDelta, cameraPathSpeed);
	}
	else {
		handleInputFreeCamera(window, timeDelta);
	}

}

glm::mat4 Camera::getViewMat() const
{
	return getInverseMatrix();
}

glm::mat4 Camera::getProjMat() const
{
	return glm::perspective(fieldOfView, aspectRatio, nearPlane, farPlane);
}

float Camera::getFieldOfView() const
{
	return fieldOfView;
}

void Camera::setFieldOfView(float fieldOfView_)
{
	fieldOfView = fieldOfView_;
}

float Camera::getAspectRatio() const
{
	return aspectRatio;
}

void Camera::setAspectRatio(float aspectRatio_)
{
	aspectRatio = aspectRatio_;
}

float Camera::getNearPlane() const
{
	return nearPlane;
}

void Camera::setNearPlane(float nearPlane_)
{
	nearPlane = nearPlane_;
}

float Camera::getFarPlane() const
{
	return farPlane;
}

void Camera::setFarPlane(float farPlane_)
{
	farPlane = farPlane_;
}

void Camera::lookAt(const glm::vec3 &target)
{
	setTransform(glm::lookAt(getLocation(), target, glm::vec3(0, 1, 0)));
}

bool Camera::checkSphereInFrustum(const glm::vec3 &sphereCenterWorldSpace, const glm::vec3 &sphereFarthestPointWorldSpace, const glm::mat4 &viewMat)
{
	// get sphere into clip space and then into normalized device coordinates through perspective division,
	// i.e. division by w which after the perspective projection stores the depth component z,
	// such that all 6 view frustum planes are simply at distance 1 or -1 from the origin
	glm::vec4 center = getProjMat() * viewMat * glm::vec4(sphereCenterWorldSpace, 1);
	glm::vec4 sphereFarthestPoint = getProjMat() * viewMat * glm::vec4(sphereFarthestPointWorldSpace, 1);
	center /= center.w;
	sphereFarthestPoint /= sphereFarthestPoint.w;

	// note: if needed, use a greater radius to avoid that objects disappear
	// whose shadows are still in view, as well as to compensate the rough bounding sphere approximation
	float radius = glm::length(sphereFarthestPoint.xy() - center.xy());
	float distanceZ = abs(sphereFarthestPoint.z - center.z);

	// check if the sphere lies beyond any of the 6 view frustum planes
	float plane = 1.0f;
	if (center.x - radius > plane || center.x + radius < -plane) return false;
	if (center.y - radius > plane || center.y + radius < -plane) return false;
	if (center.z - distanceZ > plane || center.z + distanceZ < -plane) return false;

	return true;
}

/* FOLLOW PATH MODE */

void Camera::appendPath(std::vector<std::vector<glm::vec3> > newPath)
{
	if (cameraPath.size() > 0) {
		// to connect new path to existing path, align terminal segments
		// by taking arithmetic mean of their central control points for both of the outermost control points
		glm::vec3 connectingPoint = mix(cameraPath[cameraPath.size()-1][1], newPath[0][1], 0.5);
		cameraPath[cameraPath.size()-1][2] = connectingPoint;
		newPath[0][0] = connectingPoint;
	}

	// append path
	for (auto iter = newPath.begin(); iter != newPath.end(); ++iter) {
		cameraPath.push_back(*iter);
	}
}

void Camera::updateBezierPathCamTransform(const double timeDelta, const double speed)
{
	// to get a point on a bezier curve (segment) with three control points A,B,C:
	// for parameter position (linear inpolation factor) [0,1] on curve: lerp A,B and B,C and lerp the lerped points.

	pathSegmentLerpFactor += timeDelta * speed;

	if (pathSegmentLerpFactor > 1.0) {
		// jump to next bezier curve (next path segment)
		currentPathSegment = (currentPathSegment+1) % cameraPath.size();
		pathSegmentLerpFactor -= 1.0;
	}
	//std::cout << "camera path segment current/last: " << currentPathSegment << "/" << cameraPath.size()-1 << ", lerp: " << pathSegmentLerpFactor << std::endl;

	// get segment control points
	glm::vec3 A = cameraPath[currentPathSegment][0];
	glm::vec3 B = cameraPath[currentPathSegment][1];
	glm::vec3 C = cameraPath[currentPathSegment][2];

	// determine bezier curve position
	glm::vec3 lerpPosAB = mix(A, B, pathSegmentLerpFactor);
	glm::vec3 lerpPosBC = mix(B, C, pathSegmentLerpFactor);
	glm::vec3 bezierPathPos = mix(lerpPosAB, lerpPosBC, pathSegmentLerpFactor);

	setTransform(glm::inverse(glm::lookAt(bezierPathPos, targetLookAtPos, glm::vec3(0, 1, 0))));

}

/* INPUT HANDLING */

void Camera::handleInputFreeCamera(GLFWwindow *window, float timeDelta)
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
		translate(getMatrix()[2].xyz * -timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, 'S')) {
		translate(getMatrix()[2].xyz * timeDelta * moveSpeed, SceneObject::LEFT);
	}

	if (glfwGetKey(window, 'A')) {
		translate(getMatrix()[0].xyz * -timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, 'D')) {
		translate(getMatrix()[0].xyz * timeDelta * moveSpeed, SceneObject::LEFT);
	}

	if (glfwGetKey(window, 'Q')) {
		translate(glm::vec3(0,1,0) * timeDelta * moveSpeed, SceneObject::LEFT);
	}
	else if (glfwGetKey(window, 'E')) {
		translate(glm::vec3(0,1,0) * -timeDelta * moveSpeed, SceneObject::LEFT);
	}

	// rotate camera based on mouse movement
	// the mouse pointer is reset to (0, 0) every frame, and we just take the displacement of that frame
	const float mouseSensitivity = 0.01f;
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	rotateX(-mouseSensitivity * (float)mouseY, SceneObject::RIGHT); // rotate around local x axis (tilt up/down)
	glm::vec3 location = getLocation();
	translate(-location, SceneObject::LEFT);
	rotateY(-mouseSensitivity * (float)mouseX, SceneObject::LEFT); // rotate around global y at local position
	translate(location, SceneObject::LEFT);
	glfwSetCursorPos(window, 0, 0); // reset the mouse, so it doesn't leave the window

	// handle camera zoom by changing the field of view depending on mouse scroll since last frame
	float zoomSensitivity = -0.1f;
	float fieldOfView = getFieldOfView() + zoomSensitivity * (float)scrollY;
	if (fieldOfView < glm::radians(ZOOM_MIN)) fieldOfView = glm::radians(ZOOM_MIN);
	if (fieldOfView > glm::radians(ZOOM_MAX)) fieldOfView = glm::radians(ZOOM_MAX);
	setFieldOfView(fieldOfView);
	scrollY = 0.0;

}

void Camera::onScroll(GLFWwindow *window, double deltaX, double deltaY)
{
	scrollY += deltaY;
}

void Camera::toggleNavMode()
{
	if (cameraNavMode == FOLLOW_PATH) {
		cameraNavMode = FREE_FLY;
	}
	else if (cameraNavMode == FREE_FLY) {
		cameraNavMode = FOLLOW_PATH;
	}
}

void Camera::handleNavModeChange()
{
	if (cameraNavMode == lastNavMode) {
		return;
	}

	glm::mat4 temp = getMatrix();
	setTransform(lastCamTransform);
	lastCamTransform = temp;

	lastNavMode = cameraNavMode;
}

glm::vec3 Camera::mix(const glm::vec3 &a, const glm::vec3 &b, float t) 
{
    return a * (1.0f - t) + b * t;
}

