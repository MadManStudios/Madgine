

# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)

	configure_file(${CMAKE_CURRENT_LIST_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.in)
	file(GENERATE OUTPUT Doxyfile INPUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.in)

    # note the option ALL which allows to build the docs together with the application
    add_custom_target(doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
        USES_TERMINAL)

    add_custom_target(sphinx
        COMMAND doxysphinx build docs doc docs/doxygen
        COMMAND sphinx-build -b html . doc -c docs
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating documentation with Sphinx including doxygen output"
        VERBATIM
        USES_TERMINAL)

    add_dependencies(sphinx doxygen)

    add_custom_target(sphinx_only
        COMMAND sphinx-build -b html . doc -c docs
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating documentation with Sphinx"
        VERBATIM
        USES_TERMINAL)
        
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
