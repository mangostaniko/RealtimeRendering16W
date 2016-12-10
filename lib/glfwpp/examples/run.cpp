int Application::run() {
    std::cout << "glfw::Application: run start." << std::endl;
    // Handle command line parameters
    int windowWidth = 800, windowHeight = 600;
    bool fullscreen = 0;

    if (argc_ == 1) {
        // no parameters specified, continue with default values
    } else if (
        argc_ != 4
        || (std::stringstream(argv_[1]) >> windowWidth).fail()
        || (std::stringstream(argv_[2]) >> windowHeight).fail()
        || (std::stringstream(argv_[3]) >> fullscreen).fail()
    ) {
        // if parameters are specified, must conform to given format
        std::cout << "USAGE: <resolution width> <resolution height> <fullscreen? 0/1>\n";
        exit(EXIT_FAILURE);
    }

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    window_ = new Window(
        windowWidth, windowHeight,
        std::string("GLFWPP Application"),
        (fullscreen ? monitor : NULL), NULL);
    window_->makeContextCurrent();
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << glewGetErrorString(err);
    }
    std::cout << "glfw::Application: run end." << std::endl;

    return EXIT_SUCCESS;
}
