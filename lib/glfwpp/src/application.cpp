#include <glfwpp/application.hpp>

using namespace glfw;


Application::Application(int argc, char **argv) : argc_(argc), argv_(argv) {
    std::cout << "glfw::Application: beginning constructor." << std::endl;
    glfwSetErrorCallback(error_callback);
    // Init window and OpenGL context
    // TODO: use maybe exceptions instead?
    if (!glfwInit()) {
        // Initialization failed
        exit(EXIT_FAILURE);
    }

    std::cout << "glfw::Application: finishing constructor." << std::endl;
}

Application::~Application() {
    glfwTerminate();
}

void Application::error_callback(int, const char* description)
{
    std::cerr << "glfw::Application: Error: " << description << std::endl;
}
    
//! Returns all currently connected monitors.
std::vector<GLFWmonitor *> Application::getMonitors(void) {
    static std::vector<GLFWmonitor *> monitors;

    int count;
    GLFWmonitor** monitors_ = glfwGetMonitors(&count);

    if (static_cast<unsigned int>(count) == monitors.size()) {
        for (int idx = 0; idx < count; ++idx) {
            monitors.push_back(monitors_[idx]);
        }
    }

    return monitors;
}

Window * Application::getCurrentContext(void) {
    GLFWwindow * tmp_window = glfwGetCurrentContext();
    Window *a_window = static_cast<Window *>(
        glfwGetWindowUserPointer(tmp_window)
    );
    return a_window;
}

int Application::ExtensionSupported(const std::string & extension) {
    return glfwExtensionSupported(extension.c_str());
}
