#ifndef GLFWPP_TIME_HPP
#define GLFWPP_TIME_HPP 1

#include <glfwpp/common.hpp>


namespace glfw
{

//! 
struct Time {
    Time() = delete;
    Time(const Time&) = delete;
    Time & operator=(const Time&) = delete;

    //! This function returns the value of the GLFW timer.

    //! double glfwGetTime(void)
    //! \return The current value, in seconds, or zero if an error occurred.
    static double get(void);

    //! This function sets the value of the GLFW timer.
    
    //! void glfwSetTime(double time)
    static void set(
        double time //!< [in] The new value, in seconds.
    );
};
}


#endif // GLFWPP_TIME_HPP
