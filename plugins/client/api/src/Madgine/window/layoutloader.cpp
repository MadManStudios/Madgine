#include "../clientlib.h"

#include "layoutloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/serialize/filesystem/filemanager.h"

#include "Meta/serialize/formats.h"

#include "Meta/serialize/streams/formattedserializestream.h"

#include "Meta/serialize/operations.h"

#include "mainwindow.h"

RESOURCELOADER(Engine::Window::LayoutLoader)

namespace Engine {
namespace Window {

    LayoutLoader::LayoutLoader()
        : ResourceLoader({ ".layout" })
    {
    }

    Threading::Task<bool> LayoutLoader::loadImpl(LayoutDummy &data, ResourceDataInfo &info)
    {
        throw 0;
    }

    Threading::Task<void> LayoutLoader::unloadImpl(LayoutDummy &data)
    {
        throw 0;
    }

    Threading::Task<Resources::BakeResult> LayoutLoader::bakeResources(std::vector<Filesystem::Path> &resourcesToBake, const Filesystem::Path &intermediateDir)
    {
        co_return Resources::BakeResult::NOTHING_TO_DO;
    }

    Serialize::FormattedSerializeStream LayoutLoader::Interface::Resource::readAsFormattedStream(Serialize::SerializeManager &mgr)
    {
        return Serialize::FormattedSerializeStream { Serialize::Formats::xml(), mgr.wrapStream(readAsStream(), true) };
    }

}
}
