#include "../clientlib.h"

#include "layoutloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/serialize/filesystem/filemanager.h"

#include "Meta/serialize/formats.h"

#include "Meta/serialize/streams/formattedserializestream.h"

#include "Meta/serialize/operations.h"

#include "mainwindow.h"

INSTANCELOADER(Engine::Window::LayoutLoader)

namespace Engine {
namespace Window {

    LayoutLoader::LayoutLoader()
        : InstanceLoader({ ".layout" })
    {
    }

    Threading::Task<bool> LayoutLoader::loadImpl(MainWindow &window, Resource *res)
    {
        Serialize::SerializeManager mgr { "Layout" };
        Serialize::FormattedSerializeStream file = Serialize::FormattedSerializeStream { Serialize::Formats::xml(), mgr.wrapStream(res->readAsStream(), true) };

        if (file) {
            Serialize::StreamResult result = Serialize::readState(file, window, nullptr);
            if (result.mState != Serialize::StreamState::OK) {
                LOG_ERROR("Failed loading '" << res->path() << "' with following Error: "
                                             << "\n"
                                             << result);
                co_return false;
            }
            co_return true;
        } else {
            LOG_ERROR("Failed to open " << res->path() << "!");
            co_return false;
        }
    }

    Threading::Task<void> LayoutLoader::unloadImpl(MainWindow &window)
    {
        co_return;
    }

    Threading::Task<Resources::BakeResult> LayoutLoader::bakeResources(std::vector<Filesystem::Path> &resourcesToBake, const Filesystem::Path &intermediateDir)
    {
        co_return Resources::BakeResult::NOTHING_TO_DO;
    }

}
}
