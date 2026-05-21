

# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)

    make_directory(${CMAKE_SOURCE_DIR}/out/doc)

    # note the option ALL which allows to build the docs together with the application
    add_custom_target(doxygen
        COMMAND ${CMAKE_COMMAND} -E env MADGINE_VERSION=${MADGINE_VERSION} -- ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
        USES_TERMINAL)

    add_custom_target(doxysphinx        
        COMMAND doxysphinx build . out/doc/html out/doc/doxygen
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running doxysphinx"
        VERBATIM
        USES_TERMINAL)

    add_dependencies(doxysphinx doxygen)

    add_custom_target(sphinx       
        COMMAND ${CMAKE_COMMAND} -E env MADGINE_VERSION=${MADGINE_VERSION} -- sphinx-build -b html . out/doc/html -c docs
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating documentation with Sphinx"
        VERBATIM
        USES_TERMINAL)

    #add_dependencies(sphinx doxysphinx)

else (DOXYGEN_FOUND)
  message("Doxygen needs to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
