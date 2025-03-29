include(Util)

once()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)

macro(compile_shaders target)

    get_property(sources TARGET ${target} PROPERTY SOURCES)
    
    add_dependencies(${target} ShaderGen)

    foreach (source ${sources})
        set (compile FALSE)
        
        get_filename_component(ext ${source} EXT)

        if (ext STREQUAL ".PS_hlsl")
            set(profile ps_6_2)
            set(extension ".PS_spirv")
            set(compile TRUE)
        endif()
        if (ext STREQUAL ".VS_hlsl")
            set(profile vs_6_2)
            set(extension ".VS_spirv")
            set(compile TRUE)            
        endif()
        if (ext STREQUAL ".hlsl")            
            set_source_files_properties(${source} PROPERTIES VS_TOOL_OVERRIDE "None")
        endif()
        if (compile)
            get_filename_component(name ${source} NAME_WE)
            add_custom_command(OUTPUT spirv/${name}${extension}
                COMMAND ${CMAKE_COMMAND}
                    -E make_directory 
                    spirv
                COMMAND $<TARGET_FILE:ShaderGen>
                    ${CMAKE_CURRENT_SOURCE_DIR}/${source}       
                    spirv/${name}${extension}
                    ${CMAKE_BINARY_DIR}/data
                    $<IF:$<CONFIG:DEBUG>,-g,>
                    $<TARGET_PROPERTY:ShaderGen,ShaderGenTargets>
                    -I "$<JOIN:$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>,;-I;>"
                COMMAND_EXPAND_LISTS
                MAIN_DEPENDENCY ${source}
                DEPENDS ShaderGen
                IMPLICIT_DEPENDS C ${source}
                COMMENT "Transpiling shader: ${name}${ext}"
                VERBATIM)
            target_sources(${target} PRIVATE spirv/${name}${extension})
        endif()

    endforeach()

endmacro(compile_shaders)