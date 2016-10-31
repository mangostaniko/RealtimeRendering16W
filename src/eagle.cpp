#include "eagle.h"

Eagle::Eagle(const glm::mat4 &matrix, const std::string &filePath)
	: Geometry(matrix, filePath)
{
	eagleInitTransform = matrix;
}

Eagle::~Eagle()
{

}

void Eagle::update(float timeDelta, const glm::vec3 &targetPos_, bool targetHidden_, bool targetDefenseActive_)
{
	totalTimePassed += timeDelta;
	targetPos = targetPos_;
	targetHidden = targetHidden_;
	targetDefenseActive = targetDefenseActive_;

	if (state == CIRCLING) {

		// circle in the sky
		rotate(0.5f * timeDelta, SceneObject::LEFT, glm::vec3(0, 1, 0));
//		translate(glm::vec3(0, glm::cos(totalTimePassed)/80, 0), SceneObject::RIGHT);
//		rotateZ(glm::cos(-totalTimePassed)/4000, SceneObject::RIGHT);
//		rotateX(glm::cos(-totalTimePassed)/4000, SceneObject::RIGHT);

		// wait for next time to attack
		timeSinceLastAttack += timeDelta;

		if (!targetHidden && timeSinceLastAttack > timeIntervalToNextAttack) {
			timeSinceLastAttack = 0;
			int t = (float)rand()/RAND_MAX;
			timeIntervalToNextAttack = t * ATTACK_WAIT_TIME_MIN + (1-t) * ATTACK_WAIT_TIME_MAX;
			state = ATTACKING;
		}
	}
	else if (state == ATTACKING) {

		// retreat if target hidden or target in reach and defends themselves
		if (targetHidden || (targetDefenseActive && isInTargetDefenseReach())) {
			state = RETREATING;
		}

		// orient to look at target
		glm::vec3 attackDirection = glm::normalize(targetPos - getLocation());
		glm::mat4 attackMat = getMatrix();
		attackMat[0] = -glm::vec4(attackDirection.x, 0, attackDirection.z, 0);
		attackMat[1] = glm::vec4(0, 1, 0, 0);
		attackMat[2] = glm::vec4(glm::cross(attackMat[0].xyz(), attackMat[1].xyz()), 0);
		setTransform(attackMat);

		// fly to target
		float speed = (glm::max)(glm::sqrt(glm::distance(targetPos, getLocation()))*2.0f, 10.0f);
		translate(attackDirection*speed*timeDelta, SceneObject::LEFT);

	}
	else if (state == RETREATING) {

		if (glm::distance(getLocation(), targetPos) < TARGET_DEFENSE_REACH_RADIUS*3) {

			// if we are close, fly away
			glm::vec3 retreatDirection = glm::normalize(getLocation() - targetPos);
			retreatDirection.y = 4.0f;
			translate(retreatDirection*1.5f*timeDelta, SceneObject::LEFT);
		}
		else {

			// else jump back to circling state and matrix
			state = CIRCLING;
			setTransform(eagleInitTransform);
		}


	}

}

EagleState Eagle::getState()
{
	return state;
}

bool Eagle::isInTargetDefenseReach()
{
	return glm::distance(getLocation(), targetPos) < TARGET_DEFENSE_REACH_RADIUS;
}

bool Eagle::isTargetEaten()
{
	return glm::distance(getLocation(), targetPos) < EAT_RADIUS;
}

void Eagle::resetEagle()
{
	state = CIRCLING;

	totalTimePassed = 0;
	timeSinceLastAttack = 0;
	timeIntervalToNextAttack = ATTACK_WAIT_TIME_MIN;

	targetPos = glm::vec3(0);
	targetHidden = false;
	targetDefenseActive = false;
}

