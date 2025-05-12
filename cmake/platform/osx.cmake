include(Util)

once()

include (Plugins)

if (OSX)

    macro(add_workspace_application target)

        add_executable(${target} ${ARGN})

        #set_target_properties(${target} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME ${target}
        #    MACOSX_BUNDLE_GUI_IDENTIFIER ${target}
        #    MACOSX_BUNDLE_BUNDLE_VERSION 1.0.0
        #    MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0.0)
        
    endmacro(add_workspace_application)

endif()
