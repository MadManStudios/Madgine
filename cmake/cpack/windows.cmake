
set(CPACK_GENERATOR NSIS)

set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME}${MADGINE_CONFIGURATION_SUFFIX}-${PROJECT_VERSION})	
set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})

set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH ON)

find_package(WindowsSDK)
if (WINDOWSSDK_FOUND)
	find_program(signtool signtool.exe PATHS ${WINDOWSSDK_LATEST_DIR})
	if (signtool)
		set(CPACK_NSIS_FINALIZE_CMD "\\\"${signtool}\\\" sign /fd sha256 /tr http://ts.ssl.com /td sha256 /d \\\"${PROJECT_NAME}\\\" /a %1") 
		set(CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS "\n\n\
;--------------------------------\n\
;Signing\n\
\n\
  !finalize '${CPACK_NSIS_FINALIZE_CMD}'\n\
  !uninstfinalize '${CPACK_NSIS_FINALIZE_CMD}'\n\n\
")

		
	endif()
endif()		