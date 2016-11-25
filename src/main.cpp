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
#include "effects/ssaopostprocessor.h"

void init(GLFWwindow *window);
void initSM();
void initVSM();
void initPCFSM();
void initVSMBlur();
void shadowFirstPass(glm::mat4 &lightViewPro);
void vsmBlurPass();
void debugShadowPass();
void ssaoFirstPass();
void finalDrawPass();
void update(float timeDelta);
void setActiveShader(Shader *shader);
void drawScene();
void drawText(double deltaT, int windowWidth, int windowHeight);
void cleanup();
void newGame();

GLFWwindow *window;
int windowWidth, windowHeight;
bool running                    = true;
bool paused                     = false;
bool debugInfoEnabled           = true;
bool wireframeEnabled           = false;
bool ssaoEnabled                = false;
bool ssaoBlurEnabled            = false;
bool shadowsEnabled             = true;
bool vsmShadowsEnabled          = true;
bool renderShadowMap            = false;
bool frustumCullingEnabled      = false;
bool debugDrawTransparent       = false;

Texture::FilterType filterType = Texture::LINEAR_MIPMAP_LINEAR;

// uniform buffer object to share common uniform data between shaders
// NOTE: it seems that on the gpu or opengl implementation the offsets
// i.e. the size of uniform blocks in gpu memory must be multiples of 256
// otherwise glBindBufferRange yields error 1281 (invalid value).
GLuint commonShaderUniformsUBO;
GLint uboMatricesBlockSize = 256; //2 * sizeof(glm::mat4);
GLint uboViewAndCamBlockSize = 256; //5 * sizeof(glm::vec4);
GLint uboTotalSize = uboMatricesBlockSize + uboViewAndCamBlockSize;

Shader *textureShader, *waterShader;
Shader *depthMapShader, *vsmDepthMapShader, *debugDepthShader, *blurVSMDepthShader; // shadow mapping
Shader *activeShader;
TextRenderer *textRenderer;
SSAOPostprocessor *ssaoPostprocessor;

Camera *camera; glm::mat4 cameraInitTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 10, 50)));
Eagle *eagle; glm::mat4 eagleInitTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 30, -45)));

Geometry *island;
Geometry *ocean;

Light *sun; // sun start and end positions are linearly interpolated over time of day
const glm::vec3 LIGHT_START(glm::vec3(-20, 50, -40));
const glm::vec3 LIGHT_END(glm::vec3(20, 30, -50));
const float dayLength = 60;

// Shadow Map FBO and depth texture
const int SM_WIDTH = 2048, SM_HEIGHT = 2048;
const GLfloat SM_NEAR_PLANE = 0.5f, SM_FAR_PLANE = 500.f;
GLuint depthMapFBO, vsmDepthMapFBO;
GLuint depthMap, vsmDepthMap;
GLuint pingpongFBO;
GLuint pingpongColorMap;

void frameBufferResize(GLFWwindow *window, int width, int height);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

