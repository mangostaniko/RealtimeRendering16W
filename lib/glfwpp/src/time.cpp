#include <glfwpp/time.hpp>

using namespace glfw;


double Time::get(void) {
    return glfwGetTime();
}

void Time::set(double time) {
    glfwSetTime(time);
}
