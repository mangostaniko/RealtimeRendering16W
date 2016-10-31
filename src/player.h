#ifndef PLAYER_H
#define PLAYER_H

#define GLM_SWIZZLE

#include "geometry.h"
#include "camera.h"

#define ZOOM_MIN 30.0f
#define ZOOM_MAX 80.0f

/**
 * @brief The Player class. This stores the player Geometry and a Camera,
 * as well as a GLFWwindow to handle input.
 */
class Player : public Geometry
{
	// CAMERA SPECS 
	Camera *camera;
	GLFWwindow *window;
	glm::vec3 camDirection;
	glm::vec3 camRight;
	glm::vec3 camUp;
	glm::mat4 viewMat;
	glm::mat4 projMat;

	enum CameraNavigationMode
	{
		FOLLOW_PLAYER,
		FREE_FLY
	};

	static CameraNavigationMode cameraNavMode;
	CameraNavigationMode lastNavMode;
	glm::mat4 lastCamTransform; // safe transformation matrix when changing modes

	float timePassed = 0; // in seconds

	static double scrollY; // amount scrolled since last frame

	/**
	 * @brief check if the camera navigation mode has changed and set camera accordingly
	 * note: currently only works if there are only 2 nav modes
	 */
	void handleNavModeChange();

	/**
	 * @brief glfw callback on mouse scroll
	 * @param window pointer to active window
	 * @param deltaX the scroll delta on scroll axis X
	 * @param deltaY the scroll delta on scroll axis Y
	 */
	static void onScroll(GLFWwindow *window, double deltaX, double deltaY);

	/**
	 * @brief handle input to control camera in free fly mode.
	 * @param window pointer to active window
	 * @param timeDelta the time passed since the last frame in seconds
	 */
	void handleInputFreeCamera(GLFWwindow *window, float timeDelta);


public:
	Player(const glm::mat4 &matrix_, Camera *camera_, GLFWwindow *window_, const std::string &filePath);
	virtual ~Player();

	virtual void update(float timeDelta);
	virtual void draw(Shader *shader, bool useFrustumCulling, Texture::FilterType filterType, const glm::mat4 &viewMat);

	/**
	 * @brief toggle the camera navigation mode
	 */
	void toggleNavMode();

	/**
	 * @brief get the current view matrix of the player camera
	 * @return the current view matrix
	 */
	glm::mat4 getViewMat();

	/**
	 * @brief get the current projection matrix of the player camera
	 * @return the current projection matrix
	 */
	glm::mat4 getProjMat();

};

#endif // PLAYER_H