// ONLY FOR SHADOW MAPPING DEBUG
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
    if (quadVAO == 0)
    {
        GLfloat quadVertices[] = {
            // Positions         // Texture Coords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
            1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
            1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        };
        // Setup plane VAO
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
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
// SHADOW MAP DEBUG END


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
    double deltaT = 0.0;

    while (running && !glfwWindowShouldClose(window)) {

        if (glfwGetTime() < lastTime) {
            lastTime = 0;
        }
        time = glfwGetTime(); // seconds
        deltaT = time - lastTime;
        lastTime = time;

        // glUseProgram calls are rather expensive state changes, so try to keep to a minimum
        // if more shaders are used for different objects, restructuring of these calls will be necessary
        // since a lot of other calls depend on the currently bound shader
        setActiveShader(textureShader);

        //////////////////////////
        /// UPDATE
        //////////////////////////
        if (!paused) {
            update(deltaT);
        }

		///////////////////////////////////////////////////////////
		//// SET SHARED UNIFORM DATA VIA UNIFORM BUFFER OBJECT ////
		///////////////////////////////////////////////////////////
		glBindBuffer(GL_UNIFORM_BUFFER, commonShaderUniformsUBO);

		// UBO data for Matrices shader uniform block at memory range bound to binding point 0
		GLint m4sz = sizeof(glm::mat4);
		glBufferSubData(GL_UNIFORM_BUFFER, 0 + 0*m4sz, m4sz, glm::value_ptr(camera->getViewMat())); // view mat
		glBufferSubData(GL_UNIFORM_BUFFER, 0 + 1*m4sz, m4sz, glm::value_ptr(camera->getProjMat())); // proj mat

		// UBO data for LightAndCam shader uniform block at memory range bound to binding point 1
		GLint v4sz = sizeof(glm::vec4);
		GLint offset = uboMatricesBlockSize;
		glBufferSubData(GL_UNIFORM_BUFFER, offset + 0*v4sz, v4sz, glm::value_ptr(glm::vec4(sun->getLocation(), 1)));     // light pos
		glBufferSubData(GL_UNIFORM_BUFFER, offset + 1*v4sz, v4sz, glm::value_ptr(glm::vec4(sun->getColor() * 0.3f, 1))); // ambient
		glBufferSubData(GL_UNIFORM_BUFFER, offset + 2*v4sz, v4sz, glm::value_ptr(glm::vec4(sun->getColor(), 1)));        // diffuse
		glBufferSubData(GL_UNIFORM_BUFFER, offset + 3*v4sz, v4sz, glm::value_ptr(glm::vec4(sun->getColor() * 0.8f, 1))); // specular
		glBufferSubData(GL_UNIFORM_BUFFER, offset + 4*v4sz, v4sz, glm::value_ptr(glm::vec4(camera->getLocation(), 1)));  // camera pos

		// unbind UBO
		glBindBuffer(GL_UNIFORM_BUFFER, 0);


        //////////////////////////
        /// DRAW
        //////////////////////////

        //// SHADOW MAP PASS
        // calculate lights projection and view Matrix
		glm::mat4 lightVPMat;

        if (shadowsEnabled) {
			shadowFirstPass(lightVPMat);
        }

		// set uniforms for shadow mapping in default textureShader
		setActiveShader(textureShader);
		glUniformMatrix4fv(activeShader->getUniformLocation("viewMat"), 1, GL_FALSE, glm::value_ptr(camera->getViewMat()));
		glUniformMatrix4fv(activeShader->getUniformLocation("lightVPMat"), 1, GL_FALSE, glm::value_ptr(lightVPMat));
		glUniform1i(activeShader->getUniformLocation("shadowMap"), 1);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, vsmDepthMap);

		//// SSAO PREPASS (if enabled)
        ssaoFirstPass();

		//// FINAL MAIN DRAW PASS
        //// draw with shadow mapping and ssao
        finalDrawPass();

        // draw shadow map for debugging (if enabled)
        debugShadowPass();

        drawText(deltaT, windowWidth, windowHeight);

        // end the current frame (swaps the front and back buffers)
        glfwSwapBuffers(window);


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

	// alpha blending for textures with alpha channel
	// NOTE disabled, since proper alpha blending requires that triangles are drawn from back to front
	// since depth buffer rejects fragments of greater depth once we already have a closer value
	// but we need all the color values of the fragments for blending.
	//glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // INIT SHADOW MAPPING (FBO, Texture, Shader)
    initSM();

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    paused = false;

    // INIT TEXT RENDERER
    textRenderer = new TextRenderer("data/fonts/cliff.ttf", width, height);

    // INIT SSAO POST PROCESSOR
    ssaoPostprocessor = new SSAOPostprocessor(width, height, 32);

    // INIT SHADERS
    textureShader = new Shader("shaders/textured_blinnphong.vert", "shaders/textured_blinnphong.frag");
	waterShader = new Shader("shaders/water.vert", "shaders/water.frag");
    setActiveShader(textureShader); // non-trivial cost

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



    // NOTE: the following initializations are intended to be used with a shader of the structure like textureShader
    // so dont activate any shader of different structure before those initializations are done

    // INIT WORLD + OBJECTS
    sun = new Light(glm::translate(glm::mat4(1.0f), LIGHT_START), LIGHT_END, glm::vec3(1.f, 0.89f, 0.6f), glm::vec3(0.87f, 0.53f, 0.f), dayLength);
    island = new Geometry(glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1)), "data/models/island/island.dae");
	ocean = new Geometry(glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1)), "data/models/island/water.dae");

    // INIT CAMERA
	camera = new Camera(window, cameraInitTransform, glm::radians(90.0f), width/(float)height, 0.2f, 600.0f); // mat, fov, aspect, znear, zfar

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
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
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
    camera->update(timeDelta);

    eagle->update(timeDelta, camera->getLocation() + glm::vec3(0, 2, 0), true, false);

	sun->update(timeDelta);
	//std::cout << sun->getLocation().x << " " << sun->getLocation().y << " " << sun->getLocation().z << " " << std::endl;

}


