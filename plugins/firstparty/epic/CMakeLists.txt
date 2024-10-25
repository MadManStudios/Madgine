Project(EpicServices)

add_subdirectory(shared)

if (NOT TARGET EOS)
	return()
endif()

set(EOS_LOGIN_METHOD "CREDENTIALS" CACHE STRING "")

set_property(CACHE EOS_LOGIN_METHOD PROPERTY STRINGS EXCHANGE_CODE EXTERNAL_AUTH DEV_AUTH_TOOL CREDENTIALS)

macro(abortEpic)

	MESSAGE(STATUS "Aborting Epic configuration.")
	MESSAGE(STATUS "--- Epic End ---")
	return()

endmacro(abortEpic)

MESSAGE(STATUS "--- Epic ---")

if (NOT EOS_LOGIN_METHOD)
	MESSAGE(WARNING "No EOS_LOGIN_METHOD provided.")
	abortEpic()
endif ()

if (NOT EOS_ENCYPTION_KEY)
	MESSAGE(WARNING "No EOS_ENCYPTION_KEY provided. Falling back to default.")
	abortEpic()
endif ()

if (NOT EOS_PRODUCT_ID)
	MESSAGE(WARNING "No EOS_PRODUCT_ID provided. Falling back to default.")
	abortEpic()
endif ()

if (NOT EOS_SANDBOX_ID)
	MESSAGE(WARNING "No EOS_SANDBOX_ID provided. Falling back to default.")
	abortEpic()
endif ()

if (NOT EOS_DEPLOYMENT_ID)
	MESSAGE(WARNING "No EOS_DEPLOYMENT_ID provided. Falling back to default.")
	abortEpic()
endif ()

if (NOT EOS_CLIENT_CREDENTIALS_ID)
	MESSAGE(WARNING "No EOS_CLIENT_CREDENTIALS_ID provided. Falling back to default.")
	abortEpic()
endif ()

if (NOT EOS_CLIENT_CREDENTIALS_SECRET)
	MESSAGE(WARNING "No EOS_CLIENT_CREDENTIALS_SECRET provided. Falling back to default.")
	abortEpic()
endif ()


MESSAGE(STATUS "Login Method: ${EOS_LOGIN_METHOD}")
MESSAGE(STATUS "Encryption Key: <redacted>")
MESSAGE(STATUS "Product Id: <redacted>")
MESSAGE(STATUS "Sandbox Id: <redacted>")
MESSAGE(STATUS "Deployment Id: <redacted>")
MESSAGE(STATUS "Client Credentials Id: <redacted>")
MESSAGE(STATUS "Client Credentials Secret: <redacted>")

MESSAGE(STATUS "--- Epic End ---")


set(sources "")
FolderSources(sources "" src)
FolderSources(sources src Madgine)
	FolderSources(sources Madgine firstparty epicservices.cpp epicservices.h)

add_plugin(EpicServices Madgine FirstParty ${sources} PRECOMPILED_HEADER src/Madgine/epicserviceslib.h)

target_link_plugins(EpicServices FirstParty)

target_link_libraries(EpicServices PRIVATE EOS)

target_compile_definitions(EpicServices 
	PRIVATE EOS_LOGIN_METHOD_${EOS_LOGIN_METHOD}
	PRIVATE EOS_ENCYPTION_KEY="${EOS_ENCYPTION_KEY}"
	PRIVATE EOS_PRODUCT_ID="${EOS_PRODUCT_ID}"
	PRIVATE EOS_SANDBOX_ID="${EOS_SANDBOX_ID}"
	PRIVATE EOS_DEPLOYMENT_ID="${EOS_DEPLOYMENT_ID}"
	PRIVATE EOS_CLIENT_CREDENTIALS_ID="${EOS_CLIENT_CREDENTIALS_ID}"
	PRIVATE EOS_CLIENT_CREDENTIALS_SECRET="${EOS_CLIENT_CREDENTIALS_SECRET}"
	)