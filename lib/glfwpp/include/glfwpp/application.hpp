#ifndef GLFWPP_APPLICATION_HPP
#define GLFWPP_APPLICATION_HPP 1

#include <string>
#include <iostream>
#include <sstream>

#include <glfwpp/common.hpp>
#include <glfwpp/window.hpp>


namespace glfw
{

class Application {
public:
    Application(int argc, char **argv);
    virtual ~Application();

    Application(const Application&) = delete;
    Application & operator=(const Application&) = delete;

    //! Returns all currently connected monitors.

    //! \return an array of handles for all currently connected monitors.
    static std::vector<GLFWmonitor *> getMonitors(void);

    glfw::Window * getCurrentContext(void);
    virtual int run(void)=0;

    static int ExtensionSupported(const std::string & extension);
    static void error_callback(int error, const char* description);

protected:
    glfw::Window *window_;
    int argc_;
    char **argv_;
};

}


#endif // GLFWPP_APPLICATION_HPP