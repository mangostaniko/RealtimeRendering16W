#pragma once

#include "geometry.h"


//! The Light class.

//! This stores the light Position and Color, as well as a simple Geometry to visualize the light source object.
//! Color and Position may be set to vary smoothly over time
class Light : public Geometry
{
public:
	Light(const glm::mat4 &modelMatrix_, const std::string &geometryFilePath, glm::vec3 endPos, float cycleDuration_);

    //! update the state of the Light
    virtual void update(float timeDelta //!< [in] time passed since the last frame in seconds
	, bool enableColorChange);

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

    float cycleDuration; // duration of one whole day/night cycle
    float distance; // distance between start and endpoint (= distance to interpolate)
    float timePassed; // accumulated time change

	glm::vec3 currentColor; // current color interpolated according to time of day
	const glm::vec3 nightColor;
	const glm::vec3 morningColor;
	const glm::vec3 noonColor;
	const glm::vec3 eveningColor;
};

