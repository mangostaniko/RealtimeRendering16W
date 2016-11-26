#pragma once

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
public:
    Eagle(const glm::mat4 &matrix, const std::string &filePath);
    ~Eagle();

    virtual void update(float timeDelta, const glm::vec3 &targetPos_, bool targetHidden_, bool targetDefenseActive_);

    //! Returns the current state of the eagle's behaviour
    /// \return the current state of the eagle's behaviour
    EagleState getState();

    //! Check whether target is in TARGET_DEFENSE_REACH_RADIUS of the eagle
    /// \return whether target is in TARGET_DEFENSE_REACH_RADIUS of the eagle
    bool isInTargetDefenseReach();

    //! Check whether the target is in EAT_RADIUS of the eagle
    /// \return whether the target is in EAT_RADIUS of the eagle
    bool isTargetEaten();

    void resetEagle();
private:
    // GAMEPLAY
    // all durations given in seconds

    const float EAT_RADIUS = 1.0f;
    const float TARGET_DEFENSE_REACH_RADIUS = 13.0f;

    // min time after which eagle will attempt to attack player
    const float ATTACK_WAIT_TIME_MIN = 15.0f;
    // max time after which eagle will attempt to attack player
    const float ATTACK_WAIT_TIME_MAX = 25.0f;

    glm::mat4 eagleInitTransform;
    EagleState state = CIRCLING;

    float totalTimePassed = 0;
    float timeSinceLastAttack = 0;
    float timeIntervalToNextAttack = ATTACK_WAIT_TIME_MIN;

    glm::vec3 targetPos;
    bool targetHidden = false;
    bool targetDefenseActive = false;
};

