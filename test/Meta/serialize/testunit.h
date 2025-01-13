#pragma once

#include "Meta/serialize/hierarchy/toplevelunit.h"

#include "Meta/serialize/container/syncablecontainer.h"

#include "Generic/bytebuffer.h"

#include "../../Generic/testreceiver.h"

using namespace Engine::Serialize;

struct ComplexDataType {

    ComplexDataType(int i = 2, float f = 4.0f, std::string s = "default", bool b = true)
        : i(i)
        , f(f)
        , s(std::move(s))
        , b(b)
    {
    }

    bool operator==(const ComplexDataType &other) const
    {
        return i == other.i && f == other.f && s == other.s && b == other.b;
    }

    int i;
    float f;
    std::string s;
    bool b;
};

struct TestUnit : TopLevelUnit<TestUnit> {
    SERIALIZABLEUNIT(TestUnit)

    TestUnit(int data = 1)
        : TopLevelUnit<TestUnit>(10)
        , mData(data)
    {
    }

    int foo(int i)
    {
        ++mCallCount;
        return i + 1;
    }

    void bar(int i)
    {
        ++mCallCount;
    }

    bool operator==(const TestUnit &other) const
    {
        return list1 == other.list1 && set1 == other.set1 && list2 == other.list2 && set2 == other.set2 && complexList1 == other.complexList1 && complexList2 == other.complexList2 && complexList3 == other.complexList3;
    }

    int mData;
    int mCallCount = 0;

    void call_void(int i, TestReceiver<Engine::Serialize::MessageResult> &rec);
    void call(int i, TestReceiver<Engine::Serialize::MessageResult, int> &rec);
    void query(int i, TestReceiver<Engine::Serialize::MessageResult, int> &rec);

    SERIALIZABLE_CONTAINER(list1, std::list<int>);
    SERIALIZABLE_CONTAINER(set1, std::set<int>);
    SYNCABLE_CONTAINER(list2, std::list<int>);
    SYNCABLE_CONTAINER(set2, std::set<int>);
    SERIALIZABLE_CONTAINER(map1, std::map<std::pair<int, float>, int>);

    SERIALIZABLE_CONTAINER(complexList1, std::list<ComplexDataType>);
    SERIALIZABLE_CONTAINER(complexList2, std::list<ComplexDataType>);
    SERIALIZABLE_CONTAINER(complexList3, std::list<ComplexDataType>);

    Engine::ByteBuffer bytes;
};