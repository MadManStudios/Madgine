

Project(Widgets)     

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine widgetsforward.h widgetslib.h)
		FolderSources(sources Madgine widgets atlasloader.cpp atlasloader.h bar.cpp bar.h button.cpp button.h checkbox.cpp checkbox.h classid.h combobox.cpp combobox.h compoundwidget.cpp compoundwidget.h condition.cpp condition.h image.cpp image.h label.cpp label.h layout.cpp layout.h properties.cpp properties.h scenewindow.cpp scenewindow.h tabbar.cpp tabbar.h tablewidget.cpp tablewidget.h textbox.cpp textbox.h textedit.cpp textedit.h widget.cpp widget.h widgetloader.cpp widgetloader.h widgetmanager.cpp widgetmanager.h)
			FolderSources(sources widgets behavior tempwidget.cpp tempwidget.h widgetsenders.cpp widgetsenders.h)
			FolderSources(sources widgets shaders widgets.hlsl widgets.sl)
			FolderSources(sources widgets util colortintrenderdata.cpp multilinetextrenderdata.cpp renderdata.cpp scalableimagerenderdata.cpp tabbarrenderdata.cpp textrenderdata.cpp widgetsrenderdata.cpp)
				FolderSources(sources util layouts explicitlayoutrenderdata.cpp uniformlayoutrenderdata.cpp)

add_plugin(Widgets Madgine Misc ${sources} PRECOMPILED_HEADER src/Madgine/widgetslib.h)

target_link_plugins(Widgets Client Behavior)

target_include_directories(Widgets PRIVATE ${CMAKE_CURRENT_BINARY_DIR})


set(sources "")
FolderSources(sources "" handler)
	FolderSources(sources handler Madgine)
		FolderSources(sources Madgine widgets widgethandler.cpp widgethandler.h)

add_plugin(WidgetHandler Madgine Misc ${sources} SOURCE_ROOT handler)

target_link_plugins(WidgetHandler Widgets Handler)

set(sources "")
FolderSources(sources "" tools)
	FolderSources(sources tools Madgine_Tools)
		FolderSources(sources Madgine_Tools guieditor guieditor.cpp guieditor.h widgetsettings.cpp widgetsettings.h)

add_tools(Widgets ${sources} PRECOMPILED_HEADER tools/Madgine_Tools/widgetstoolslib.h)

target_link_plugins(WidgetsTools ClientTools BehaviorTools)

