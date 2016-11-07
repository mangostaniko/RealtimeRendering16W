#include "particlesystem.h"

// vertex positions and uvs defining a quad, used to render particles.
static const GLfloat quadVertices[] = {
    // positions   // uvs
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

/**
 * used to sort vector of shared_ptr by their pointed values of type T
 */
template <typename T>
struct SortSharedPtr {
    bool operator()(const std::shared_ptr<T>& a, const std::shared_ptr<T>& b) {
        return *a < *b;
    }
};

ParticleSystem::ParticleSystem(const glm::mat4 &matrix_, const std::string &texturePath, int maxParticleCount_, float spawnRate_, float timeToLive_, float gravity_)
    : SceneObject(matrix_)
    , maxParticleCount(maxParticleCount_)
    , spawnRate(spawnRate_)
    , timeToLive(timeToLive_)
    , gravity(gravity_)
{

        particleShader = new Shader("shaders/particles.vert", "shaders/particles.frag");
    particleTexture = new Texture(texturePath, true);
    std::cout << "loaded texture: " << texturePath << std::endl;


    // generate vertex array object (vao) bindings. the vao simply stores the state of the subsequent bindings
    // so that they can be reactived quickly later, instead of doing it all over again
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);


    // setup particle quad vertices buffer (shared among all particles using instancing)
    glGenBuffers(1, &particleQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, particleQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // setup particle instance data buffer
    // sends GL_STREAM_DRAW hint to GL implementation, since alternating write/read expected. data is assigned later.
    glGenBuffers(1, &particleInstanceDataVBO);
    glBindBuffer(GL_ARRAY_BUFFER, particleInstanceDataVBO);
    glBufferData(GL_ARRAY_BUFFER, maxParticleCount * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    // enable shader attributes at given indices to supply buffer data to them
    // the indices/layout of the shader attribute are defined in the particle shader source file
    GLint particleQuadVerticesAttribIndex   = 0;
    GLint particleInstanceDataAttribIndex   = 1;
    glEnableVertexAttribArray(particleQuadVerticesAttribIndex);
    glEnableVertexAttribArray(particleInstanceDataAttribIndex);

    // associate data from current bound buffer with shader program attributes
    // and specify the format of the vertex attrib array data
    // PARAMETERS:
    // attrib index in shader, num elements, type, normalized?, stride (vertex attrib array size), offset within the array
    glBindBuffer(GL_ARRAY_BUFFER, particleQuadVBO);
    glVertexAttribPointer(particleQuadVerticesAttribIndex, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0); // 4 elements (xy pos + uv)
    glBindBuffer(GL_ARRAY_BUFFER, particleInstanceDataVBO);
    glVertexAttribPointer(particleInstanceDataAttribIndex, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0); // 4 elements (xyz + size)


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

ParticleSystem::~ParticleSystem()
{
    glDeleteBuffers(1, &particleQuadVBO);
    glDeleteBuffers(1, &particleInstanceDataVBO);

    delete particleShader;
    delete particleTexture;
}

void ParticleSystem::draw(const glm::mat4 &viewMat, const glm::mat4 &projMat, const glm::vec3 &color)
{

    particleShader->useShader();

    // pass matrices to shader
    GLint modelViewMatLocation = glGetUniformLocation(particleShader->programHandle, "modelViewMat"); // get uniform location in shader
    glUniformMatrix4fv(modelViewMatLocation, 1, GL_FALSE, glm::value_ptr(viewMat * getMatrix())); // shader location, count, transpose?, value pointer
    GLint projMatLocation = glGetUniformLocation(particleShader->programHandle, "projMat");
    glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, glm::value_ptr(projMat));

    // pass texture to shader
    GLint particleTexLocation = glGetUniformLocation(particleShader->programHandle, "particleTexture"); // get uniform location in shader
    glUniform1i(particleTexLocation, 0); // bind shader texture location with texture unit 0
    particleTexture->bind(0); // activate texture unit 0 and bind texture to it

    // pass color
    glUniform3f(glGetUniformLocation(particleShader->programHandle, "color"), color.x, color.y, color.z);


    // for instanced drawing we use glDrawArraysInstanced, which behaves like glDrawArrays,
    // but also advances the used buffer attributes after a given number of drawn instances,
    // depending on the glVertexAttribDivisor value assigned for that buffer,
    // which divides the instances into the number of different attributes to be assigned.
    glBindVertexArray(vao);
    glVertexAttribDivisor(0, 0); // quad vertex buffer              (always use same vertices)
    glVertexAttribDivisor(1, 1); // particle instance data buffer   (advance for each instance)
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, particles.size()); // mode, first index, last index, instance count
    glBindVertexArray(0);


}

void ParticleSystem::update(float timeDelta, const glm::mat4 &viewMat)
{

    // SPAWN PARTICLES

    if (!spawningPaused) {

        secondsSinceLastSpawn += timeDelta;
        int spawnedParticleCount = int(secondsSinceLastSpawn*spawnRate + 0.5f);
        if (spawnedParticleCount > 0) { secondsSinceLastSpawn = 0.0f; }

        for (int i = 0; i < spawnedParticleCount; ++i) {
            if (particles.size() < maxParticleCount) {
                std::shared_ptr<Particle> particle = std::make_shared<Particle>();
                particle->timeToLive = timeToLive;
                particle->velocity = glm::vec3(randomFloat()-0.2f, randomFloat(), randomFloat()-0.2f) * 3.0f; // simulate wind
                particles.push_back(particle);
            } else {
                spawningPaused = true;
            }
        }

    }

    // SIMULATE PARTICLES

    glm::mat4 modelViewMat = viewMat * getMatrix();

    for (unsigned int i = 0; i < particles.size(); ++i) {
        std::shared_ptr<Particle> particle = particles[i];

        particle->timeToLive -= timeDelta;
        if (particle->timeToLive < 0) {
            particles.erase(particles.begin()+i);
            continue;
        }

        // simulate gravitational acceleration
        particle->velocity += glm::vec3(0.0f, -9.81f, 0.0f) * timeDelta * gravity;
        particle->pos += particle->velocity * timeDelta;

        // particle depth for sorting to draw with alpha
        glm::vec3 particleViewPos = glm::vec3(modelViewMat * glm::vec4(particle->pos.x, particle->pos.y, particle->pos.z, 1));
        particle->viewDepth = -particleViewPos.z;

    }

    // sort in back to front drawing order.
    // this is needed for alpha blending since zbuffer test rejects fragments that lie behind,
    // but their color data is needed for blending.
    std::sort(particles.begin(), particles.end(), SortSharedPtr<Particle>());

    // fill array to pass to particle instance data buffer
    std::vector<float> particleInstanceData;
    for (unsigned int i = 0; i < particles.size(); ++i) {
        std::shared_ptr<Particle> particle = particles[i];

        particleInstanceData.push_back(particle->pos.x);
        particleInstanceData.push_back(particle->pos.y);
        particleInstanceData.push_back(particle->pos.z);
        particleInstanceData.push_back(particle->timeToLive / timeToLive);
    }

    // UPDATE BUFFER

    // note about buffer updates when streaming:
    // when streaming (i.e. alternately writing and reading frequently) the GL implementation
    // might delay buffer write operations until it has finished all draw calls from that buffer.
    // to avoid such lockdowns, buffer respecification ('orphaning') can be used,
    // whereby glBufferData is called with NULL data pointer and same other arguments as initially.
    // most implementations will then allocate a new memory block and bind it to the buffer handle
    // while still using the old memory block for drawing, until all drawing has been completed.

    // update particle instance data
    glBindBuffer(GL_ARRAY_BUFFER, particleInstanceDataVBO);
    glBufferData(GL_ARRAY_BUFFER, maxParticleCount * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particleInstanceData.size() * sizeof(GLfloat), &particleInstanceData[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void ParticleSystem::respawn(glm::vec3 location)
{
    setLocation(location);
    particles.clear();
    secondsSinceLastSpawn = 0;
    spawningPaused = false;
}

float ParticleSystem::randomFloat()
{
    return (float)rand()/RAND_MAX;
}
