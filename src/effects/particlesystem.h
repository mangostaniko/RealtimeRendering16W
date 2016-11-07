#ifndef SUZANNEISLAND_EFFECTS_PARTICLESYSTEM_HPP
#define SUZANNEISLAND_EFFECTS_PARTICLESYSTEM_HPP 1

#include <vector>
#include <memory>
#include <algorithm>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include "../sceneobject.hpp"
#include "../shader.h"
#include "../texture.hpp"


struct Particle
{
    glm::vec3 pos, velocity;
    GLfloat viewDepth = 0.0f; // for sorting
    GLfloat timeToLive = 1000.0f; // seconds

    // for sorting in back to front drawing order.
    // this is needed for alpha blending since zbuffer test rejects fragments that lie behind,
    // but their color data is needed for blending.
    bool operator<(Particle& other)
    {
        return this->viewDepth > other.viewDepth;
    }
};


class ParticleSystem : SceneObject
{
public:
    ParticleSystem(const glm::mat4 &matrix_, const std::string &texturePath, int maxParticleCount_, float spawnRate_, float timeToLive_, float gravity_);
    ~ParticleSystem();

    //! Update the particles in the particle system
    void update(
        float timeDelta, //!< [in] the time since the last frame in seconds
        const glm::mat4 &viewMat
    );

    //! Draw the particles in the particle system
    void draw(const glm::mat4 &viewMat, const glm::mat4 &projMat, const glm::vec3 &color);

    //! Clear all particles and reinitiate spawning
    void respawn(glm::vec3 location);

private:
    GLuint vao;
    GLuint particleQuadVBO;
    GLuint particleInstanceDataVBO;

    Shader *particleShader = nullptr;
    Texture *particleTexture = nullptr;

    unsigned int maxParticleCount = 1000; // maximum total particle count
    bool spawningPaused = true;
    float spawnRate = 200.0f;             // how many particles to spawn per second
    float secondsSinceLastSpawn = -1.0f;  // time since last spawned particle, in seconds
    float timeToLive = 10.0f;             // time in seconds until particle disappears
    float gravity = 0.1f;                 // factor for gravitational acceleration

    std::vector<std::shared_ptr<Particle>> particles;

    //! Returns a pseudorandom float in range [0, 1]
    /// \return pseudorandom float in range [0, 1]
    float randomFloat();
};


#endif // SUZANNEISLAND_EFFECTS_PARTICLESYSTEM_HPP
