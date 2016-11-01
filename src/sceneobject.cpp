#include "sceneobject.h"

SceneObject::SceneObject(const glm::mat4 &modelMatrix_)
    : modelMatrix(modelMatrix_)
{
	inverseMatrix = glm::inverse(modelMatrix_);
}

SceneObject::~SceneObject()
{

}


const glm::mat4& SceneObject::getMatrix() const
{
	return modelMatrix;
}

const glm::mat4& SceneObject::getInverseMatrix() const
{
	return inverseMatrix;
}

glm::vec3 SceneObject::getLocation() const
{
	return modelMatrix[3].xyz;
}

void SceneObject::setLocation(const glm::vec3 &location)
{
	modelMatrix[3] = glm::vec4(location, 1.0f);
}

void SceneObject::setTransform(const glm::mat4 &matrix_) {
	modelMatrix = matrix_;
	inverseMatrix = glm::inverse(modelMatrix);
}

void SceneObject::applyTransformation(const glm::mat4 &transform_, const glm::mat4 &inverse_, Order multOrder)
{
	if (multOrder == LEFT) {
		modelMatrix        = transform_*modelMatrix;
		inverseMatrix    = inverseMatrix*inverse_;
	}
	else {
		modelMatrix        = modelMatrix * transform_;
		inverseMatrix    = inverse_*inverseMatrix;
	}
}

void SceneObject::rotateX(float radians, Order multOrder)
{
	applyTransformation(glm::rotate(glm::mat4(), radians, glm::vec3(1.0f, 0.0f, 0.0f)), glm::rotate(glm::mat4(), -radians, glm::vec3(1.0f, 0.0f, 0.0f)), multOrder);
}

void SceneObject::rotateY(float radians, Order multOrder)
{
	applyTransformation(glm::rotate(glm::mat4(), radians, glm::vec3(0.0f, 1.0f, 0.0f)), glm::rotate(glm::mat4(), -radians, glm::vec3(0.0f, 1.0f, 0.0f)), multOrder);
}

void SceneObject::rotateZ(float radians, Order multOrder)
{
	applyTransformation(glm::rotate(glm::mat4(), radians, glm::vec3(0.0f, 0.0f, 1.0f)), glm::rotate(glm::mat4(), -radians, glm::vec3(0.0f, 0.0f, 1.0f)), multOrder);
}

void SceneObject::rotate(float radians, Order multOrder, const glm::vec3 &axis_)
{
	applyTransformation(glm::rotate(glm::mat4(), radians, axis_), glm::rotate(glm::mat4(), -radians, axis_), multOrder);
}

void SceneObject::translate(const glm::vec3 &t_, Order multOrder)
{
	applyTransformation(glm::translate(glm::mat4(), t_), glm::translate(glm::mat4(), -t_), multOrder);
}

void SceneObject::scale(const glm::vec3 &s_, Order multOrder)
{
	applyTransformation(glm::scale(glm::mat4(), s_), glm::scale(glm::mat4(), 1.0f / s_), multOrder);
}

std::string SceneObject::matrixToString(const glm::mat4 &matrix)
{
	std::stringstream matStr;

	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			matStr << matrix[col][row] << " ";
		}
		matStr << std::endl;
	}

	return matStr.str();
}

