#pragma once

#include "Generic/containers/compactcontainer.h"
#include "Generic/containers/freelistcontainer.h"
#include "Generic/offsetptr.h"
#include "Generic/replace.h"

#include "Meta/serialize/hierarchy/serializableunit.h"
#include "Meta/serialize/streams/pendingrequest.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

#include "entitycomponentcontainer.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct EntityComponentActionPayload {
            size_t mComponentIndex;
            OffsetPtr mOffset;
            const void *mComponent;
            void *mData;
        };

        struct MADGINE_SCENE_EXPORT EntityComponentBase {
            using Container = Containers::FreeListContainer<std::deque<Placeholder<0>>, EntityComponentFreeListConfig<Placeholder<0>>>;
        };

    }
}
}