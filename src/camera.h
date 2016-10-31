#ifndef CAMERA_H
#define CAMERA_H

#define GLM_SWIZZLE

#include "sceneobject.h"

#include <iostream>

/**
 * @brief A Camera is a SceneObject that maintains a view matrix, as well as
 * parameters defining the projection matrix, i.e. the viewing frustum.
 */
class Camera : public SceneObject
{
	float fieldOfView;
	float aspectRatio;
	float nearPlane;
	float farPlane;

public:
	Camera(const glm::mat4 &matrix_, float fieldOfView_, float aspectRatio_, float nearPlane_, float farPlane_);
	Camera(const glm::mat4 &matrix_);
	virtual ~Camera();

	/**
	 * @brief update the state of the Camera
	 * @param timeDelta the time passed since the last frame in seconds
	 */
	virtual void update(float timeDelta);

	/**
	 * @brief get the view matrix, i.e. the inverse camera model matrix
	 * in which the camera is at the origin and looking in -z direction
	 * @return the view matrix
	 */
	glm::mat4 getViewMatrix() const;

	/**
	 * @brief get the current projection matrix defining the view frustum
	 * calculated from the parameters stored
	 * @return the current projection matrix defining the view frustum
	 */
	glm::mat4 getProjectionMatrix() const;

	/**
	 * @brief get field of view of the viewing frustum
	 * @return the field of view angle in radians
	 */
	float getFieldOfView() const;

	/**
	 * @brief set field of view of the viewing frustum
	 * @param fieldOfView_ the new field of view angle in radians
	 */
	void setFieldOfView(float fieldOfView_);

	/**
	 * @brief get the aspect ratio of the viewing frustum (width / height)
	 * @return the aspect ratio (width / height)
	 */
	float getAspectRatio() const;

	/**
	 * @brief set the aspect ratio of the viewing frustum (width / height)
	 * @param aspectRatio_ the new aspect ratio (width / height)
	 */
	void setAspectRatio(float aspectRatio_);

	/**
	 * @brief get distance from camera to near plane of the viewing frustum
	 * @return the distance from camera to near plane
	 */
	float getNearPlane() const;

	/**
	 * @brief set distance from camera to near plane of the viewing frustum
	 * @param nearPlane_ the new distance from camera to near plane
	 */
	void setNearPlane(float nearPlane_);

	/**
	 * @brief get distance from camera to near plane of the viewing frustum
	 * @return the distance from camera to near plane
	 */
	float getFarPlane() const;

	/**
	 * @brief set distance from camera to far plane of the viewing frustum
	 * @return farPlane_ the new distance from camera to far plane
	 */
	void setFarPlane(float farPlane_);

	/**
	 * @brief rotate camera to look at a given point
	 * @param target the target point to look at. this should not be the camera location.
	 */
	void lookAt(const glm::vec3 &target);

	/**
	 * @brief determine whether a sphere with given center and farthest point in world space
	 * lies completely within the view frustum.
	 * note that the farthest point is passed instead of the radius to apply matrices
	 * to do the checks in clip space.
	 * @param sphereCenter the center of the sphere in world space
	 * @param sphereFarthestPoint the farthest point from sphere center in world space
	 * @return whether the sphere lies completely within the view frustum
	 */
	bool checkSphereInFrustum(const glm::vec3 &sphereCenterWorldSpace, const glm::vec3 &sphereFarthestPointWorldSpace, const glm::mat4 &viewMat);

};

#endif // CAMERA_H
