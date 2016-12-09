find_package(Doxygen)


# check if doxygen is even installed
if (NOT DOXYGEN_FOUND)
    message(STATUS "Doxygen not found, API documentation cannot be generated.")
    return()
endif (NOT DOXYGEN_FOUND)

# prepare doxygen configuration file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/api/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/api/Doxyfile
    @ONLY
)

# Generate new target to build the html documentation
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/api/html/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/api/Doxyfile
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/api/Doxyfile
)

# add project's doc as target
add_custom_target(
    ${PROJECT_NAME}_doc
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/api/html/index.html
    COMMENT "Building ${PROJECT_NAME}'s API documentation."
)

if (NOT TARGET doc)
    add_custom_target(
        doc
        COMMENT "Building ${CMAKE_PROJECT_NAME}'s API documentation."
    )
endif ()

add_dependencies(doc ${PROJECT_NAME}_doc)

# cleanup ${PROJECT_BINARY_DIR}/docs/api on "make clean"
set_property(
    DIRECTORY APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES
    ${PROJECT_BINARY_DIR}/docs/api/html
    ${PROJECT_BINARY_DIR}/docs/api/latex
)

set(DOC_PATH "share/doc/${CPACK_PACKAGE_NAME}-${VERSION}")

# install HTML API documentation and manual pages
install(
    DIRECTORY   ${CMAKE_CURRENT_BINARY_DIR}/api/html
    DESTINATION ${DOC_PATH}
)

# install man pages into packages, scope is now project root..
install(
    DIRECTORY   ${CMAKE_CURRENT_BINARY_DIR}/api/man/man3
    DESTINATION share/man/man3/
)
