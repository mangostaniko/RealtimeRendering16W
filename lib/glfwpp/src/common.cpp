#include <glfwpp/common.hpp>

using namespace glfw;


template <typename a_Tp>
Comp2D<a_Tp>::Comp2D(a_Tp width, a_Tp height) {
    set(width, height);
}

template <typename a_Tp>
a_Tp Comp2D<a_Tp>::get(Comp2D<a_Tp>::Component comp) {
    return data[comp];
}

template <typename a_Tp>
a_Tp* Comp2D<a_Tp>::get() {
    return data;
}

template <typename a_Tp>
void Comp2D<a_Tp>::set(a_Tp width, a_Tp height) {
    data[0] = width;
    data[1] = height;
}

template <typename a_Tp>
void Comp2D<a_Tp>::set(Comp2D<a_Tp>::Component comp, a_Tp value) {
    data[comp] = value;
}

template <typename a_Tp>
double Comp2D<a_Tp>::getAspect(void) {
    return data[0]/static_cast<double>(data[1]);
}
