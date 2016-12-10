#ifndef GLFWPP_COMMON_HPP
#define GLFWPP_COMMON_HPP 1

#include <GLFW/glfw3.h>


namespace glfw
{

//! Size class encapsulates both width and height components.

//! This helps keep function calls simple by encapsulating both width and
//! height components in one object as they are most of the time required
//! together.
template <typename a_Tp>
class Comp2D {
public:
    //! Constructs a Size object
    Comp2D() = default;

    //! Constructs a Size object with preset values
    
    //! Components are in screen coordinates and must be greater
    //! than zero.
    Comp2D(
        a_Tp width, //!< [in] The width component.
        a_Tp height //!< [in] The height component.
    );

    // delete copy constructor and assignment
    Comp2D(const Comp2D&) = delete;
    Comp2D & operator=(const Comp2D&) = delete;

    // default move constructor and assignment
    Comp2D(Comp2D&&) = default;
    Comp2D & operator=(Comp2D&&) = default;

    enum Component : uint8_t {
        WIDTH = 0,
        HEIGHT = 1
    };

    //! Returns a size component
    
    //! \return component specified by the given enum's value.
    a_Tp get(
        Comp2D::Component comp //!< [in] The component to return.
    );

    a_Tp* get();

    //! Sets the size components
    
    //! Sets the size components given the width and height components' values.
    void set(
        a_Tp width, //!< [in] The width component.
        a_Tp height //!< [in] The height component.
    );

    //! Sets a size component

    //! Sets a size component given the component to set and it's value.
    void set(
        Comp2D::Component comp, //!< [in] The size component to set.
        a_Tp value //!< [in] The new value of the component to set to.
    );

    double getAspect(void);
private:
    a_Tp data[2] = {0};
};

//! Screen coordinate components

//! Components are expressed in integers, and are used to convey size and
//! location on screen.
using ScreenCoor = Comp2D<int>;

//! Cursor coordinate components

//! Components are expressed in double floating precision, and are used to
//! convey cursor position on screen.
using CursorCoor = Comp2D<double>;

}


#endif // GLFWPP_COMMON_HPP
