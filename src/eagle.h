#ifndef SUZANNEISLAND_EAGLE_HPP
#define SUZANNEISLAND_EAGLE_HPP

#include <glm/gtx/vector_angle.hpp>
#include "geometry.h"


enum EagleState
{
    CIRCLING    = 0,
    ATTACKING   = 1,
    RETREATING  = 2
};

class Eagle : public Geometry
{
    // GAMEPLAY
    // all durations given in seconds

    const float EAT_RADIUS = 1.0f;
    const float TARGET_DEFENSE_REACH_RADIUS = 13.0f;
    const float ATTACK_WAIT_TIME_MIN = 15.0f; // min time after which eagle will attempt to attack player
    const float ATTACK_WAIT_TIME_MAX = 25.0f; // max time after which eagle will attempt to attack player
    glm::mat4 eagleInitTransform;

    EagleState state = CIRCLING;

    float totalTimePassed = 0;
    float timeSinceLastAttack = 0;
    float timeIntervalToNextAttack = ATTACK_WAIT_TIME_MIN;

    glm::vec3 targetPos;
    bool targetHidden = false;
    bool targetDefenseActive = false;

public:
    Eagle(const glm::mat4 &matrix, const std::string &filePath);
    ~Eagle();

    virtual void update(float timeDelta, const glm::vec3 &targetPos_, bool targetHidden_, bool targetDefenseActive_);

    /**
     * @brief return the current state of behaviour of the eagle
     * @return the current state of behaviour of the eagle
     */
    EagleState getState();

    /**
     * @brief check whether the target is in TARGET_DEFENSE_REACH_RADIUS of the eagle
     * @return whether the target is in TARGET_DEFENSE_REACH_RADIUS of the eagle
     */
    bool isInTargetDefenseReach();

    /**
     * @brief check whether the target is in EAT_RADIUS of the eagle
     * @return whether the target is in EAT_RADIUS of the eagle
     */
    bool isTargetEaten();

    void resetEagle();
private:
};


#endif // SUZANNEISLAND_EAGLE_HPP
