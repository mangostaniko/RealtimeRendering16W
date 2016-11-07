#ifndef SUZANNEISLAND_LIGHT_HPP
#define SUZANNEISLAND_LIGHT_HPP 1

#include "sceneobject.hpp"


//! The Light class.

//! This stores the light Position and Color, in update method position and
//! color change to simulate day time change.
class Light : public SceneObject
{
public:
    Light(const glm::mat4 &modelMatrix_, glm::vec3 endPos, glm::vec3 startCol, glm::vec3 endCol, float cycleDuration_);

    //! update the state of the Light
    virtual void update(
        float timeDelta //!< [in] time passed since the last frame in seconds
    );

    //! Returns the current color as calculated
    glm::vec3 getColor() const;
private: 
    // enum for the 4 sections of each day
    enum DayTime {
        MORNING = 0, AFTERNOON = 1, EVENING = 2, NIGHT = 3
    } dayTime;

    glm::vec3 startPosition;
    glm::vec3 endPosition;
    glm::vec3 direction; // unit vector pointing from start to end

    glm::vec3 startColor;
    glm::vec3 endColor;
    glm::vec3 currentColor; // current color as calculated by interpolation

    float cycleDuration; // duration of one whole day/night cycle
    float distance; // distance between start and endpoint (= distance to interpolate)
    float timePassed; // accumulated time change

    const glm::vec3 noonColor; // color of the noon (sky blue)
    const glm::vec3 nightColor; // color of the night (dark grey)
};


#endif // SUZANNEISLAND_LIGHT_HPP
