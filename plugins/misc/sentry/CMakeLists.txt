

add_subdirectory(shared)

PROJECT(Sentry)


set(sources)
FolderSources(sources "" src)
	FolderSources(sources src Madgine sentry.cpp sentry.h sentryforward.h sentrylib.h)
	
add_plugin(Sentry Madgine Misc ${sources} PRECOMPILED_HEADER src/Madgine/sentrylib.h)

target_link_plugins(Sentry Root)

target_link_libraries(Sentry PRIVATE sentry::sentry)
