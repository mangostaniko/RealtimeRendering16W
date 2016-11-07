#ifndef SUZANNEISLAND_SCENEOBJECT_HPP
#define SUZANNEISLAND_SCENEOBJECT_HPP 1

#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>


//! A base class for all scene objects.

//! A SceneObject holds the transformation matrix and its inverse and provides
//! functions to manipulate them.
class SceneObject
{
public:
    SceneObject(const glm::mat4 &modelMatrix_)
        : modelMatrix(modelMatrix_)
    {
        inverseMatrix = glm::inverse(modelMatrix_);
    }

    virtual ~SceneObject()
    {}

    //! enum specifying matrix multiplication order.
    
    //! Incoming matrix is positioned left or right of current matrix in
    //! multiplication
    enum Order
    {
        LEFT, //!< M_result = M_inc * M
        RIGHT //!< M_result = M * M_inc
    };

    //! Apply a transformation matrix to the current matrix
    void applyTransformation(
        const glm::mat4 &trans_mat, //!< [in] a transformation matrix
        const glm::mat4 &invtrans_mat, //!< [in] a transform's inverse matrix
        Order mult_order //!< [in] multiplication's order
    );

    //! \return the const reference to the current model matrix
    const glm::mat4& getMatrix() const;

    //! Replaces current matrix and sets its inverse
    void setTransform(
        const glm::mat4 &new_trans_mat //!< [in] new transformation matrix
    );

    //! \return the inverse matrix of the current model matrix
    const glm::mat4& getInverseMatrix() const;

    //! Returns the location of the SceneObject.

    //! By convention the location of the SceneObject is the rightmost column
    //! of the model matrix, defined to always be multiplied with a factor
    //! of 1, such that in the linear combination of the matrix multiplication
    //! it acts like a translation independent of the xyz values of the given
    //! point, thus specifying the origin of the model matrix.
    //! \return the location of the SceneObject as a 3D vector
    glm::vec3 getLocation() const;

    //! Set SceneObject's location: rightmost column of the model matrix
    //! \return the location of the SceneObject
    void setLocation(const glm::vec3 &location);


    //! Applies an X axis rotation operation to the current transformation
    void rotateX(
        float radians, //!< [in] rotation angle in radians
        Order mult_order //!< [in] multiplication order
    );

    //! Applies a Y axis rotation operation to the current transformation
    void rotateY(
        float radians, //!< [in] rotation angle in radians
        Order mult_order //!< [in] multiplication order
    );

    //! Applies a Z axis rotation operation to the current transformation
    void rotateZ(
        float radians, //!< [in] rotation angle in radians
        Order mult_order //!< [in] multiplication order
    );

    //!Applies a rotation around a given vector (= axis)
    void rotate(
        float radians, //!< [in] rotation angle in radians
        Order mult_order, //!< [in] multiplication order
        const glm::vec3 &rot_axis //!< [in] axis around which to rotate
    );

    //! Applies a translation operation to the current transformation
    void translate(
        const glm::vec3 &trans_vec, //!< [in] translation vector
        Order mult_order //!< [in] multiplication order
    );


    //! Applies a scale operation to the current transformation
    void scale(
        const glm::vec3 &scaling_vec, //!< [in] scaling vector
        Order mult_order //!< [in] multiplication order
    );

    //! Get a string to visualize the given matrix
    //! \return a string representing the given matrix
    std::string matrixToString(
        const glm::mat4 &matrix //!< [in] matrix to get a string representation
    );
private:
    glm::mat4 modelMatrix;
    glm::mat4 inverseMatrix;
};

inline const glm::mat4& SceneObject::getMatrix() const
{
    return modelMatrix;
}

inline const glm::mat4& SceneObject::getInverseMatrix() const
{
    return inverseMatrix;
}

inline glm::vec3 SceneObject::getLocation() const
{
    return modelMatrix[3].xyz;
}

inline void SceneObject::setLocation(const glm::vec3 &location)
{
    modelMatrix[3] = glm::vec4(location, 1.0f);
}

inline void SceneObject::setTransform(const glm::mat4 &new_trans_mat) {
    modelMatrix = new_trans_mat;
    inverseMatrix = glm::inverse(modelMatrix);
}

inline void SceneObject::applyTransformation(
    const glm::mat4 &trans_mat,
    const glm::mat4 &invtrans_mat,
    Order mult_order)
{
    if (mult_order == LEFT) {
        modelMatrix = trans_mat * modelMatrix;
        inverseMatrix = inverseMatrix * invtrans_mat;
    }
    else {
        modelMatrix = modelMatrix * trans_mat;
        inverseMatrix = invtrans_mat * inverseMatrix;
    }
}

inline void SceneObject::rotateX(float radians, Order mult_order)
{
    applyTransformation(
        glm::rotate(glm::mat4(), radians, glm::vec3(1.0f, 0.0f, 0.0f)),
        glm::rotate(glm::mat4(), -radians, glm::vec3(1.0f, 0.0f, 0.0f)),
        mult_order
    );
}

inline void SceneObject::rotateY(float radians, Order mult_order)
{
    applyTransformation(
        glm::rotate(glm::mat4(), radians, glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::rotate(glm::mat4(), -radians, glm::vec3(0.0f, 1.0f, 0.0f)),
        mult_order
    );
}

inline void SceneObject::rotateZ(float radians, Order mult_order)
{
    applyTransformation(
        glm::rotate(glm::mat4(), radians, glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::rotate(glm::mat4(), -radians, glm::vec3(0.0f, 0.0f, 1.0f)),
        mult_order
    );
}

inline void SceneObject::rotate(float radians, Order mult_order, const glm::vec3 &rot_axis)
{
    applyTransformation(
        glm::rotate(glm::mat4(), radians, rot_axis),
        glm::rotate(glm::mat4(), -radians, rot_axis),
        mult_order
    );
}

inline void SceneObject::translate(const glm::vec3 &trans_vec, Order mult_order)
{
    applyTransformation(
        glm::translate(glm::mat4(), trans_vec),
        glm::translate(glm::mat4(), -trans_vec),
        mult_order
    );
}

inline void SceneObject::scale(const glm::vec3 &scaling_vec, Order mult_order)
{
    applyTransformation(
        glm::scale(glm::mat4(), scaling_vec),
        glm::scale(glm::mat4(), 1.0f / scaling_vec),
        mult_order
    );
}

inline std::string SceneObject::matrixToString(const glm::mat4 &matrix)
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


#endif // SUZANNEISLAND_SCENEOBJECT_HPP
