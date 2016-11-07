#ifndef SUZANNEISLAND_GEOMETRY_HPP
#define SUZANNEISLAND_GEOMETRY_HPP 1

#include <vector>
#include <memory>

#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "sceneobject.hpp"
#include "surface.h"
#include "shader.h"
#include "texture.hpp"
#include "camera.h"


//! A SceneObject that holds Surfaces containing mesh data and textures.
class Geometry : public SceneObject
{
public:
    Geometry(const glm::mat4 &matrix_, const std::string &filePath);
    virtual ~Geometry();

    //! Update the state of the SceneObject
    virtual void update(
        float timeDelta //!< [in] time passed since last frame in seconds
    );

    //! draw the SceneObject using given shader
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

    //! Returns min vertex of axis-aligned bounding box enclosing all surfaces.
    /// \return min vertex of axis-aligned bounding box enclosing all surfaces.
    glm::vec3 getBBMin();
    //! Returns max vertex of axis-aligned bounding box enclosing all surfaces.
    /// \return max vertex of axis-aligned bounding box enclosing all surfaces.
    glm::vec3 getBBMax();

    //! The number of surfaces being drawn
    static int drawnSurfaceCount;
private:
    //!< surfaces store mesh data and textures
    std::vector<std::shared_ptr<Surface>> surfaces;

    //!< the path of the directory containing the model file to load
    std::string directoryPath;

    // pointers to all textures loaded by the surfaces of this geometry, to avoid loading twice
    static std::vector<std::shared_ptr<Texture>> loadedTextures;

    //! Load surfaces from file

    //! This loads only the first diffuse, specular and normal texture for
    //! each surface and stores them in this order in the surface.
    void loadSurfaces(
        const std::string &filePath //!< [in] file path to load surfaces from
    );

    //! Process all meshes contained in given node and recursively process all child nodes
    void processNode(
        aiNode *node, //!< the current node to process
        const aiScene *scene //!< the aiScene containing the node
    );

    //! Load data from assimp aiMesh to new Surface object

    //! This loads only the first diffuse, specular and normal texture for
    //! each surface and stores them in this order in the surface
    void processMesh(
        aiMesh *mesh, //!< [in] the assimp mesh to process
        const aiScene *scene //!< [in] the assimp scene containing the mesh
    );

    //! Load assimp aiMesh texture of given type.
    
    //! Textures of same filePath are reused among the geometry object.
    //! \return a pointer to the texture
    std::shared_ptr<Texture> loadMaterialTexture(
        aiMaterial *mat, //!< [in] the assimp mesh material
        aiTextureType type //!< [in] the assimp texture type
    );
};


#endif // SUZANNEISLAND_GEOMETRY_HPP
