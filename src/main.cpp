#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <random>
#include <ctime>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "sceneobject.hpp"
#include "camera.h"
#include "eagle.h"
#include "light.h"
#include "textrenderer.h"
#include "effects/ssao_effect.h"
#include "effects/water_effect.h"
#include "effects/lightbeams_effect.h"
#include "effects/particlesystem.h"
#include "effects/skybox_effect.h"

void init(GLFWwindow *window);
void initSM();
void initVSM();
void initPCFSM();
void initVSMBlur();
void shadowPrepass(glm::mat4 &lightViewPro);
void vsmBlurPass();
void debugShadowPass();
void ssaoPrepass();
void waterPrepass();
void lightbeamsPrepass();
void mainGeometryDrawPass();
void update(float timeDelta);
void draw();
void setActiveShader(Shader *shader);
void drawGeometry();
void drawWater();
void drawLightbeams();
void drawText();
void drawScreenFillingQuad();
void cleanup();
void newGame();

GLFWwindow *window;
int windowWidth, windowHeight;
double deltaT;
bool running                    = true;
bool paused                     = false;
bool debugInfoEnabled           = false;
bool ssaoEnabled                = false;
bool ssaoBlurEnabled            = false;
bool shadowsEnabled             = true;
bool vsmShadowsEnabled          = true;
bool renderShadowMap            = false;
bool frustumCullingEnabled      = false;
bool drawWireframe              = false;
bool drawTransparent            = false;
bool drawLightbeamsDebug        = false;
bool drawSkyboxEnabled          = true;
bool sunColorChangeEnabled      = true;

Texture::FilterType textureFilterMethod = Texture::LINEAR_MIPMAP_LINEAR;

// uniform buffer object to share common uniform data between shaders
// NOTE: it seems that on the gpu or opengl implementation the offsets
// i.e. the size of uniform blocks in gpu memory must be multiples of 256
// otherwise glBindBufferRange yields error 1281 (invalid value).
GLuint commonShaderUniformsUBO;
GLuint uboMatricesBlockSize = 256; //2 * sizeof(glm::mat4);
GLuint uboViewAndCamBlockSize = 256; //5 * sizeof(glm::vec4);
GLuint uboTotalSize = uboMatricesBlockSize + uboViewAndCamBlockSize;

Shader *texturedBlinnPhongShader, *flatSingleColorShader;
Shader *depthMapShader, *vsmDepthMapShader, *debugDepthShader, *blurVSMDepthShader; // shadow mapping
Shader *activeShader;
TextRenderer *textRenderer;
SSAOEffect *ssaoEffect;
WaterEffect *waterEffect;
LightbeamsEffect *lightbeamsEffect;
ParticleSystem *particlesFire;
ParticleSystem *particlesSmoke;
SkyboxEffect *skyboxEffect;

Camera *camera; glm::mat4 cameraInitTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 10, 50)));

Eagle *eagle; glm::mat4 eagleInitTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 30, -45)));
Geometry *island;
Geometry *campfire;
Geometry *ocean;

Light *sun; // sun start and end positions are linearly interpolated over time of day
const glm::vec3 LIGHT_START(glm::vec3(-20, 30, -100));
const glm::vec3 LIGHT_END(glm::vec3(50, 30, -100));
const float dayLength = 60;
const float cameraFollowPathSpeed = 0.3f;

// Shadow Map FBO and depth texture
const int SM_WIDTH = 2048, SM_HEIGHT = 2048;
const GLfloat SM_NEAR_PLANE = 0.5f, SM_FAR_PLANE = 500.f;
GLuint depthMapFBO, vsmDepthMapFBO;
GLuint depthMap, vsmDepthMap;
GLuint pingpongFBO;
GLuint pingpongColorMap;

void frameBufferResize(GLFWwindow *window, int width, int height);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

GLuint quadVAO = 0;
GLuint quadVBO;


