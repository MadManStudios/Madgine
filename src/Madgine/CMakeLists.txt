add_subdirectory(codegen)
set_target_properties(CodeGen PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_subdirectory(render)

add_subdirectory(serialize)

add_subdirectory(cli)