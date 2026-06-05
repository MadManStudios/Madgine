#include "../rootlib.h"

#include "rootcomponentbase.h"

#include "Modules/threading/task.h"

#include "Meta/reflect/metatable_impl.h"

METATABLE_BEGIN(Engine::Core::RootComponentBase)
METATABLE_END(Engine::Core::RootComponentBase)

namespace Engine {
namespace Core {
    RootComponentBase::RootComponentBase(Root &root)
        : mRoot(root)
    {
    }

    Threading::Task<int> RootComponentBase::runTools()
    {
        co_return 0;
    }

}
}
