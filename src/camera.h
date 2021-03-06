#pragma once

#define ZOOM_MIN 30.0f
#define ZOOM_MAX 80.0f

#include <vector>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "sceneobject.hpp"


/**
 * @brief A Camera is a SceneObject that maintains a view matrix, as well as
 * parameters defining the projection matrix, i.e. the viewing frustum,
 * as well as functions for input handling in different camera modes.
 */
class Camera : public SceneObject
{
    public:
    Camera(GLFWwindow *window_, const glm::mat4 &matrix_, float fieldOfView_, float aspectRatio_, float nearPlane_, float farPlane_);
    Camera(GLFWwindow *window_, const glm::mat4 &matrix_);
    virtual ~Camera();

	/// Update the state of the Camera
	virtual void update(
	    float timeDelta, ///< [in] time passed since the last frame in seconds
	    float cameraPathSpeed ///< [in] speed at which camera moves along path
	);

    /**
    * @brief get the view matrix, i.e. the inverse camera model matrix
    * in which the camera is at the origin and looking in -z direction
    * @return the view matrix
    */
    glm::mat4 getViewMat() const;

    /**
    * @brief get the current projection matrix defining the view frustum
    * calculated from the parameters stored
    * @return the current projection matrix defining the view frustum
    */
    glm::mat4 getProjMat() const;

    //! Get field of view of the viewing frustum
    /// \return the field of view angle in radians
    float getFieldOfView() const;

    //! Set field of view of the viewing frustum
    void setFieldOfView(
        float fieldOfView_ //!< [in] the new field of view angle in radians
    );

    //! Get the aspect ratio of the viewing frustum (width/height)
    /// \return the aspect ratio (width/height)
    float getAspectRatio() const;

    //! Set the aspect ratio of the viewing frustum (width / height)
    void setAspectRatio(
        float aspectRatio_ //!< [in] the new aspect ratio (width/height)
    );

    //! Get distance from camera to near plane of the viewing frustum
    /// \return the distance from camera to near plane
    float getNearPlane() const;

    //! Set distance from camera to near plane of the viewing frustum
    void setNearPlane(
        float nearPlane_ //!< [in] the new distance from camera to near plane
    );

    //! Get distance from camera to near plane of the viewing frustum
    /// \return the distance from camera to near plane
    float getFarPlane() const;

    //! Set distance from camera to far plane of the viewing frustum
    /// \return farPlane_ the new distance from camera to far plane
    void setFarPlane(float farPlane_);

    //! Rotate camera to look at a given target point
    /// The target should not be the camera location.
    void lookAt(
        const glm::vec3 &target //!< [in] the target point to look at.
    );

    //! Determine whether a sphere with given center and farthest point in
    //! world space lies completely within the view frustum. Note that the
    //! farthest point is passed instead of the radius to apply matrices
    //! to do the checks in clip space.
    /// \return whether the sphere lies completely within the view frustum
    bool checkSphereInFrustum(
        const glm::vec3 &sphereCenterWorldSpace, //!< [in] the center of the sphere in world space
        const glm::vec3 &sphereFarthestPointWorldSpace, //!< [in] the farthest point from sphere center in world space
        const glm::mat4 &viewMat //!< [in] Viewing matrix
    );

    //! Toggle the camera navigation mode
    void toggleNavMode();
    
	/// clears the current camera path
	inline void clearCameraPath() { cameraPath.clear(); }

	/// appends a number of path segments to current camera path (control points of bezier curve segments).
	/// a path segment is a bezier curve determined via linear interpolation (lerping) of three control points.
	/// for parameter position [0,1] on bezier curve with control points A,B,C: lerp A,B and B,C and lerp the lerped points.
	void appendPath(std::vector<std::vector<glm::vec3> > newPath);

	/// returns the current camera path (control points of bezier curve segments).
	/// a path segment is a bezier curve determined via linear interpolation of three control points.
	/// for parameter position [0,1] on bezier curve with control points A,B,C: lerp A,B and B,C and lerp the lerped points.
	inline const std::vector<std::vector<glm::vec3> >& getPath() const { return cameraPath; }

	/// get target look at position for FOLLOW_PATH mode
	inline const glm::vec3& getTargetLookAtPos() const { return targetLookAtPos; }

	/// set target look at position for FOLLOW_PATH mode
	inline void setTargetLookAtPos(const glm::vec3& lookAtPos) { targetLookAtPos = lookAtPos; }
    
private:

    GLFWwindow *window; //!< GLFW window for input handling

    float fieldOfView;
    float aspectRatio;
    float nearPlane;
    float farPlane;

    enum CameraNavigationMode
    {
        FOLLOW_PATH,
        FREE_FLY
    };

    static CameraNavigationMode cameraNavMode;
    CameraNavigationMode lastNavMode;
    glm::mat4 lastCamTransform; // backup transformation matrix before changing camera mode

    float timePassed = 0; //!< in seconds
    static double scrollY; //!< amount scrolled since last frame
    
	/// update position on bezier path, after some time has passed
	void updateBezierPathCamTransform(const double timeDelta, const double speed);

	std::vector<std::vector<glm::vec3>> cameraPath; ///< bezier path to follow. path segments are defined by 3 control points each.
	int currentPathSegment; ///< current segment (bezier curve) on bezier path
	float pathSegmentLerpFactor; ///< lin. interpolation factor for bezier control points on current path segment. in range [0,1[.
	glm::vec3 targetLookAtPos; ///< target position to look at while in FOLLOW_PATH mode

    /**
    * @brief check if the camera navigation mode has changed and set camera accordingly
    * note: currently only works if there are only 2 nav modes
    */
    void handleNavModeChange();

    //! GLFW callback on mouse scroll
    static void onScroll(
        GLFWwindow *window, //!< [in,out] window pointer to active window
        double deltaX, //!< [in] the scroll delta on scroll axis X
        double deltaY //!< [in] the scroll delta on scroll axis Y
    );

    //! Handle input to control camera in free fly mode.
    void handleInputFreeCamera(
        GLFWwindow *window, //!< [in,out] pointer to active window
        float timeDelta //!< [in] time passed since the last frame in seconds
    );
    
    glm::vec3 mix(const glm::vec3 &a, const glm::vec3 &b, float t);
    
};