int main(int argc, char **argv)
{
	// HANDLE COMMAND LINE PARAMETERS

	windowWidth = 1440;
	windowHeight = 900;
	int refresh_rate = 60;
	bool fullscreen = 0;

	if (argc == 1) {
		// no parameters specified, continue with default values

	} else if (argc != 4 || (std::stringstream(argv[1]) >> windowWidth).fail() || (std::stringstream(argv[2]) >> windowHeight).fail() || (std::stringstream(argv[3]) >> fullscreen).fail()) {
		// if parameters are specified, must conform to given format

		std::cout << "USAGE: <resolution width> <resolution height> <fullscreen? 0/1>\n";
		exit(EXIT_FAILURE);
	}

	// INIT WINDOW AND OPENGL CONTEXT

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_REFRESH_RATE, refresh_rate);
	glfwWindowHint(GLFW_SAMPLES, 16);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *videoMode = glfwGetVideoMode(monitor);

	window = nullptr;
	window = glfwCreateWindow(windowWidth, windowHeight, "SUZANNE ISLAND", (fullscreen ? monitor : NULL), NULL);
	if (!window)
	{
		std::cerr << "ERROR: Failed to open GLFW window.\n";
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// center window on screen
	glfwSetWindowPos(window, videoMode->width/2 - windowWidth/2, videoMode->height/2 - windowHeight/2);

	glfwMakeContextCurrent(window);

	// capture mouse pointer and hide it
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, 0, 0);

	// Clear Color and set Viewport
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glViewport(0, 0, windowWidth, windowHeight);

	// print OpenGL version
	std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << glewGetErrorString(err);
	}

	// set callbacks
	glfwSetFramebufferSizeCallback(window, frameBufferResize);
	glfwSetKeyCallback(window, keyCallback);

	// most initializations happen here
	init(window);

	//////////////////////////
	/// MAIN LOOP
	//////////////////////////

	double time = 0.0;
	double lastTime = 0.0;
	deltaT = 0.0;

	while (running && !glfwWindowShouldClose(window)) {

		if (glfwGetTime() < lastTime) {
			lastTime = 0;
		}
		time = glfwGetTime(); // seconds
		deltaT = time - lastTime;
		lastTime = time;

		// print fps in window title
		std::stringstream ss;
		ss << "SUZANNE ISLAND [" << int(1/deltaT + 0.5) << " FPS]";
		glfwSetWindowTitle(window, ss.str().c_str());

		//////////////////////////
		/// UPDATE
		//////////////////////////
		if (!paused) {
			update(deltaT);
		}


		//////////////////////////
		/// DRAW
		//////////////////////////

		draw();


		//////////////////////////
		/// ERRORS AND EVENTS
		//////////////////////////
		GLenum glErr = glGetError();
		if (glErr != GL_NO_ERROR) {
			std::cerr << "ERROR: OpenGL Error " << glErr << std::endl;
		}

		glfwPollEvents();

		if (running) {
			running = !glfwGetKey(window, GLFW_KEY_ESCAPE);
		}
	}

	// release resources
	cleanup();

	glfwTerminate();
	exit(EXIT_SUCCESS); // for system independent success code
	return 0; // to silence compiler warnings
}


