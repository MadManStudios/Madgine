#include "../metalib.h"

#include "scopeptr.h"

#include "metatable.h"
#include "scopefield.h"
#include "scopeiterator.h"
#include "virtualscope.h" //enforce export of VirtualScopeBase<void>

namespace Engine {
namespace Reflect {

    ScopeField ScopePtr::operator[](std::string_view key) const
    {
        return *find(key);
    }

    /*bool ScopePtr::isEditable(const std::string &key) const
    {
        return (*find(key)).isEditable();
    }*/

    ScopeIterator ScopePtr::find(std::string_view key) const
    {
        return mType->find(key, Value { *this });
    }

    ScopeIterator ScopePtr::begin() const
    {
        return { Value { *this }, mType ? mType->mMembers : nullptr };
    }

    ScopeIterator ScopePtr::end() const
    {
        return { Value { *this }, nullptr };
    }

    std::string ScopePtr::name() const
    {
        if (mScope)
            return mType->name(Value { *this });
        else
            return "<NULL>";
    }

    void ScopePtr::moveAssign(ScopePtr other) const
    {
        mType->moveAssign(*this, other);
    }

    Result ScopePtr::call(Value &retVal, const ArgumentList &args) const
    {
        return mType->call(Value { *this }, retVal, args);
    }

}
}