
if (ANDROID)

	add_subdirectory(oboe-1.8.1 EXCLUDE_FROM_ALL)

	add_notices(oboe
			READABLE_NAME Oboe
			LICENSE_FILE oboe-1.8.1/LICENSE
			CREDITS "Audio-IO: Oboe - https://github.com/google/oboe")

endif()