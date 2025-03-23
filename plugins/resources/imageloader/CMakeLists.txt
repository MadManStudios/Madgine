
Project(ImageLoader)

set(sources "")
FolderSources(sources "" Madgine imageloaderlib.h)
	FolderSources(sources Madgine imageloader imagedata.h imageloader.cpp imageloader.h)


add_plugin(ImageLoader Madgine Utility ${sources} PRECOMPILED_HEADER Madgine/imageloaderlib.h SOURCE_ROOT .)

target_link_plugins(ImageLoader Resources)
