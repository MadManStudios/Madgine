#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/serialize/hierarchy/serializableunit.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../testManager.h"

using namespace Engine::Serialize;
using namespace std::chrono_literals;

struct S : SerializableUnitBase {
    int j;
};

SERIALIZETABLE_BEGIN(S)
FIELD(j)
SERIALIZETABLE_END(S)

struct TestStruct : SerializableUnitBase {

    int i;
    S s;
};

SERIALIZETABLE_BEGIN(TestStruct)
FIELD(i)
FIELD(s)
SERIALIZETABLE_END(TestStruct)

TEST(Serialize_Table, Test1)
{

    TestManager mgr1("table1");
    TestManager mgr2("table2");


    Buffer buffer;
    HANDLE_MGR_RESULT(mgr1, mgr1.setMasterBuffer(buffer));
    FormattedBufferedStream &stream1 = mgr1.getMasterStream(1);
    mgr1.sendMessages();
    HANDLE_MGR_RECEIVER(mgr2.setSlaveBuffer(receiver, buffer));
    FormattedBufferedStream &stream2 = *mgr2.getSlaveStream();

    TestStruct t1;
    t1.i = 1;
    t1.s.j = 2;

    stream1.beginMessageWrite();
    write(stream1, t1, "Test");    
    stream1.endMessageWrite();
    stream1.sendMessages();

    TestStruct t2;

    FormattedBufferedStream::MessageReadMarker msg;
    HANDLE_STREAM_RESULT(stream2.beginMessageRead(msg));
    ASSERT_TRUE(msg);
    HANDLE_STREAM_RESULT(read(stream2, t2, "Test"));
    HANDLE_STREAM_RESULT(msg.end());

    ASSERT_EQ(t1.i, t2.i);
}
