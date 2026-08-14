#include "Meta/metalib.h"

#include "testunit.h"

#include "Generic/execution/algorithm.h"
#include "Generic/execution/execution.h"

#include "Meta/serialize/configs/creator.h"

#include "Meta/serialize/serializetable_impl.h"

SERIALIZETABLE_BEGIN(ComplexDataType)
SERIALIZETABLE_END(ComplexDataType)

constexpr const char *list1ArgNames(size_t index)
{
    constexpr std::array<const char *, 4> names = {
        "first",
        "second",
        "third",
        "fourth"
    };
    return names[index];
}

const char *writeList1(Engine::Serialize::FormattedSerializeStream &out, const ComplexDataType &data)
{
    out.beginExtendedWrite("Item", 4);
    write(out, data.i, "first");
    write(out, data.f, "second");
    write(out, data.s, "third");
    write(out, data.b, "fourth");
    return "Item";
}
Engine::Serialize::StreamResult readList1(Engine::Serialize::FormattedSerializeStream &in, int &i, float &f, std::string &s, bool &b)
{
    STREAM_PROPAGATE_ERROR(in.beginExtendedRead("Item", 4));
    STREAM_PROPAGATE_ERROR(read(in, i, "first"));
    STREAM_PROPAGATE_ERROR(read(in, f, "second"));
    STREAM_PROPAGATE_ERROR(read(in, s, "third"));
    STREAM_PROPAGATE_ERROR(read(in, b, "fourth"));
    return {};
}

SERIALIZETABLE_BEGIN(TestUnit)

    FIELD(list1)
    FIELD(list2)
    FIELD(set1)
    FIELD(set2)
    FIELD(map1, KeyValueCreator<DefaultCreator, DefaultCreator>)

    FIELD(complexList1, CustomCreator<&readList1, &writeList1>)
    FIELD(complexList2)
    FIELD(complexList3)

    FIELD(bytes)

    SYNCFUNCTION(foo)
    SYNCFUNCTION(bar)

SERIALIZETABLE_END(TestUnit)

Engine::Execution::Future<Engine::Serialize::MessageResult> TestUnit::call_void(int i)
{
    return TopLevelUnit<TestUnit>::call<&TestUnit::bar>(i);
}

Engine::Execution::Future<Engine::Serialize::MessageResult, int> TestUnit::call(int i)
{
    return TopLevelUnit<TestUnit>::call<&TestUnit::foo>(i);
}

Engine::Execution::Future<Engine::Serialize::MessageResult, int> TestUnit::query(int i)
{
    return TopLevelUnit<TestUnit>::query<&TestUnit::foo>(i);
}