include_guard(GLOBAL)

cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0049 NEW)

function(FolderSources VARNAME PATHVAR FOLDER )
  
  set (PATH ${${PATHVAR}})
  string(REPLACE "/" \\ PATH_BACK "${PATH}")

  if("${FOLDER}" STREQUAL "")
	set (FULLFOLDER ${PATH})
  set (FULLFOLDER_BACK ${PATH_BACK})
  else()
	if("${PATH}" STREQUAL "")
	  set (FULLFOLDER ${FOLDER})
    set (FULLFOLDER_BACK ${FOLDER})
	else()
	  set (FULLFOLDER ${PATH}/${FOLDER})
    set (FULLFOLDER_BACK ${PATH_BACK}\\${FOLDER})
	endif()
  endif()

  if("${FULLFOLDER}" STREQUAL "")
    set (FULLPATH "")
    set (FULLPATH_BACK "")
  else()
	set (FULLPATH ${FULLFOLDER}/)
  set (FULLPATH_BACK ${FULLFOLDER_BACK}\\)  
  endif()
  
  set (_sources "")
  set (_sources_back "")

  foreach( src ${ARGN})
    set (_sources ${_sources} ${FULLPATH}${src})
    set (_sources_back ${_sources_back} ${FULLPATH_BACK}${src})
  endforeach() 

  set (${VARNAME} ${${VARNAME}} ${_sources} PARENT_SCOPE)

  SOURCE_GROUP ("${FULLFOLDER_BACK}" FILES ${_sources_back})
  
  if (NOT "${FOLDER}" STREQUAL "")
	set (${FOLDER} ${FULLFOLDER} PARENT_SCOPE)
  endif()

  
 
endfunction()