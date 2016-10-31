#ifndef SURFACE_H
#define SURFACE_H

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <memory>
#include <stddef.h>

#include "shader.h"
#include "texture.h"

/**
 * @brief Vertex struct for internal representation
 */
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

/**
 * @brief A Surface holds mesh data (vertex data and indices) and textures.
 * This communicates with Vertex Buffer Objects to store vertex data directly on GPU memory
 * and with compiled shader programs for drawing.
 */
class Surface
{
	// Mesh Data
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices; // indices associate vertices to define mesh topology

	// Bounding Sphere
	// for view frustum culling
	glm::vec3 boundingSphereCenter;
	glm::vec3 boundingSphereFarthestPoint;

	// Textures
	std::shared_ptr<Texture> texDiffuse, texSpecular, texNormal;

	// handle for vertex array object (vao). the vao simply stores the bindings set when its active
	// so that they can be reactived quickly later, instead of setting it up all over again.
	GLuint vao;

	// handles for vram buffers.
	GLuint vertexBuffer, indexBuffer;

	/**
	 * @brief initialize vba, copy vertex data to vram buffers and associate with shader attributes
	 */
	void initBuffers();

public:
	Surface(const std::vector<Vertex> &vertices_, const std::vector<GLuint> &indices_, const std::shared_ptr<Texture> &texDiffuse_, const std::shared_ptr<Texture> &texSpecular_, const std::shared_ptr<Texture> &texNormal_);
	~Surface();

	std::vector<Vertex> getVertices();
	std::vector<GLuint> getIndices();

	/**
	 * @brief draw triangles from vertex data from buffers bound as specified by the vba.
	 * note: the transformation matrices must be set already in shader program!
	 * @param shader the compiled shader program to use for drawing
	 */
	void draw(Shader *shader, Texture::FilterType filterType);

	/**
	 * @brief get the center of the bounding sphere
	 * for this surface to be used in view frustum culling
	 * @return the bounding sphere center
	 */
	glm::vec3 getBoundingSphereCenter();

	/**
	 * @brief get the farthest point on the bounding sphere
	 * whose length defines the bounding sphere radius.
	 * used for view frustum culling
	 * @return the farthest point on the bounding sphere
	 */
	glm::vec3 getBoundingSphereFarthestPoint();

private:

	/**
	 * @brief calculate parameters defining a bounding sphere
	 * for this surface to be used in view frustum culling
	 */
	void calculateBoundingSphere();

};

#endif // SURFACE_H
