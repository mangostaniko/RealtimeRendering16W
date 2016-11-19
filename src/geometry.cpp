#include "geometry.h"


int Geometry::drawnSurfaceCount = 0;
std::vector<std::shared_ptr<Texture>> Geometry::loadedTextures = {};
glm::vec3 boundingBoxMin;
glm::vec3 boundingBoxMax;

Geometry::Geometry(const glm::mat4 &matrix_, const std::string &filePath)
    : SceneObject(matrix_)
{
    std::cout << "LOADING MODEL " << filePath << std::endl;
    loadSurfaces(filePath);
}

Geometry::~Geometry()
{}

glm::mat3 Geometry::getNormalMatrix() const
{
    return glm::transpose(glm::mat3(getInverseMatrix()));
}

glm::vec3 Geometry::getBBMin()
{
    return boundingBoxMin;
}

glm::vec3 Geometry::getBBMax()
{
    return boundingBoxMax;
}

void Geometry::update(float timeDelta)
{}

void Geometry::draw(Shader *shader, Camera *camera, bool useFrustumCulling, Texture::FilterType filterType, const glm::mat4 &viewMat)
{
    // pass model matrix to shader
    GLint modelMatLocation = glGetUniformLocation(shader->programHandle, "modelMat"); // get uniform location in shader
    glUniformMatrix4fv(modelMatLocation, 1, GL_FALSE, glm::value_ptr(getMatrix())); // shader location, count, transpose?, value pointer

    // pass normal matrix to shader
    GLint normalMatLocation = glGetUniformLocation(shader->programHandle, "normalMat");
    glUniformMatrix3fv(normalMatLocation, 1, GL_FALSE, glm::value_ptr(getNormalMatrix()));

    // draw surfaces
    for (GLuint i = 0; i < surfaces.size(); ++i) {

        // view frustum culling using bounding spheres
        if (useFrustumCulling) {
            glm::vec3 boundingSphereCenter = (getMatrix() * glm::vec4(surfaces[i]->getBoundingSphereCenter(), 1)).xyz;
            glm::vec3 boundingSphereFarthestPoint = (getMatrix() * glm::vec4(surfaces[i]->getBoundingSphereFarthestPoint(), 1)).xyz;

            if (!camera->checkSphereInFrustum(boundingSphereCenter, boundingSphereFarthestPoint, viewMat)) {
                continue;
            }
        }
        drawnSurfaceCount += 1;
        surfaces[i]->draw(shader, filterType);
    }
}

void Geometry::loadSurfaces(const std::string &filePath)
{
    // read surface data from file using Assimp.
    //
    // IMPORTANT ASSIMP POSTPROCESS FLAGS
    // - aiProcess_PreTransformVertices: to load vertices in world space i.e. apply transformation matrices, which we dont load
    // - aiProcess_Triangulate: needed for OpenGL
    // if there are problems with the uvs, try aiProcess_FlipUVs
    // note: experiment with flags like aiProcess_SplitLargeMeshes, aiProcess_OptimizeMeshes, when using bigger models.

    Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(filePath, aiProcess_PreTransformVertices | aiProcess_Triangulate);

    // check for errors
    if (!scene || !scene->mRootNode || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "ERROR ASSIMP: " << importer.GetErrorString() << std::endl;
        return;
    }

    // save path to the directory containing the file
    directoryPath = filePath.substr(0, filePath.find_last_of('/'));

    // recursively process Assimp root node
    processNode(scene->mRootNode, scene);

    std::cout << "loaded " << surfaces.size() << " surfaces." << std::endl;
    std::cout << "loaded " << loadedTextures.size() << " textures." << std::endl;
}

void Geometry::processNode(aiNode *node, const aiScene *scene)
{
    // process all meshes contained in this node.
    // note that the node->mMeshes just define the hierarchy
    // and store indices to the actual data in scene->mMeshes
    for (GLuint i = 0; i < node->mNumMeshes; ++i) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }

    // then process all child nodes
    for (GLuint i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }

}

void Geometry::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::shared_ptr<Texture> surfaceTextureDiffuse, surfaceTextureSpecular, surfaceTextureNormal;

    // process mesh vertices (positions, normals, uvs)
    for (GLuint i = 0; i < mesh->mNumVertices; ++i) {

        Vertex vertex;
        glm::vec3 vector;

        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        // normals
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        // uvs
        // note: while meshes can have 8 uv channels in assimp, we only take data from first uv channel.
        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.uv = vec;
        }
        else {
            vertex.uv = glm::vec2(0.0f, 0.0f);
        }

        // update axis aligned bounding box
        if (glm::all(glm::lessThan(vertex.position, boundingBoxMin))) {
            boundingBoxMin = vertex.position;
        }
        else if (glm::all(glm::greaterThan(vertex.position, boundingBoxMax))) {
            boundingBoxMax = vertex.position;
        }

        vertices.push_back(vertex);
    }

    // process mesh faces and store indices
    for (GLuint i = 0; i < mesh->mNumFaces; ++i) {

        aiFace face = mesh->mFaces[i];
        for (GLuint j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // process material and store textures
    // note: we only load the first diffuse, specular and normal texture reffered to by the assimp material
    // and store them in this order
    if (mesh->mMaterialIndex >= 0) {

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        surfaceTextureDiffuse = loadMaterialTexture(material, aiTextureType_DIFFUSE);
        surfaceTextureSpecular = loadMaterialTexture(material, aiTextureType_SPECULAR);
        surfaceTextureNormal = loadMaterialTexture(material, aiTextureType_NORMALS);
    }

    // return a Surface object created from the extracted aiMesh data
    surfaces.push_back(std::make_shared<Surface>(vertices, indices, surfaceTextureDiffuse, surfaceTextureSpecular, surfaceTextureNormal));

}

std::shared_ptr<Texture> Geometry::loadMaterialTexture(aiMaterial *mat, aiTextureType type)
{
    aiString texturePath;
    std::shared_ptr<Texture> texture = nullptr;

    if (mat->GetTexture(type, 0, &texturePath) == AI_SUCCESS) {

        // check if we already loaded the texture of the given path for another mesh
        bool skip = false;
        for (auto existingTexture : loadedTextures) {

            if (existingTexture->getFilePath() == (directoryPath + '/' + texturePath.C_Str())) {
                skip = true;
                texture = existingTexture; // use pointer to existing texture
                break;
            }
        }

        // otherwise load the texture from the file
        if (!skip) {
			loadedTextures.push_back(std::make_shared<Texture>(directoryPath + '/' + texturePath.C_Str()));
            std::cout << "loaded texture: " << directoryPath + '/' + texturePath.C_Str() << std::endl;
            texture = loadedTextures.back();
        }

    }

    return texture;
}
