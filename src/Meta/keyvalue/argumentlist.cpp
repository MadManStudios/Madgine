#include "../metalib.h"

#include "argumentlist.h"
#include "valuetype.h"

namespace Engine {

ArgumentList::ArgumentList() = default;

ArgumentList::ArgumentList(std::initializer_list<ValueType> vals)
    : mElements(vals)
{
}

ArgumentList::ArgumentList(std::vector<ValueType> vals)
    : mElements(std::move(vals))
{
}

ArgumentList::ArgumentList(size_t size)
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

const ValueType &ArgumentList::operator[](size_t i) const
{
    return mElements[i];
}

ValueType &ArgumentList::operator[](size_t i)
{
    return mElements[i];
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

void ArgumentList::push_back(ValueType &&v)
{
    mElements.push_back(std::move(v));
}

const ValueType &ArgumentList::at(size_t i) const
{
    return mElements.at(i);
}

void ArgumentList::insert(std::vector<ValueType>::const_iterator where, std::vector<ValueType>::const_iterator from, std::vector<ValueType>::const_iterator to)
{
    mElements.insert(where, from, to);
}

std::vector<ValueType>::iterator ArgumentList::begin()
{
    return mElements.begin();
}

std::vector<ValueType>::iterator ArgumentList::end()
{
    return mElements.end();
}

std::vector<ValueType>::const_iterator ArgumentList::begin() const
{
    return mElements.begin();
}

std::vector<ValueType>::const_iterator ArgumentList::end() const
{
    return mElements.end();
}

}