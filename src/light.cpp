#include "light.h"


Light::Light(const glm::mat4 &modelMatrix_, const std::string &geometryFilePath, glm::vec3 endPos, float cycleDuration_)
    : Geometry(modelMatrix_, geometryFilePath)
    , endPosition(endPos)
    , cycleDuration(cycleDuration_)
    , timePassed(0.f)
    , nightColor  (0.1f, 0.1f, 0.2f)
    , morningColor(0.4f, 0.5f, 0.6f)
    , noonColor   (0.7f, 0.7f, 0.5f)
    , eveningColor(0.6f, 0.2f, 0.0f)
    , currentColor(nightColor)
{
    startPosition = getLocation();
    direction = glm::normalize(endPosition - getLocation());
    distance = glm::abs(glm::length(endPosition - getLocation()));
    dayTime = NIGHT;
}

void Light::update(float timeDelta)
{
    timePassed += timeDelta;

    float daySectionDuration = cycleDuration * 0.25f;
    float t = (timePassed / daySectionDuration);

    /*
        Following if statements represent the daily cycle from Midnight to Midnight
        Color and Position are interpolated according to time passed, Position has to be reset to start 
        at Midnight
    */
	if (dayTime == MORNING) {
		currentColor = morningColor * (1.f - t) + t * noonColor;

        if (timePassed > daySectionDuration) {
            timePassed = 0.f;
            dayTime = AFTERNOON;
        }
    }
	else if (dayTime == AFTERNOON) {
		currentColor = noonColor * (1.f - t) + t * eveningColor;

        if (timePassed > daySectionDuration) {
            timePassed = 0.f;
            dayTime = EVENING;
        }
    }
	else if (dayTime == EVENING) {
		currentColor = eveningColor * (1.f - t) + t * nightColor;
        
        if (timePassed > daySectionDuration) {
            timePassed = 0.f;
            dayTime = NIGHT;
            setLocation(startPosition);
        }
    }
	else if (dayTime == NIGHT) {
		currentColor = nightColor * (1.f - t) + t * morningColor;

        if (timePassed > daySectionDuration) {
            timePassed = 0.f;
            dayTime = MORNING;
        }
    }

    t = timeDelta / (daySectionDuration * 2.f);
    glm::vec3 transDist = direction * (distance * t);
    translate(transDist, SceneObject::RIGHT);

}

glm::vec3 Light::getColor() const
{
    return currentColor;
}
