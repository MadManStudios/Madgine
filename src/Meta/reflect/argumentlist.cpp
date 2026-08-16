#include "../metalib.h"

#include "argumentlist.h"

#include "value.h"

namespace Engine {
namespace Reflect {

    ArgumentList::ArgumentList() = default;

    ArgumentList::ArgumentList(std::true_type, size_t size)
        : mElements(size)
    {
    }

    ArgumentList::ArgumentList(const ArgumentList &other) = default;

    ArgumentList::ArgumentList(ArgumentList &&) = default;

    ArgumentList::~ArgumentList() = default;

    ArgumentList &ArgumentList::operator=(const ArgumentList &) = default;
    ArgumentList &ArgumentList::operator=(ArgumentList &&) = default;

    void ArgumentList::clear()
    {
        mElements.clear();
    }

    const Value &ArgumentList::operator[](size_t i) const
    {
        return mElements[i];
    }

    Value &ArgumentList::operator[](size_t i)
    {
        return mElements[i];
    }

    Result ArgumentList::get(Value &retVal, size_t i) const
    {
        retVal = mElements[i];
        return {};
    }

    void ArgumentList::reserve(size_t size)
    {
        mElements.reserve(size);
    }

    void ArgumentList::resize(size_t size)
    {
        mElements.resize(size);
    }

    size_t ArgumentList::size() const
    {
        return mElements.size();
    }

    void ArgumentList::push_back(Value &&v)
    {
        mElements.push_back(std::move(v));
    }

    const Value &ArgumentList::at(size_t i) const
    {
        static Value sEmpty;
        if (i >= mElements.size())
            return sEmpty;
        return mElements.at(i);
    }

    void ArgumentList::insert(ArgumentList::const_iterator where, ArgumentList::const_iterator from, ArgumentList::const_iterator to)
    {
        mElements.insert(where, from, to);
    }

    ArgumentList::iterator ArgumentList::begin()
    {
        return mElements.begin();
    }

    ArgumentList::iterator ArgumentList::end()
    {
        return mElements.end();
    }

    ArgumentList::const_iterator ArgumentList::begin() const
    {
        return mElements.begin();
    }

    ArgumentList::const_iterator ArgumentList::end() const
    {
        return mElements.end();
    }

    std::ostream &operator<<(std::ostream &out, const ArgumentList &list)
    {
        out << "{ ";
        StringUtil::StreamJoiner join { out, ", " };
        for (const Value &v : list.mElements) {
            join.next() << v.toShortString();
        }
        return out << " }";
    }

}
}