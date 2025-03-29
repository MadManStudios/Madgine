include(../Util)

once()

macro(set_config out_name config var_name)
	if (${config}_General_${var_name})
		set(${out_name} ${${config}_General_${var_name}})
	endif()
endmacro(set_config)
