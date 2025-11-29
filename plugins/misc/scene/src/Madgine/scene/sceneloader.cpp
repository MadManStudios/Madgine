#include "../scenelib.h"

#include "sceneloader.h"


#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Meta/serialize/serializemanager.h"
#include "Meta/serialize/formats.h"

#include "scenecontainer.h"

INSTANCELOADER(Engine::Scene::SceneLoader)

namespace Engine {
namespace Scene {

    SceneLoader::SceneLoader()
        : InstanceLoader({ ".scene" }, { .mIconName = "SceneIcon.png" })
    {
    }

    Threading::Task<bool> SceneLoader::loadImpl(SceneContainer &container, Resource *res)
    {
        Serialize::SerializeManager serializeMgr { "SceneLoader" };
        Serialize::FormattedSerializeStream stream { Serialize::Formats::xml(), serializeMgr.wrapStream(res->readAsStream(), true) };

        Serialize::StreamResult result = Serialize::readState(stream, container, "Container");
        if (result.mState != Serialize::StreamState::OK) {
            LOG_ERROR(result);
            co_return false;
        }

        co_return true;
    }
}
}
