#include "camera.h"

Camera::Camera(const glm::mat4 &matrix_, float fieldOfView_, float aspectRatio_, float nearPlane_, float farPlane_)
	: SceneObject(matrix_)
    , fieldOfView(fieldOfView_)
    , aspectRatio(aspectRatio_)
    , nearPlane(nearPlane_)
    , farPlane(farPlane_)
{
}

Camera::Camera(const glm::mat4 &matrix_)
	: SceneObject(matrix_)
    , fieldOfView(glm::pi<float>()/3)
    , aspectRatio(4.0f/3.0f)
    , nearPlane(0.2f)
    , farPlane(100.0f)
{

}

Camera::~Camera()
{

}


void Camera::update(float timeDelta)
{
	/* left empty */
}


glm::mat4 Camera::getViewMatrix() const
{
	return getInverseMatrix();
}

glm::mat4 Camera::getProjectionMatrix() const
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
	glm::vec4 center = getProjectionMatrix() * viewMat * glm::vec4(sphereCenterWorldSpace, 1);
	glm::vec4 sphereFarthestPoint = getProjectionMatrix() * viewMat * glm::vec4(sphereFarthestPointWorldSpace, 1);
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

