include_guard(GLOBAL)

include (Workspace)

if (WINDOWS)    

    macro(add_workspace_application target)

        add_executable(${target} WIN32 ${ARGN})
        
    endmacro(add_workspace_application)

endif()