void drawScene()
{
	Geometry::drawnSurfaceCount = 0;

    if (wireframeEnabled) {
        // enable wireframe
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

	//////////////////////////////////////////////////
	/// DEFAULT ACTIVE SHADER TEXTURED BLINN-PHONG
	//////////////////////////////////////////////////
	setActiveShader(textureShader);

	if (debugDrawTransparent) {
		glUniform1f(activeShader->getUniformLocation("debugDrawTransparent"), true);
    }
    else {
		glUniform1f(activeShader->getUniformLocation("debugDrawTransparent"), false);
    }

	// pass common material color to shader
	glUniform3f(activeShader->getUniformLocation("material.specular"), 0.2f, 0.2f, 0.2f);

    // DRAW GEOMETRY

	// we need to disable back face culling to render palm leaves which are not closed meshes
	glDisable(GL_CULL_FACE);
	glUniform1f(activeShader->getUniformLocation("material.shininess"), 64.f);
	island->draw(activeShader, camera, frustumCullingEnabled, filterType, camera->getViewMat());
	glEnable(GL_CULL_FACE);

	glUniform1f(activeShader->getUniformLocation("material.shininess"), 32.f);
	eagle->draw(activeShader, camera, frustumCullingEnabled, filterType, camera->getViewMat());

	//////////////////////////
	/// ACTIVE SHADER WATER
	setActiveShader(waterShader);
	ocean->draw(activeShader, camera, frustumCullingEnabled, filterType, camera->getViewMat());
	setActiveShader(textureShader);
	//////////////////////////

    if (wireframeEnabled) {
        // disable wireframe
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}


void drawText(double deltaT, int windowWidth, int windowHeight)
{
    glDisable(GL_DEPTH_TEST);

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
        textRenderer->renderText("PAUSED", 25.0f, 150.0f, 0.7f, glm::vec3(1, 0.35f, 0.7f));
    }

    glEnable(GL_DEPTH_TEST);
}


void shadowFirstPass(glm::mat4 &lightViewPro)
{
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
    drawScene();
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
    RenderQuad();
    horizontal = !horizontal;

    glBindFramebuffer(GL_FRAMEBUFFER, vsmDepthMapFBO);
	glUniform1i(activeShader->getUniformLocation("horizontal"), horizontal);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pingpongColorMap);
    RenderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void ssaoFirstPass()
{
    if (ssaoEnabled) {
        //// SSAO PREPASS
        //// draw ssao input data (screen colors and view space positions) to framebuffer textures
        ssaoPostprocessor->bindScreenDataFramebuffer();
		glUniform1i(activeShader->getUniformLocation("useShadows"), 0);
		glUniform1i(activeShader->getUniformLocation("useSSAO"), 0);
		glUniform1i(activeShader->getUniformLocation("useVSM"), 0);
        glClearColor(sun->getColor().x, sun->getColor().y, sun->getColor().z, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawScene();

        //// SSAO PASS
        //// draw ssao output data to framebuffer texture
        ssaoPostprocessor->calulateSSAOValues(camera->getProjMat());
        setActiveShader(textureShader);

        //// SSAO BLUR PASS
        if (ssaoBlurEnabled) {
            ssaoPostprocessor->blurSSAOResultTexture();
            //ssaoPostprocessor->blurSSAOResultTexture();
            setActiveShader(textureShader);
        }
    }
}


void finalDrawPass()
{
    glClearColor(sun->getColor().x, sun->getColor().y, sun->getColor().z, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniform1i(activeShader->getUniformLocation("useShadows"), shadowsEnabled);
	glUniform1i(activeShader->getUniformLocation("useSSAO"), ssaoEnabled);
	glUniform1i(activeShader->getUniformLocation("useVSM"), vsmShadowsEnabled);

	ssaoPostprocessor->bindSSAOResultTexture(activeShader->getUniformLocation("ssaoTexture"), 2);

    drawScene();
}


void debugShadowPass()
{
    if (renderShadowMap) {

        setActiveShader(debugDepthShader);

		glUniform1f(debugDepthShader->getUniformLocation("near_plane"), SM_NEAR_PLANE);
		glUniform1f(debugDepthShader->getUniformLocation("far_plane"), SM_FAR_PLANE);
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, vsmDepthMap);

        RenderQuad();
    }
}

void newGame()
{
    delete sun;

    glfwSetTime(0);

    sun = new Light(glm::translate(glm::mat4(1.0f), LIGHT_START), LIGHT_END, glm::vec3(1.f, 0.89f, 0.6f), glm::vec3(0.87f, 0.53f, 0.f), dayLength);

    // RESET CAMERA
    camera->setTransform(cameraInitTransform);
    eagle->setTransform(eagleInitTransform);
    eagle->resetEagle();

    paused = false;
}


void cleanup()
{
    delete textureShader; textureShader = nullptr;
	delete waterShader; waterShader = nullptr;
    delete depthMapShader; depthMapShader = nullptr;
    delete debugDepthShader; debugDepthShader = nullptr;
    delete vsmDepthMapShader; vsmDepthMapShader = nullptr;
    delete blurVSMDepthShader; blurVSMDepthShader = nullptr;
    activeShader = nullptr;

	delete textRenderer; textRenderer = nullptr;
	delete ssaoPostprocessor; ssaoPostprocessor = nullptr;

    delete camera; camera = nullptr;
    delete eagle; eagle = nullptr;
    delete island; island = nullptr;
	delete ocean; ocean = nullptr;
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
    ssaoPostprocessor->setupFramebuffers(windowWidth, windowHeight);
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
        wireframeEnabled = !wireframeEnabled;
        if (wireframeEnabled) {
            std::cout << "DRAW WIREFRAME ENABLED" << std::endl;
        }
        else {
            std::cout << "DRAW WIREFRAME DISABLED" << std::endl;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
        filterType = static_cast<Texture::FilterType>((static_cast<int>(filterType)+3) % 6);

        switch (filterType) {
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
        int filterTypeInt = static_cast<int>(filterType);
        filterType = static_cast<Texture::FilterType>((static_cast<int>(filterType)+1) % 3 + (filterTypeInt/3)*3);

        switch (filterType) {
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
        ssaoEnabled = !ssaoEnabled;
        if (ssaoEnabled) {
            std::cout << "SSAO ENABLED" << std::endl;
        }
        else {
            std::cout << "SSAO DISABLED" << std::endl;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_F7) == GLFW_PRESS) {
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

    if (glfwGetKey(window, GLFW_KEY_F8) == GLFW_PRESS) {
        frustumCullingEnabled = !frustumCullingEnabled;
        if (frustumCullingEnabled) {
            std::cout << "VIEW FRUSTUM CULLING ENABLED" << std::endl;
        }
        else {
            std::cout << "VIEW FRUSTUM CULLING DISABLED" << std::endl;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_F9) == GLFW_PRESS) {
		debugDrawTransparent = !debugDrawTransparent;
		if (debugDrawTransparent) {
            std::cout << "TRANSPARENCY ENABLED" << std::endl;
        }
        else {
            std::cout << "TRANSPARENCY DISABLED" << std::endl;
        }
    }

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
