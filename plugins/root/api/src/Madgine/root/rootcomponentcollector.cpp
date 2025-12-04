#include "../rootlib.h"

#include "rootcomponentcollector.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "root.h"

DEFINE_UNIQUE_COMPONENT(Engine::Root, RootComponent)

namespace Engine {
namespace Root {

    RootComponentBase &getRootComponent(size_t i)
    {
        return Root::getSingleton().getComponent(i);
    }

}
}
