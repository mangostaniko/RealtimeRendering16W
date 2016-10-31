#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/constants.hpp>

#include <sstream>

/**
 * @brief A SceneObject holds a transformation matrix and its inverse
 * and provides functions to manipulate them.
 * This is intended as a base class for all objects that appear in the scene.
 */
class SceneObject
{
private:

	glm::mat4 modelMatrix;
	glm::mat4 inverseMatrix;

public:
	SceneObject(const glm::mat4 &modelMatrix_);
	virtual ~SceneObject();

	
	/**
	 * @brief enum specifying matrix multiplication order
	 */
	enum Order 
	{
		/**
		* incoming matrix is positioned left of current matrix in multiplication:
		* M_result = M_inc * M.
		*/
		LEFT, 
		
		/**
		* incoming matrix is positioned right of current matrix in multiplication:
		* M_result = M * M_inc.
		*/
		RIGHT
	};

	/**
	 * @brief apply a transformation matrix to the current matrix
	 * @param transform_ matrix defining a transformation
	 * @param inverse_ the inverse matrix of transform
	 * @param multOrder order of multiplication
	 */
	void applyTransformation(const glm::mat4 &transform_, const glm::mat4 &inverse_, Order multOrder);

	/**
	 * @return the const reference to the current model matrix
	 */
	const glm::mat4& getMatrix() const;

	/**
     * @brief replaces current matrix and sets its inverse
	 * @param matrix_ the new transformation matrix
	 */
	void setTransform(const glm::mat4 &matrix_);

	/**
	* @return the constant reference to the inverse matrix of the current model matrix
	*/
	const glm::mat4& getInverseMatrix() const;

	/**
	 * @brief return the location of the SceneObject
	 * by convention, that is the rightmost column of the model matrix,
	 * defined to always be multiplied with a factor of 1, such that in the
	 * linear combination of the matrix multiplication it acts like a translation
	 * independent of the xyz values of the given point, thus specifying the origin
	 * of the model matrix.
	 * @return the location of the SceneObject
	 */
	glm::vec3 getLocation() const;

	/**
	 * @brief set the location of the SceneObject
	 * i.e. rightmost column of the model matrix
	 * @return the location of the SceneObject
	 */
	void setLocation(const glm::vec3 &location);

	/**
	* @brief applies an X axis rotation operation to the current transformation
	* @param radians rotation angle
	* @param multOrder order of multiplication
	*/
	void rotateX(float radians, Order multOrder);

	/**
	* @brief applies a Y axis rotation operation to the current transformation
	* @param radians rotation angle
	* @param multOrder order of multiplication
	*/
	void rotateY(float radians, Order multOrder);

	/**
	* @brief applies a Z axis rotation operation to the current transformation
	* @param radians rotation angle
	* @param multOrder order of multiplication
	*/
	void rotateZ(float radians, Order multOrder);

	/**
	* @brief applies a rotation around a given vector (= axis)
	* @param radians rotation angle
	* @param multOrder order of multiplication
	* @param axis_ the axis around which to rotate
	*/
	void rotate(float radians, Order multOrder, const glm::vec3 &axis_);

	/**
	* @brief applies a translation operation to the current transformation
	* @param t translation vector
	* @param multOrder order of multiplication
	*/
	void translate(const glm::vec3 &t_, Order multOrder);

	/**
	 * @brief applies a scale operation to the current transformation 
	 * @param s scaling vector
	 * @param multOrder order of multiplication
	 */
	void scale(const glm::vec3 &s_, Order multOrder);

	/**
	 * @brief get a string to visualize the given matrix
	 * @param matrix the matrix to get a string representation of
	 * @return a string representing the given mastrix
	 */
	std::string matrixToString(const glm::mat4 &matrix);
};

#endif // SCENEOBJECT_H
