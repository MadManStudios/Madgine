Project(Meta VERSION 1.0.0)

option(META_ENABLE_MESSAGE_LOGGING "Enables Logging functionality for sync messages")

set(sources)

FolderSources(sources "" "" metaforward.h metalib.h)
	FolderSources(sources "" base64 base64.cpp base64.h)
	FolderSources(sources "" inject)
	FolderSources(sources "" reflect accessor.h apifunction.cpp apifunction.h argumentlist.cpp argumentlist.h boundapifunction.cpp boundapifunction.h enum.cpp enum.h flags.cpp flags.h functiontable.cpp functiontable.h function.cpp result.cpp result.h range.h metatable.cpp metatable.h nativeobject.cpp nativeobject.h objectinstance.cpp objectinstance.h objectptr.cpp objectptr.h ownedvalue.cpp ownedvalue.h scopefield.cpp scopefield.h scopeiterator.cpp scopeiterator.h scopeptr.cpp scopeptr.h value.cpp value.h type.cpp type.h util.cpp util.h)
	FolderSources(sources "" reflectserialize valuetypeserialize.cpp valuetypeserialize.h)
	FolderSources(sources "" type storage.cpp storage.h storageops.cpp storageops.h storageops_forward.h storageops_impl.h storageops_primitives.cpp typenames.cpp typenames.h variantstorage.cpp)
	FolderSources(sources "" math atlas2.cpp atlas2.h boundingbox.h bounds.cpp bounds.h frustum.cpp frustum.h geometry2.cpp geometry2.h geometry3.cpp geometry3.h matrix4.cpp matrix4.h matrix3.cpp matrix3.h plane.h quaternion.cpp quaternion.h ray3.h transformation.cpp transformation.h vector4.h vector3.h vector2.h)
	FolderSources(sources "" serialize formats.cpp formats.h operations.cpp operations.h serializemanager.cpp serializemanager.h syncmanager.cpp syncmanager.h visitor.cpp visitor.h)
		FolderSources(sources serialize container serializable.h synced.h syncablecontainer.h serializablecontainer.h serialized.h syncable.cpp syncable.h)
		FolderSources(sources serialize formatter iniformatter.cpp iniformatter.h jsonformatter.cpp jsonformatter.h safebinaryformatter.cpp safebinaryformatter.h unsafebinaryformatter.cpp unsafebinaryformatter.h xmlformatter.cpp xmlformatter.h)
		FolderSources(sources serialize hierarchy serializableids.h serializableunit.cpp serializableunit.h serializableunitptr.cpp serializableunitptr.h syncableunit.cpp syncableunit.h toplevelunit.cpp toplevelunit.h serializetable.cpp serializetable.h)
		FolderSources(sources serialize streams buffered_streambuf.cpp buffered_streambuf.h formattedmessagestream.cpp formattedmessagestream.h formattedserializestream.cpp formattedserializestream.h formatter.cpp formatter.h messagelogger.cpp messagelogger.h message_streambuf.cpp message_streambuf.h readmessage.cpp readmessage.h serializestream.cpp serializestream.h serializestreamdata.cpp serializestreamdata.h serializablemapholder.cpp serializablemapholder.h streamresult.cpp streamresult.h syncstreamdata.cpp syncstreamdata.h writemessage.cpp writemessage.h)
	

add_workspace_library(Meta ${sources} SOURCE_ROOT .. PRECOMPILED_HEADER metalib.h)

configure_file(metaconfig.in.h metaconfig.h)
target_include_directories(Meta PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

target_link_libraries(Meta PUBLIC Generic)

if (MSVC)
	target_link_options(Meta PRIVATE "/NATVIS:${CMAKE_CURRENT_LIST_DIR}/Meta.natvis")
endif()

install_to_workspace(Runtime TARGETS Meta)
