#include <glfwpp/window.hpp>

using namespace glfw;


Window::Window(
    int width, int height,
    const std::string &title,
    GLFWmonitor *monitor, glfw::Window *share
) {
    std::cout << "glfw::Window: beginning constructor." << std::endl;
    window_ = nullptr;
    if (share != NULL) {
        window_ = glfwCreateWindow(width, height, title.c_str(), monitor, share->window());
    }
    else {
        window_ = glfwCreateWindow(width, height, title.c_str(), monitor, NULL);
    }

    // TODO: use exceptions instead
    if (!window_) {
        std::cerr
            << "glfw::Window: ERROR: Failed to open GLFW window."
            << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Use glfwGetUserPointer to get wrapper's state within static
    // callbacks.
    glfwSetWindowUserPointer(window_, this);
    std::cout << "glfw::Window: finishing constructor." << std::endl;
}

Window::~Window() {
    glfwDestroyWindow(window_);
}

//Window::operator GLFWwindow*() { return window_; }
//Window::operator GLFWwindow*&() { return window_; }

void Window::swap_buffers() {
    glfwSwapBuffers(window_);
}

int Window::shouldClose(void) {
    return glfwWindowShouldClose(window_);
}

void Window::setShouldClose(int value) {
    glfwSetWindowShouldClose(window_, value);
}

GLFWwindow * Window::window(void) {
    return window_;
}

//! Get window monitor
GLFWmonitor* Window::getMonitor(void) {
    return glfwGetWindowMonitor(window_);
}

//! Set window monitor
void Window::setMonitor(
    GLFWmonitor * monitor,
    int xpos, int ypos,
    int width, int height,
    int refreshRate
) {
    glfwSetWindowMonitor(
        window_, monitor,
        xpos, ypos,
        width, height,
        refreshRate
    );
}

//! Window position
glfw::ScreenCoor Window::getPos() {
    int width, height;
    glfwGetWindowPos(window_, &width, &height);
    return glfw::ScreenCoor(width, height);
}

void Window::setPos(int xpos, int ypos) {
    glfwSetWindowPos(window_, xpos, ypos);
}

//! Window size
glfw::ScreenCoor Window::getSize(void) {
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    return glfw::ScreenCoor(width, height);
}

void Window::setSize(int width, int height) {
    glfwSetWindowSize(window_, width, height);
}

void Window::setSizeLimits(int min_w, int min_h, int max_w, int max_h) {
    glfwSetWindowSizeLimits(window_, min_w, min_h, max_w, max_h);
}

//! Callback registration functions
GLFWwindowposfun Window::setPosCallback(GLFWwindowposfun cbfun) {
    return glfwSetWindowPosCallback(window_, cbfun);
}

GLFWwindowsizefun Window::setSizeCallback(GLFWwindowsizefun cbfun) {
    return glfwSetWindowSizeCallback(window_, cbfun);
}

// --- Context management code ------------------------------------------------

void Window::makeContextCurrent() {
    glfwMakeContextCurrent(window_);
}

// --- Input mode setup code --------------------------------------------------

void Window::setInputMode(int mode, int value) {
    glfwSetInputMode(window_, mode, value);
}

// --- Mouse controls callback registration and management code ---------------

glfw::CursorCoor Window::getCursorPos(void) {
    double xpos, ypos;
    glfwGetCursorPos(window_, &xpos, &ypos);
    return glfw::CursorCoor(xpos, ypos);
}

void Window::setCursorPos(double xpos, double ypos) {
    glfwSetCursorPos(window_, xpos, ypos);
}

void Window::setCursor(GLFWcursor *cursor) {
    glfwSetCursor(window_, cursor);
}

// --- Key controls callback registration and management code -----------------

int Window::getKey(int key) {
    return glfwGetKey(window_, key);
}

const char * Window::getKeyName(int key, int scancode) {
    return glfwGetKeyName(key, scancode);
}

//! Registers C-style key action callback functions
GLFWkeyfun Window::setKeyCallback(GLFWkeyfun cbfun) {
    return glfwSetKeyCallback(window_, cbfun);
}

//! Default key action callback function
void Window::onKeyAction(int key, int, int action, int)
{
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_F1: {
                    std::cout
                        << "glfw::Window: Hello! This is all the help"
                        << "you'll be getting buddy!"
                        << std::endl;
                break;
            }
            case GLFW_KEY_ESCAPE:
                setShouldClose(GLFW_TRUE);
                break;
        }
    }
}

void Window::keyEvent(
    GLFWwindow *a_window,
    int key, int scancode,
    int action, int mods
) {
    glfw::Window *win_obj = static_cast<glfw::Window *>(
        glfwGetWindowUserPointer(a_window)
    );
    win_obj->onKeyAction(key, scancode, action, mods);
}

// ----------------------------------------------------------------------------

GLFWscrollfun Window::setScrollCallback(GLFWscrollfun cbfun) {
    return glfwSetScrollCallback(window_, cbfun);
}

// --- Framebuffer callback registration and management code ------------------

//! Registers C-style framebuffer resize callback functions
GLFWframebuffersizefun Window::setFramebufferSizeCallback(GLFWframebuffersizefun cbfun) {
    return glfwSetFramebufferSizeCallback(window_, cbfun);
}

//! Default frame buffer resize callback function
void Window::onFramebufferResize(
    int width, //!< [in] new framebuffer width
    int height //!< [in] new framebuffer height
) {
    std::cout
        << "glfw::Window: FRAMEBUFFER RESIZED: "
        << width << ", " << height
        << std::endl;
    glViewport(0, 0, width, height);
}

// ----------------------------------------------------------------------------
    
//! Window hints setting and resetting
void Window::setHint(int hint, int value) {
    glfwWindowHint(hint, value);
}

void Window::setHintsDefault(void) {
    glfwDefaultWindowHints();
}

//! Redirects C style callback functions to wrapper object methods.
void Window::framebufferResize(
    GLFWwindow *a_window, int width, int height) {
    glfw::Window *window = static_cast<glfw::Window *>(
        glfwGetWindowUserPointer(a_window)
    );
    window->onFramebufferResize(width, height);
}

