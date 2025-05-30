FIND_PACKAGE(OpenGL)
FIND_PACKAGE(GLUT)

IF ( NOT GLUT_FOUND )
  IF ( MSVC )
    SET ( GLUT_FOUND 1 )
    SET ( GLUT_INCLUDE_DIR ${Assimp_SOURCE_DIR}/samples/freeglut/ )
    SET ( GLUT_LIBRARIES ${Assimp_SOURCE_DIR}/samples/freeglut/lib/freeglut.lib )
  ELSE ()
    MESSAGE( WARNING "Please install glut." )
  ENDIF ()
ENDIF ()

if ( MSVC )
  ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
  ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
endif ()

INCLUDE_DIRECTORIES(
  ${Assimp_SOURCE_DIR}/code
  ${OPENGL_INCLUDE_DIR}
  ${GLUT_INCLUDE_DIR}
  ${SAMPLES_SHARED_CODE_DIR}
)

LINK_DIRECTORIES(
  ${Assimp_BINARY_DIR}
  ${Assimp_BINARY_DIR}/lib/
)

ADD_EXECUTABLE( assimp_simpletexturedogl WIN32
  SimpleTexturedOpenGL/src/model_loading.cpp
  ${SAMPLES_SHARED_CODE_DIR}/UTFConverter.cpp
  ${SAMPLES_SHARED_CODE_DIR}/UTFConverter.h
)

TARGET_USE_COMMON_OUTPUT_DIRECTORY(assimp_simpletexturedogl)

SET_PROPERTY(TARGET assimp_simpletexturedogl PROPERTY DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

TARGET_LINK_LIBRARIES( assimp_simpletexturedogl assimp ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} )

SET_TARGET_PROPERTIES( assimp_simpletexturedogl PROPERTIES
  OUTPUT_NAME assimp_simpletexturedogl
)

INSTALL( TARGETS assimp_simpletexturedogl
  DESTINATION "${ASSIMP_BIN_INSTALL_DIR}" COMPONENT assimp_dev
)

