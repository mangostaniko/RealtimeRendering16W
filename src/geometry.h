#ifndef GEOMETRY_H
#define GEOMETRY_H

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <memory>

#include "sceneobject.h"
#include "surface.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

/**
 * @brief A Geometry is a SceneObject that holds Surfaces which contain mesh data and textures.
 */
class Geometry : public SceneObject
{

	// surfaces store mesh data and textures
	std::vector<std::shared_ptr<Surface>> surfaces;

	// the path of the directory containing the model file to load
	std::string directoryPath;

	// pointers to all textures loaded by the surfaces of this geometry, to avoid loading twice
	static std::vector<std::shared_ptr<Texture>> loadedTextures;

	/**
	 * @brief load surfaces from file
	 * note: this loads only the first diffuse, specular and normal texture for each surface
	 * and stores them in this order in the surface
	 * @param filePath the path of the file to load surfaces from
	 */
	void loadSurfaces(const std::string &filePath);

	/**
	 * @brief process all meshes contained in given node
	 * and recursively process all child nodes
	 * @param node the current node to process
	 * @param scene the aiScene containing the node
	 */
	void processNode(aiNode *node, const aiScene *scene);

	/**
	 * @brief load data from assimp aiMesh to new Surface object
	 * note: this loads only the first diffuse, specular and normal texture for each surface
	 * and stores them in this order in the surface
	 * @param mesh the aiMesh to process
	 * @param scene the aiScene containing the mesh
	 * @return
	 */
	void processMesh(aiMesh *mesh, const aiScene *scene);

	/**
	 * @brief load assimp aiMesh texture of given type.
	 * textures of same filePath are reused among the geometry object.
	 * @param mat the assimp mesh material
	 * @param type the aiTextureType
	 * @return a pointer to the texture
	 */
	std::shared_ptr<Texture> loadMaterialTexture(aiMaterial *mat, aiTextureType type);

public:

	Geometry(const glm::mat4 &matrix_, const std::string &filePath);
	virtual ~Geometry();

	/**
	 * @brief update the state of the SceneObject
	 * @param timeDelta the time passed since the last frame in seconds
	 */
	virtual void update(float timeDelta);

	/**
	 * @brief draw the SceneObject using given shader
	 */
	virtual void draw(Shader *shader, Camera *camera, bool useFrustumCulling, Texture::FilterType filterType, const glm::mat4 &viewMat);

	/**
	 * @brief return a the transposed inverse of the modelMatrix.
	 * this should be used to transform normals into world space.
	 * the inverse is used since the normals of a scaled vector are inversely scaled,
	 * the transpose is used to revert the inversion of the rotational components
	 * while not affecting the scale factors which lie on the main diagonal.
	 * mat3 is used since the translational component is irrelevant for normals.
	 * @return the matrix to transform normals into world space.
	 */
	glm::mat3 getNormalMatrix() const;

	// return min and max vertex of axis aligned bounding box enclosing all surfaces
	glm::vec3 getBBMin();
	glm::vec3 getBBMax();

	// the number of surfaces being drawn
	static int drawnSurfaceCount;

};

#endif // GEOMETRY_H
