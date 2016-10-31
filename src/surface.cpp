#include "surface.h"

Surface::Surface(const std::vector<Vertex> &vertices_, const std::vector<GLuint> &indices_, const std::shared_ptr<Texture> &texDiffuse_, const std::shared_ptr<Texture> &texSpecular_, const std::shared_ptr<Texture> &texNormal_)
    : vertices(vertices_)
	, indices(indices_)
	, texDiffuse(texDiffuse_)
	, texSpecular(texSpecular_)
	, texNormal(texNormal_)
{
	calculateBoundingSphere();
	initBuffers();
}

void Surface::initBuffers()
{
	// generate vertex array object (vao) bindings. the vao simply stores the state of the subsequent bindings
	// so that they can be reactived quickly later, instead of doing it all over again
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// copy vertex data to GL_ARRAY_BUFFER in vram.
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer); // bind to active context
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW); // copy data

	// copy indices to GL_ELEMENT_ARRAY_BUFFER in vram. these define the mesh structure.
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	// enable shader attributes at given indices to supply vertex data to them
	// the indices/layout of the shader attribute are defined in the shader source file
	GLint positionAttribIndex   = 0;
	GLint normalAttribIndex     = 1;
	GLint uvAttribIndex         = 2;
	glEnableVertexAttribArray(positionAttribIndex);
	glEnableVertexAttribArray(normalAttribIndex);
	glEnableVertexAttribArray(uvAttribIndex);

	// associate data from current bound array buffer with shader program attributes
	// and specify the format of the vertex attrib array data
	// PARAMETERS:
	// attrib index in shader, num elements, type, normalized?, vertex attrib array size, offset within the array
	glVertexAttribPointer(positionAttribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glVertexAttribPointer(normalAttribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	glVertexAttribPointer(uvAttribIndex, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, uv));

	// unbind vao. the state of bindings until here are stored in vao.
	glBindVertexArray(0);

	// unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void Surface::calculateBoundingSphere()
{
	// approximate bounding sphere center using arithmetic mean
	glm::vec3 arithmeticMeanPosition;
	for (Vertex v : vertices) {
		arithmeticMeanPosition += v.position;
	}
	arithmeticMeanPosition /= vertices.size();

	boundingSphereCenter = arithmeticMeanPosition;

	// determine bounding sphere radius as distance to farthest vertex from center
	float maxRadius = 0;
	for (Vertex v : vertices) {
		float currentRadius = glm::length(v.position - boundingSphereCenter);
		if (currentRadius > maxRadius) {
			boundingSphereFarthestPoint = v.position;
		}
	}
}

glm::vec3 Surface::getBoundingSphereCenter()
{
	return boundingSphereCenter;
}

glm::vec3 Surface::getBoundingSphereFarthestPoint()
{
	return boundingSphereFarthestPoint;
}

Surface::~Surface()
{
	// delete buffers (free vram)
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexBuffer);

	glDeleteVertexArrays(1, &vao);
}

void Surface::draw(Shader *shader, Texture::FilterType filterType)
{
	// pass textures to shader
	// for now just uses the diffuse texture

	if (texDiffuse) {
		GLint diffuseTexLocation = glGetUniformLocation(shader->programHandle, "material.diffuse"); // get uniform location in shader
		glUniform1i(diffuseTexLocation, 0); // bind shader texture location with texture unit 0
		texDiffuse->bind(0); // activate texture unit 0 and bind texture to it
		texDiffuse->setFilterMode(filterType);
	}
	/*
	if (texSpecular) {
		GLint specularTexLocation = glGetUniformLocation(shader->programHandle, "material.specular");
		glUniform1i(specularTexLocation, 1);
		texSpecular->bind(1);
	}
	if (texNormal) {
		GLint normalsTexLocation = glGetUniformLocation(shader->programHandle, "normalTexture");
		glUniform1i(normalsTexLocation, 2);
		texNormal->bind(2);
	}*/

	// draw triangles from given indices
	glBindVertexArray(vao); // bind the vertex array used to supply vertices
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); // use given indices
	glBindVertexArray(0);

	// DEBUG PRINT VERTICES
	//std::cout << vertices.size() << std::endl;
	/*//
	for (unsigned int i = 0; i < vertices.size(); ++i) {
		std::cout << "pos " << vertices[i].position[0] << vertices[i].position[1] << vertices[i].position[2] << std::endl;
		std::cout << "normal " << vertices[i].normal[0] << vertices[i].normal[1] << vertices[i].normal[2] << std::endl;
		std::cout << "uv " << vertices[i].uv[0] << vertices[i].uv[1] << std::endl;

	}
	//*/

}

std::vector<Vertex> Surface::getVertices()
{
	return vertices;
}

std::vector<GLuint> Surface::getIndices()
{
	return indices;
}