void init(GLFWwindow *window)
{
	// enable z buffer test
	glEnable(GL_DEPTH_TEST);

	// AUTOMATIC GAMMA CORRECTION
	// use sRGB color space, which is gamma corrected.
	// color textures are typically stored in sRGB, output should also be in sRGB color space.
	// however our calculations are done in linear space.
	// enabling GL_FRAMEBUFFER_SRGB will tell OpenGL to transform sRGB texture to linear space
	// and transform back for the framebuffer write in the end.
	// thus we dont need to do manual gamma correction in fragment shader.
	glEnable(GL_FRAMEBUFFER_SRGB);

	// Enable clipping plane 0 for vertex clipping, to limit rendering to parts of scene.
	// this is set individually in each vertex shader via gl_ClipDistance[0].
	// plane is defined as oriented distance from origin, via a normalized direction vector and a distance.
	// we check if parallel component (cosine) of vertex is beyond the distance, i.e. dot(vertpos, direction) > d.
	// using negative plane distance in a vec4 (x,y,z,-d) the dot product will be the offset of vertpos distance to plane distance
	// thus simply check dot(vertpos, direction) > 0 to see if we are beyond or before the plane in oriented direction.
	// gl_ClipDistance[0] takes exactly the result of such a dot product clipping all that lies before the plane.
	// HOWEVER it defines the plane pointing towards the origin thus the result must be inverted.
	// the values are then interpolated for the fragment shader.
	glEnable(GL_CLIP_DISTANCE0);

	// INIT SHADOW MAPPING (FBO, Texture, Shader)
	initSM();

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	paused = false;

	// INIT TEXT RENDERER
	textRenderer = new TextRenderer("data/fonts/cliff.ttf", width, height);

	// INIT EFFECTS
	ssaoEffect = new SSAOEffect(width, height, 32);
	waterEffect = new WaterEffect(width, height, 0.5f, 0.8f, "data/models/water/waterDistortionDuDv.png", 0.02f, 0.03f);
	lightbeamsEffect = new LightbeamsEffect(width, height);

	// INIT SKYBOX
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	std::vector<const GLchar*> cubemapImgPaths;
	cubemapImgPaths.push_back("data/skybox/right.tga");
	cubemapImgPaths.push_back("data/skybox/left.tga");
	cubemapImgPaths.push_back("data/skybox/top.tga");
	cubemapImgPaths.push_back("data/skybox/bottom.tga");
	cubemapImgPaths.push_back("data/skybox/front.tga");
	cubemapImgPaths.push_back("data/skybox/back.tga");
	skyboxEffect = new SkyboxEffect(cubemapImgPaths);

	// INIT PARTICLES
	// maxParticleCount, spawnRate (per second), timeToLive (seconds), gravity
	particlesFire = new ParticleSystem(glm::mat4(1.0f), "data/particles/smoke.png", 30000, 20.f, 3.f, -0.10f);
	particlesFire->respawn(glm::vec3(0.0f, 7.3f, 0.0f));
	particlesSmoke = new ParticleSystem(glm::mat4(1.0f), "data/particles/smoke.png", 3000, 10.f, 15.f, -0.10f);
	particlesSmoke->respawn(glm::vec3(0.0f, 10.0f, 0.0f));

	// INIT SHADERS
	flatSingleColorShader = new Shader("shaders/flat_singlecolor.vert", "shaders/flat_singlecolor.frag");
	texturedBlinnPhongShader = new Shader("shaders/textured_blinnphong.vert", "shaders/textured_blinnphong.frag");
	setActiveShader(texturedBlinnPhongShader); // non-trivial cost

	// INIT UNIFORM BUFFER OBJECT
	// For often reused uniform data (viewMat, ProjMat, lightData, etc.).
	// Allows for different shader programs to access same uniforms from gpu memory block.
	// So modifying a single buffer is enough to update specified uniforms in multiple shaders.
	// Otherwise we would need calls to bind uniforms and assign the same data for each shader individually.
	// This uses a very specific memory layout called std140.
	// The compiler is not allowed to pack this layout for optimization,
	// to ensure it stays the same for all shader programs that use it.
	// MAKE SURE TO MAINTAIN UNIFORM BLOCK LAYOUT ACROSS ALL SHADERS FILES!
	// THIS LAYOUT ONLY ALLOWS VECTORS TO BE VEC2 OR VEC4
	// (VEC3 ARE PADDED, BUT DONT RELY ON GL IMPLEMENTATION FOR IT) !!
	// ORDER OF UNIFORMS WITHIN BLOCK MUST BE CONSISTENT !
	glGenBuffers(1, &commonShaderUniformsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, commonShaderUniformsUBO);
	glBufferData(GL_UNIFORM_BUFFER, uboTotalSize, NULL, GL_STATIC_DRAW); // allocate bytes of memory, no data assigned
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// UBO binding works as follows:
	// UBO <---> Binding Point <---> Shader Uniform Block Index
	// Binding Points can be set explicitly in the shader block layout, thus we just need to bind:
	// UBO <---> Binding Point
	// Note that a single UBO can store data for multiple different shader uniform blocks
	// by binding certain ranges of the UBO memory to different binding locations.
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, commonShaderUniformsUBO, 0, uboMatricesBlockSize); // binding point 0, ubo offset, range
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, commonShaderUniformsUBO, uboMatricesBlockSize, uboViewAndCamBlockSize);

	// To assign UBO data use the following:
	//glBindBuffer(GL_UNIFORM_BUFFER, <ubo>);
	//glBufferSubData(GL_UNIFORM_BUFFER, <offset>, <size>, &<data>);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);



	// NOTE: the following initializations are intended to be used with a shader
	// that uses the following attribute location layout
	// positionAttribIndex   = 0;
	// normalAttribIndex     = 1;
	// uvAttribIndex         = 2;

	// INIT WORLD + OBJECTS
	sun = new Light(glm::translate(glm::mat4(1.0f), LIGHT_START), "data/models/sphere.dae", LIGHT_END, dayLength);
	island = new Geometry(glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1)), "data/models/island/island.dae");
	campfire = new Geometry(glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(1.3, 1.2, 1.3)), glm::vec3(0, 5.7f, 0)), "data/models/campfire/campfire.dae");
	ocean = new Geometry(glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1)), "data/models/water/water.dae");

	// INIT CAMERA

	// camera bezier path to follow in FOLLOW_PATH mode
	// each bezier path segment is defined by three control points.
	std::vector<std::vector<glm::vec3>> cameraPathApproaching(1);
	cameraPathApproaching[0].push_back(glm::vec3(100, 50, 100));
	cameraPathApproaching[0].push_back(glm::vec3(80, 30, 80));
	cameraPathApproaching[0].push_back(glm::vec3(60, 8, 60));

	float bezierPathCircleRadius = 50; float bezierPathCircleHeight = 5;
	std::vector<std::vector<glm::vec3>> cameraPathCircling(4);
	cameraPathCircling[0].push_back(glm::vec3(bezierPathCircleRadius, bezierPathCircleHeight, 0));
	cameraPathCircling[0].push_back(glm::vec3(bezierPathCircleRadius, bezierPathCircleHeight, bezierPathCircleRadius));
	cameraPathCircling[0].push_back(glm::vec3(0,bezierPathCircleHeight,bezierPathCircleRadius));
	cameraPathCircling[1].push_back(glm::vec3(0,bezierPathCircleHeight,bezierPathCircleRadius));
	cameraPathCircling[1].push_back(glm::vec3(-bezierPathCircleRadius*1.4f,bezierPathCircleHeight,bezierPathCircleRadius*1.4f));
	cameraPathCircling[1].push_back(glm::vec3(-bezierPathCircleRadius*1.4f,bezierPathCircleHeight,0));
	cameraPathCircling[2].push_back(glm::vec3(-bezierPathCircleRadius*1.4f,bezierPathCircleHeight,0));
	cameraPathCircling[2].push_back(glm::vec3(-bezierPathCircleRadius*1.4f,bezierPathCircleHeight*4,-bezierPathCircleRadius*1.4f));
	cameraPathCircling[2].push_back(glm::vec3(0,bezierPathCircleHeight*4,-bezierPathCircleRadius*1.4f));
	cameraPathCircling[3].push_back(glm::vec3(0,bezierPathCircleHeight*4,-bezierPathCircleRadius*1.4f));
	cameraPathCircling[3].push_back(glm::vec3(bezierPathCircleRadius*1.4f,bezierPathCircleHeight*4,-bezierPathCircleRadius*1.4));
	cameraPathCircling[3].push_back(glm::vec3(bezierPathCircleRadius*1.4f,bezierPathCircleHeight*4,0));

	camera = new Camera(window, cameraInitTransform, glm::radians(90.0f), width/(float)height, 0.2f, 600.0f); // mat, fov, aspect, znear, zfar
	camera->appendPath(cameraPathApproaching);
	camera->appendPath(cameraPathCircling);
	camera->setTargetLookAtPos(glm::vec3(0, 8, 0));

	// INIT EAGLE
	eagle = new Eagle(eagleInitTransform, "data/models/eagle/eagle.dae");

	printf("FINISHED MODEL LOADING\n");

	glfwSetTime(0);
}


