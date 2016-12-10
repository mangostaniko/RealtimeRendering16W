#ifndef GLFWPP_WINDOW_HPP
#define GLFWPP_WINDOW_HPP 1

#include <string>
#include <iostream>
#include <vector>

#include <glfwpp/common.hpp>


namespace glfw
{

class Window {
public:
    Window(int width, int height, const std::string &title, GLFWmonitor *monitor, glfw::Window *share);
    ~Window();

    Window(const Window&) = delete;
    Window & operator=(const Window&) = delete;
    virtual int render(int videoModeWidth, int videoModeHeight)=0;

    void swap_buffers();

    int shouldClose(void);

    //! Sets the value of the close flag.
    void setShouldClose(
        int value //!< [in] Usual values are GLFW_TRUE and GLFW_FALSE
    );

    GLFWwindow * window(void);

    //! Gets window's monitor

    //! \return The monitor, or NULL if the window is in windowed mode or
    //! an error occurred.
    GLFWmonitor* getMonitor(void);

    //! Sets window's monitor

    //! Values are expressed in screen coordinates, of the upper-left corner
    //! of the client area or video mode.
    //! Refresh rate, in Hz, of the video mode, or GLFW_DONT_CARE
    void setMonitor(
        GLFWmonitor * monitor, //!< [in] The desired monitor, or NULL to set windowed mode.
        int xpos, int ypos, //!< [in] The desired x and y-coordinates.
        int width, int height, //!< [in] The desired with and height.
        int refreshRate //!< [in] The desired refresh rate, in Hz.
    );

    //! Returns the window position

    //! \return The coordinates along the width and height components.
    glfw::ScreenCoor getPos();

    //! Sets the window position

    //! Values are expressed in screen coordinates, of the upper-left corner
    //! of the client area of the specified windowed mode window. 
    void setPos(
        int xpos, //!< [in] The x-coordinate.
        int ypos //!< [in] The y-coordinate.
    );

    //! Returns the window size

    //! \return Returns the width and height of the window as the size object.
    glfw::ScreenCoor getSize(void);

    //! Sets the window size

    //! Values are expressed in screen coordinates, of the client area.
    void setSize(
        int width, //!< [in] The desired width.
        int height //!< [in] The desired height.
    );

    //! Sets the size limits of the client area of the specified window.
    
    //! Values are expressed in screen coordinates,
    //! of the client area.
    void setSizeLimits(
        int minwidth, //!< [in] The minimum width or GLFW_DONT_CARE.
        int minheight, //!< [in] The minimum height, or GLFW_DONT_CARE.
        int maxwidth, //!< [in] The maximum width, or GLFW_DONT_CARE.
        int maxheight //!< [in] The maximum height, or GLFW_DONT_CARE.
    );

    //! Callback registration functions
    GLFWwindowposfun setPosCallback(
        GLFWwindowposfun cbfun //!< [in] window position callback function.
    );

    GLFWwindowsizefun setSizeCallback(
        GLFWwindowsizefun cbfun //!< [in] window size callback function.
    );

    // --- Context management code --------------------------------------------

    void makeContextCurrent();

    // --- Input mode setup code ----------------------------------------------

    //! Sets an input mode option for the specified window.

    //! Wraps void glfwSetInputMode(GLFWwindow *window, int mode, int value)
    //! The mode must be one of GLFW_CURSOR, GLFW_STICKY_KEYS or
    //! GLFW_STICKY_MOUSE_BUTTONS.
    void setInputMode(
        int mode, //!< [in] One of GLFW_CURSOR, GLFW_STICKY_KEYS or GLFW_STICKY_MOUSE_BUTTONS.
        int value //!< [in] The new value of the specified input mode.
    );

    // --- Mouse controls callback registration and management code -----------

    //! Returns the cursor's position in this window

    //! Value returned is relative to the upper-left corner of the client
    //! area of this window.
    //! \return position in screen coordinates.
    glfw::CursorCoor getCursorPos(void);

    //! Sets the cursor's position in this window.

    //! Values are expressed in screen coordinates, of the cursor relative to
    //! the upper-left corner of the client area of this window.
    void setCursorPos(
        double xpos, //!< [in] The desired x-coordinate.
        double ypos //!< [in] The desired y-coordinate.
    );

    //! Sets the cursor image when over the client area of this window

    //! The set cursor will only be visible when the cursor mode of the
    //! window is GLFW_CURSOR_NORMAL.
    void setCursor(
        GLFWcursor *cursor //!< [in] The cursor to set, or NULL to switch back to the default arrow cursor.
    );

    // --- Key controls callback registration and management code -------------

    //! Returns the last state for the specified key and window.

    //! Wraps int glfwGetKey(GLFWwindow * window, int key)
    //! \return One of GLFW_PRESS or GLFW_RELEASE.
    int getKey(
        int key //!< [in] The desired keyboard key.
    );

    //! This function returns the localized name of the specified printable key.

    //! Wraps const char* glfwGetKeyName(int key, int scancode)
    //! \return The localized name of the key, or NULL.
    const char * getKeyName(
        int key, //!< [in] The key to query, or GLFW_KEY_UNKNOWN.
        int scancode //!< [in] The scancode of the key to query.
    );

    //! Registers C-style key action callback functions
    GLFWkeyfun setKeyCallback(
        GLFWkeyfun cbfun //!< [in] keyboard key callback function.
    );

    //! Default key action callback function
    virtual void onKeyAction(
        int key, //!< [in] key code of the key that caused the event
        int scancode, //!< [in] a system and platform specific constant
        int action, //!< [in] type of key event: GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
        int mods //!< [in] modifier keys held down on event
    );

    //! Redirects C style callback functions to wrapper object methods.
    static void keyEvent(
        GLFWwindow *window, //!< [in] active window pointer
        int key, //!< [in] key code of the key that caused the event
        int scancode, //!< [in] a system and platform specific constant
        int action, //!< [in] type of key event: GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
        int mods //!< [in] modifier keys held down on event
    );

    // ------------------------------------------------------------------------

    GLFWscrollfun setScrollCallback(
        GLFWscrollfun cbfun //!< [in] scroll callback function
    );

    // --- Framebuffer callback registration and management code --------------

    //! Registers C-style framebuffer resize callback functions
    GLFWframebuffersizefun setFramebufferSizeCallback(
        GLFWframebuffersizefun cbfun //!< [in] framebuffer resize callback
    );

    //! Default frame buffer resize callback function
    virtual void onFramebufferResize(
        int width, //!< [in] new framebuffer width
        int height //!< [in] new framebuffer height
    );

    //! Redirects C style callback functions to wrapper object methods.
    static void framebufferResize(
        GLFWwindow *a_window, //!< [in] active window pointer
        int width, //!< [in] new framebuffer width
        int height //!< [in] new framebuffer height
    );

    // ------------------------------------------------------------------------

    //! Window hints setting and resetting
    static void setHint(
        int hint, //!< [in] The window hint to set.
        int value //!< [in] The new value of the window hint.
    );

    static void setHintsDefault(void);

protected:
    GLFWwindow *window_;
};

}


#endif // GLFWPP_WINDOW_HPP