void initSM()
{
	// SM Shaders
	depthMapShader = new Shader("shaders/depth_shader.vert", "shaders/depth_shader.frag");
	debugDepthShader = new Shader("shaders/quad_debug.vert", "shaders/quad_debug.frag");
	vsmDepthMapShader = new Shader("shaders/depth_shader_vsm.vert", "shaders/depth_shader_vsm.frag");
	blurVSMDepthShader = new Shader("shaders/blur_vsm.vert", "shaders/blur_vsm.frag");

	initVSM();
	//initPCFSM();
	initVSMBlur();
}


void initPCFSM()
{
	// INIT SHADOW MAPPING (Framebuffer + ShadowMap + Shaders)
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);

	// ShadowMap
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, SM_WIDTH, SM_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// SM Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void initVSM()
{
	// INIT SHADOW MAPPING (Framebuffer + ShadowMap + Shaders)
	glGenFramebuffers(1, &vsmDepthMapFBO);

	// ShadowMomentsMap
	glGenTextures(1, &vsmDepthMap);
	glBindTexture(GL_TEXTURE_2D, vsmDepthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SM_WIDTH, SM_HEIGHT, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glGenerateMipmap(GL_TEXTURE_2D);

	// SM Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, vsmDepthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, vsmDepthMap, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void initVSMBlur()
{
	glGenFramebuffers(1, &pingpongFBO);
	glGenTextures(1, &pingpongColorMap);

	glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO);
	glBindTexture(GL_TEXTURE_2D, pingpongColorMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SM_WIDTH, SM_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorMap, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void update(float timeDelta)
{
	camera->update(timeDelta, cameraFollowPathSpeed);

	sun->update(timeDelta, sunColorChangeEnabled);

	eagle->update(timeDelta, camera->getLocation() + glm::vec3(0, 2, 0), true, false);

	particlesFire->update(timeDelta, camera->getViewMat());
	particlesSmoke->update(timeDelta, camera->getViewMat());

	waterEffect->updateWaves(timeDelta);

	///////////////////////////////////////////////////////////
	//// SET SHARED UNIFORM DATA VIA UNIFORM BUFFER OBJECT ////
	///////////////////////////////////////////////////////////

	glBindBuffer(GL_UNIFORM_BUFFER, commonShaderUniformsUBO);

	GLint m4sz = sizeof(glm::mat4);
	GLint v4sz = sizeof(glm::vec4);

	// UBO data for VertexShaderUniforms shader uniform block at memory range bound to binding point 0
	glBufferSubData(GL_UNIFORM_BUFFER, 0 + 0*m4sz, m4sz, glm::value_ptr(camera->getViewMat())); // view mat
	glBufferSubData(GL_UNIFORM_BUFFER, 0 + 1*m4sz, m4sz, glm::value_ptr(camera->getProjMat())); // proj mat

	// UBO data for LightAndCam shader uniform block at memory range bound to binding point 1
	GLint offset = uboMatricesBlockSize;
	glBufferSubData(GL_UNIFORM_BUFFER, offset + 0*v4sz, v4sz, glm::value_ptr(glm::vec4(sun->getLocation(), 1)));     // light world pos
	glBufferSubData(GL_UNIFORM_BUFFER, offset + 1*v4sz, v4sz, glm::value_ptr(glm::vec4(sun->getColor() * 0.3f, 1))); // ambient
	glBufferSubData(GL_UNIFORM_BUFFER, offset + 2*v4sz, v4sz, glm::value_ptr(glm::vec4(sun->getColor(), 1)));        // diffuse
	glBufferSubData(GL_UNIFORM_BUFFER, offset + 3*v4sz, v4sz, glm::value_ptr(glm::vec4(sun->getColor() * 0.8f, 1))); // specular
	glBufferSubData(GL_UNIFORM_BUFFER, offset + 4*v4sz, v4sz, glm::value_ptr(glm::vec4(camera->getLocation(), 1)));  // cam world pos

	// unbind UBO
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

void draw()
{

	////////////////////////////////////
	/// PRE PASS
	/// DRAW TO OTHER FRAME BUFFERS
	////////////////////////////////////

	if (shadowsEnabled) {
		glm::mat4 lightVPMat;
		shadowPrepass(lightVPMat);

		// set uniforms for shadow mapping in default textureShader
		setActiveShader(texturedBlinnPhongShader);
		glUniformMatrix4fv(activeShader->getUniformLocation("lightVPMat"), 1, GL_FALSE, glm::value_ptr(lightVPMat));
		glUniform1i(activeShader->getUniformLocation("shadowMap"), 1); // bind tex unit 0 to tex location 0 of blinn phong shader
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, vsmDepthMap);
	}

	if (ssaoEnabled)
		ssaoPrepass();

	waterPrepass();
	lightbeamsPrepass();

	////////////////////////////////////
	/// MAIN PASS
	/// DRAW TO DEFAULT FRAMEBUFFER
	////////////////////////////////////

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(sun->getColor().x, sun->getColor().y, sun->getColor().z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (drawSkyboxEnabled)
		skyboxEffect->drawSkybox(camera->getViewMat(), camera->getProjMat());

	// draw geometry that depends on depth test
	mainGeometryDrawPass();

	// draw screenspace effects
	drawWater();
	drawLightbeams();

	// draw shadow map for debugging
	if (shadowsEnabled && renderShadowMap) {
		debugShadowPass();
	}

	// draw text
	drawText();

	// end the current frame (swaps the front and back buffers)
	glfwSwapBuffers(window);

}

void shadowPrepass(glm::mat4 &lightViewPro)
{
	setActiveShader(texturedBlinnPhongShader);

	// Calculate Light View-Projection Matrix
	glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, SM_NEAR_PLANE, SM_FAR_PLANE);
	//glm::mat4 lightProjection = glm::perspective(100.f, (GLfloat) SM_WIDTH / (GLfloat) SM_HEIGHT, nearPlane, farPlane);
	glm::mat4 lightView = glm::lookAt(sun->getLocation(), glm::vec3(0.f), glm::vec3(0, 1, 0));
	lightViewPro = lightProjection * lightView;

	// set viewport and bind framebuffer
	glViewport(0, 0, SM_WIDTH, SM_HEIGHT);

	//if (vsmShadowsEnabled) {
	glBindFramebuffer(GL_FRAMEBUFFER, vsmDepthMapFBO);
	setActiveShader(vsmDepthMapShader);
	glUniformMatrix4fv(activeShader->getUniformLocation("lightVPMat"), 1, GL_FALSE, glm::value_ptr(lightViewPro));
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	drawGeometry();
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (vsmShadowsEnabled) {
		vsmBlurPass();
	}
	/*}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		setActiveShader(depthMapShader);
		glClear(GL_DEPTH_BUFFER_BIT);
		glUniformMatrix4fv(activeShader->getUniformLocation("lightVPMat"), 1, GL_FALSE, glm::value_ptr(lightViewPro));
		drawScene();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	*/
	// bind default FB and reset viewport
	glViewport(0, 0, windowWidth, windowHeight);
}

void vsmBlurPass()
{
	GLboolean horizontal = true;

	glViewport(0, 0, SM_WIDTH, SM_HEIGHT);
	setActiveShader(blurVSMDepthShader);

	glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO);
	glUniform1i(activeShader->getUniformLocation("horizontal"), horizontal);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, vsmDepthMap);
	drawScreenFillingQuad();
	horizontal = !horizontal;

	glBindFramebuffer(GL_FRAMEBUFFER, vsmDepthMapFBO);
	glUniform1i(activeShader->getUniformLocation("horizontal"), horizontal);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pingpongColorMap);
	drawScreenFillingQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void waterPrepass()
{

	// GENERATE REFLECTION TEXTURE
	// render all geometry above the water surface
	// see textured blinnphong vertex shader for clipping plane format
	waterEffect->bindReflectionFrameBuffer();
	glClearColor(sun->getColor().x, sun->getColor().y, sun->getColor().z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform4f(activeShader->getUniformLocation("clippingPlane"), 0, -1, 0, ocean->getLocation().y); // clip all below water

	// use camera to look in the reflected direction from the mirrored position under the water surface
	glUniform2f(activeShader->getUniformLocation("useYMirroredCamera"), true, ocean->getLocation().y);
	if (drawSkyboxEnabled)
		skyboxEffect->drawSkybox(camera->getViewMat(), camera->getProjMat());
	setActiveShader(texturedBlinnPhongShader);
	drawGeometry();
	glUniform2f(activeShader->getUniformLocation("useYMirroredCamera"), false, ocean->getLocation().y);

	// GENERATE REFRACTION TEXTURE
	// render all geometry below the water surface
	waterEffect->bindRefractionFrameBuffer();
	glClearColor(sun->getColor().x, sun->getColor().y, sun->getColor().z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform4f(activeShader->getUniformLocation("clippingPlane"), 0, 1, 0, -ocean->getLocation().y); // clip all above water
	if (drawSkyboxEnabled)
		skyboxEffect->drawSkybox(camera->getViewMat(), camera->getProjMat());
	setActiveShader(texturedBlinnPhongShader);
	drawGeometry();

	waterEffect->bindDefaultFrameBuffer();

}

void lightbeamsPrepass()
{
	// GENERATE OCCLUSION TEXTURE
	// occlusion texture stores sky colors and is black where sky is occluded
	// render all geometry (all possible occluders) in black

	setActiveShader(flatSingleColorShader);

	lightbeamsEffect->bindOcclusionFrameBuffer();
	glClearColor(sun->getColor().x, sun->getColor().y, sun->getColor().z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform3f(activeShader->getUniformLocation("color"), 0.0f, 0.0f, 0.0f);
	drawGeometry();

	// draw light source geometry in white
	glUniform3f(activeShader->getUniformLocation("color"), 1.0f, 1.0f, 1.0f);
	sun->draw(activeShader, camera, frustumCullingEnabled, textureFilterMethod, camera->getViewMat());

	lightbeamsEffect->bindDefaultFrameBuffer();
}


void ssaoPrepass()
{

	//// SSAO PREPASS
	//// draw ssao input data (screen colors and view space positions) to framebuffer textures
	ssaoEffect->bindScreenDataFramebuffer();

	glClearColor(sun->getColor().x, sun->getColor().y, sun->getColor().z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniform1i(activeShader->getUniformLocation("useShadows"), 0);
	glUniform1i(activeShader->getUniformLocation("useSSAO"), 0);
	glUniform1i(activeShader->getUniformLocation("useVSM"), 0);

	drawGeometry();

	//// SSAO PASS
	//// draw ssao output data to framebuffer texture
	ssaoEffect->calulateSSAOValues(camera->getProjMat());

	//// SSAO BLUR PASS
	if (ssaoBlurEnabled)
		ssaoEffect->blurSSAOResultTexture();

	setActiveShader(texturedBlinnPhongShader);

}

void drawGeometry()
{

	//////////////////////////////////////////////////
	/// DEFAULT ACTIVE SHADER TEXTURED BLINN-PHONG
	//////////////////////////////////////////////////

	Geometry::drawnSurfaceCount = 0;

	if (drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // enable wireframe

	glUniform1f(activeShader->getUniformLocation("drawTransparent"), drawTransparent);
	glUniform3f(activeShader->getUniformLocation("material.specular"), 0.2f, 0.2f, 0.2f);

	// we need to disable back face culling for palm leaves which are not closed meshes
	glDisable(GL_CULL_FACE);
	glUniform1f(activeShader->getUniformLocation("material.shininess"), 64.f);
	island->draw(activeShader, camera, frustumCullingEnabled, textureFilterMethod, camera->getViewMat());
	glEnable(GL_CULL_FACE);

	campfire->draw(activeShader, camera, frustumCullingEnabled, textureFilterMethod, camera->getViewMat());

	glUniform1f(activeShader->getUniformLocation("material.shininess"), 32.f);
	eagle->draw(activeShader, camera, frustumCullingEnabled, textureFilterMethod, camera->getViewMat());

	if (drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // disable wireframe

}


void drawWater()
{
	Shader *waterShader = waterEffect->setupWaterShader();
	ocean->draw(waterShader, camera, frustumCullingEnabled, textureFilterMethod, camera->getViewMat());
	waterShader = nullptr;
}

void drawLightbeams()
{
	glEnable(GL_BLEND); // blend result onto default framebuffer
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	if (drawLightbeamsDebug)
		glBlendFunc(GL_ONE, GL_ZERO);

	Shader *lightbeamsShader = lightbeamsEffect->setupLightbeamsShader(sun->getLocation(), camera->getProjMat()*camera->getViewMat());
	drawScreenFillingQuad(); // draw whole screen with lightbeams shader
	lightbeamsShader = nullptr;

	glDisable(GL_BLEND);
}

void drawText()
{
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND); // blend result onto default framebuffer
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (debugInfoEnabled) {

		int startY = 400;
		int deltaY = 20;
		float fontSize = 0.35f;
		textRenderer->renderText("drawn surface count: " + std::to_string(Geometry::drawnSurfaceCount), 25, startY+2*deltaY, fontSize, glm::vec3(1));
		textRenderer->renderText("delta time: " + std::to_string(int(deltaT*1000 + 0.5)) + " ms", 25, startY+3*deltaY, fontSize, glm::vec3(1));
		textRenderer->renderText("fps: " + std::to_string(int(1/deltaT + 0.5)), 25, startY+4*deltaY, fontSize, glm::vec3(1));

		if (!paused) {
			textRenderer->renderText("time until end of day: " + std::to_string(int(dayLength - glfwGetTime())), 25.0f, startY+6*deltaY, fontSize, glm::vec3(1));
		}
	}

	if (paused) {
		textRenderer->renderText("PAUSED", 25.0f, 150.0f, 0.7f, glm::vec3(1.0f, 0.35f, 0.7f));
	}

	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);
}


void mainGeometryDrawPass()
{
	setActiveShader(texturedBlinnPhongShader);

	glUniform1i(activeShader->getUniformLocation("useShadows"), shadowsEnabled);
	glUniform1i(activeShader->getUniformLocation("useSSAO"), ssaoEnabled);
	glUniform1i(activeShader->getUniformLocation("useVSM"), vsmShadowsEnabled);
	glUniform4f(activeShader->getUniformLocation("clippingPlane"), 0.0f, 0.0f, 0.0f, 0.0f); // no clipping

	ssaoEffect->bindSSAOResultTexture(activeShader->getUniformLocation("ssaoTexture"), 2); // tex location 2 of blinn phong shader
	drawGeometry();

	// draw light source geometry
	setActiveShader(flatSingleColorShader);
	glUniform3f(activeShader->getUniformLocation("color"), 1.0f, 1.0f, 1.0f);
	sun->draw(activeShader, camera, frustumCullingEnabled, textureFilterMethod, camera->getViewMat());

	// draw fireplace particles
	particlesFire->draw(camera->getViewMat(), camera->getProjMat(), glm::vec3(0.4f, 0.25f, 0.2f)); // color
	//particlesSmoke->draw(camera->getViewMat(), camera->getProjMat(), glm::vec3(0.1f, 0.1f, 0.1f)); // color

}


void debugShadowPass()
{

	setActiveShader(debugDepthShader);

	glUniform1f(debugDepthShader->getUniformLocation("near_plane"), SM_NEAR_PLANE);
	glUniform1f(debugDepthShader->getUniformLocation("far_plane"), SM_FAR_PLANE);
	glUniform1i(activeShader->getUniformLocation("depthMap"), 0); // bind tex unit 0 to tex location 0 of debug depth shader
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, vsmDepthMap);

	drawScreenFillingQuad();

}

void drawScreenFillingQuad()
{
	// draw a screen filling quad
	// i.e. draw the whole screen
	// useful e.g. for postprocessing effects
	// NOTE: [-1,1] at zero depth are the edges of the screen in normalized device coordinates
	// shaders must ensure that quad vertices are not transformed

	if (quadVAO == 0) {
		// define quad
		GLfloat quadVertices[] = {
		    // positions         // texture coords
		    -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
		    -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
		     1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
		     1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		};
		// setup quad vao
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	// draw
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void newGame()
{

	glfwSetTime(0);

	delete sun;
	sun = new Light(glm::translate(glm::mat4(1.0f), LIGHT_START), "data/models/sphere.dae", LIGHT_END, dayLength);

	// RESET CAMERA
	camera->setTransform(cameraInitTransform);
	eagle->setTransform(eagleInitTransform);
	eagle->resetEagle();

	// RESPAWN PARTICLES
	particlesFire->respawn(glm::vec3(0.0f, 7.3f, 0.0f));
	particlesSmoke->respawn(glm::vec3(0.0f, 10.0f, 0.0f));

	paused = false;
}


void cleanup()
{
	delete texturedBlinnPhongShader;
	delete flatSingleColorShader;
	delete depthMapShader;
	delete debugDepthShader;
	delete vsmDepthMapShader;
	delete blurVSMDepthShader;

	delete textRenderer;
	delete ssaoEffect;
	delete waterEffect;
	delete lightbeamsEffect;
	delete particlesFire;
	delete particlesSmoke;
	delete skyboxEffect;

	delete camera;
	delete eagle;
	delete island;
	delete campfire;
	delete ocean;
}


/**
 * @brief glfw callback function for when the frame buffer (or window) gets resized
 * @param window pointer to active window
 * @param width new framebuffer width
 * @param height new framebuffer height
 */
void frameBufferResize(GLFWwindow *window, int width, int height)
{
	std::cout << "FRAMEBUFFER RESIZED: " << width << ", " << height << std::endl;
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, windowWidth, windowHeight);
	ssaoEffect->setupFramebuffers(windowWidth, windowHeight);
}


/**
 * @brief glfw keyCallback on key events
 * @param window pointer to active window
 * @param key the key code of the key that caused the event
 * @param scancode a system and platform specific constant
 * @param action type of key event: GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
 * @param mods modifier keys held down on event
 */
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
		camera->toggleNavMode();
	}

	if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
		newGame();
		std::cout << "GAME RESTARTED" << std::endl;
	}

	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
		debugInfoEnabled = !debugInfoEnabled;
		if (debugInfoEnabled) {
			std::cout << "DEBUG INFO ENABLED" << std::endl;
		}
		else {
			std::cout << "DEBUG INFO DISABLED" << std::endl;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
		drawWireframe = !drawWireframe;
		if (drawWireframe) {
			std::cout << "DRAW WIREFRAME ENABLED" << std::endl;
		}
		else {
			std::cout << "DRAW WIREFRAME DISABLED" << std::endl;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
		textureFilterMethod = static_cast<Texture::FilterType>((static_cast<int>(textureFilterMethod)+3) % 6);

		switch (textureFilterMethod) {
			case Texture::NEAREST_MIPMAP_OFF:
				std::cout << "TEXTURE FILTER NEAREST" << std::endl;
				break;
			case Texture::NEAREST_MIPMAP_NEAREST:
				std::cout << "TEXTURE FILTER NEAREST" << std::endl;
				break;
			case Texture::NEAREST_MIPMAP_LINEAR:
				std::cout << "TEXTURE FILTER NEAREST" << std::endl;
				break;
			case Texture::LINEAR_MIPMAP_OFF:
				std::cout << "TEXTURE FILTER LINEAR" << std::endl;
				break;
			case Texture::LINEAR_MIPMAP_NEAREST:
				std::cout << "TEXTURE FILTER LINEAR" << std::endl;
				break;
			case Texture::LINEAR_MIPMAP_LINEAR:
				std::cout << "TEXTURE FILTER LINEAR" << std::endl;
				break;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS) {
		int filterTypeInt = static_cast<int>(textureFilterMethod);
		textureFilterMethod = static_cast<Texture::FilterType>((static_cast<int>(textureFilterMethod)+1) % 3 + (filterTypeInt/3)*3);

		switch (textureFilterMethod) {
			case Texture::NEAREST_MIPMAP_OFF:
				std::cout << "MIPMAP OFF" << std::endl;
				break;
			case Texture::NEAREST_MIPMAP_NEAREST:
				std::cout << "MIPMAP FILTER NEAREST" << std::endl;
				break;
			case Texture::NEAREST_MIPMAP_LINEAR:
				std::cout << "MIPMAP FILTER LINEAR" << std::endl;
				break;
			case Texture::LINEAR_MIPMAP_OFF:
				std::cout << "MIPMAP OFF" << std::endl;
				break;
			case Texture::LINEAR_MIPMAP_NEAREST:
				std::cout << "MIPMAP FILTER NEAREST" << std::endl;
				break;
			case Texture::LINEAR_MIPMAP_LINEAR:
				std::cout << "MIPMAP FILTER LINEAR" << std::endl;
				break;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F6) == GLFW_PRESS) {
		sunColorChangeEnabled = !sunColorChangeEnabled;
		if (sunColorChangeEnabled) {
			std::cout << "SUN DAYTIME COLOR CHANGE ENABLED" << std::endl;
		} else {
			std::cout << "SUN DAYTIME COLOR CHANGE ENABLED" << std::endl;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F7) == GLFW_PRESS) {
		drawSkyboxEnabled = !drawSkyboxEnabled;
		if (drawSkyboxEnabled) {
			std::cout << "DRAW SKYBOX ENABLED" << std::endl;
		} else {
			std::cout << "DRAW SKYBOX DISABLED" << std::endl;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F8) == GLFW_PRESS) {
		drawLightbeamsDebug = !drawLightbeamsDebug;
		if (drawLightbeamsDebug) {
			std::cout << "DRAW LIGHTBEAMS DEBUG ENABLED" << std::endl;
		} else {
			std::cout << "DRAW LIGHTBEAMS DEBUG DISABLED" << std::endl;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F9) == GLFW_PRESS) {
		if (!shadowsEnabled) {
			shadowsEnabled = !shadowsEnabled;
			vsmShadowsEnabled = false;
			std::cout << "PCF SHADOWS ENABLED" << std::endl;
		}
		else if (shadowsEnabled) {
			if (vsmShadowsEnabled) {
				shadowsEnabled = !shadowsEnabled;
				std::cout << "SHADOWS DISABLED" << std::endl;
			}
			else {
				vsmShadowsEnabled = true;
				std::cout << "VSM SHADOWS ENABLED" << std::endl;
			}
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS) {
		frustumCullingEnabled = !frustumCullingEnabled;
		if (frustumCullingEnabled) {
			std::cout << "VIEW FRUSTUM CULLING ENABLED" << std::endl;
		}
		else {
			std::cout << "VIEW FRUSTUM CULLING DISABLED" << std::endl;
		}
	}

	/*
	if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS) {
		drawTransparent = !drawTransparent;
		if (drawTransparent) {
			std::cout << "TRANSPARENCY ENABLED" << std::endl;
		}
		else {
			std::cout << "TRANSPARENCY DISABLED" << std::endl;
		}
	}*/

	if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
		renderShadowMap = !renderShadowMap;
		if (renderShadowMap) {
			std::cout << "DEBUG DRAW SHADOW MAP ENABLED" << std::endl;
		}
		else {
			std::cout << "DEBUG DRAW SHADOW MAP DISABLED" << std::endl;
		}
	}
}

void setActiveShader(Shader *shader)
{
	activeShader = shader;
	activeShader->useShader();
}
