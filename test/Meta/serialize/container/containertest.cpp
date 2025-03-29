#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/serialize/hierarchy/serializableunit.h"

#include "Meta/serialize/streams/serializestreamdata.h"

#include "Meta/serialize/container/noparent.h"

#include "../testManager.h"
#include "../testunit.h"

#include "Meta/serialize/operations.h"

#include "Meta/serialize/container/container_operations.h"

#include "Generic/execution/algorithm.h"
#include "Generic/execution/execution.h"

using namespace Engine::Serialize;
using namespace std::chrono_literals;

TEST(Serialize_Container, SyncedUnit)
{

    TestManager mgr1("container1");
    TestManager mgr2("container2");

    NoParent<TestUnit> unit1;
    NoParent<TestUnit> unit2;

    unit1.list1.emplace(unit1.list1.end(), 3);
    unit1.list1.emplace(unit1.list1.end(), 4);
    unit1.list1.emplace(unit1.list1.end(), 5);

    unit1.list2.emplace(unit1.list2.end(), 1);
    unit1.list2.emplace(unit1.list2.end(), 2);

    unit1.set1 = { 1, 2, 3 };
    unit2.set1 = { 4, 5, 6 };

    HANDLE_MGR_RECEIVER(mgr1.addTopLevelItemImpl(receiver, &unit1, 10));
    HANDLE_MGR_RECEIVER(mgr2.addTopLevelItemImpl(receiver, &unit2, 10));

    Buffer buffer;
    HANDLE_MGR_RESULT(mgr1, mgr1.setMasterBuffer(buffer));
    mgr1.sendMessages();
    HANDLE_MGR_RECEIVER(mgr2.setSlaveBuffer(receiver, buffer));


    ASSERT_EQ(unit1.list1, unit2.list1);
    ASSERT_EQ(unit1.list2, unit2.list2);
    ASSERT_EQ(unit1.set1, unit2.set1);
    
    GenericTestReceiver calledFuture;
    Engine::Execution::detach(Engine::Execution::then_receiver(unit1.list2.emplace_async(unit1.list2.end(), 6), calledFuture));
    ASSERT_TRUE(calledFuture.is_ready());
    ASSERT_EQ(unit1.list2.back(), 6);

    mgr1.sendMessages();
    mgr2.receiveMessages(1, 0ms);

    ASSERT_EQ(unit1.list2, unit2.list2);

    calledFuture.reset();
    Engine::Execution::detach(Engine::Execution::then_receiver(unit2.list2.emplace_async(unit2.list2.end(), 7), calledFuture));

    ASSERT_EQ(unit1.list2, unit2.list2);

    mgr2.sendMessages();
    mgr1.receiveMessages(1, 0ms);

    ASSERT_EQ(unit1.list2.back(), 7);

    mgr1.sendMessages();
    ASSERT_FALSE(calledFuture.is_ready());
    mgr2.receiveMessages(1, 0ms);
    ASSERT_TRUE(calledFuture.is_ready());

    ASSERT_EQ(unit1.list2, unit2.list2);

    calledFuture.reset();
    Engine::Execution::detach(Engine::Execution::then_receiver(unit1.list2.erase_async(std::next(unit1.list2.begin())), calledFuture));
    ASSERT_TRUE(calledFuture.is_ready());

    ASSERT_EQ(unit1.list2.size(), 3);

    mgr1.sendMessages();
    mgr2.receiveMessages(1, 0ms);

    ASSERT_EQ(unit2.list2.size(), 3);
    ASSERT_EQ(unit1.list2, unit2.list2);
}

TEST(Serialize_Container, Array)
{

    //Engine::Threading::WorkGroup wg;

    std::array<uint32_t, 128> array;
    for (size_t i = 0; i < 128; ++i)
        array[i] = 2 * i;

    Buffer buffer;
    FormattedSerializeStream stream1 {
        Formats::safebinary(), SerializeStream { std::make_unique<TestBuf>(buffer, true) }
    };
    FormattedSerializeStream stream2 {
        Formats::safebinary(), SerializeStream { std::make_unique<TestBuf>(buffer, false) }
    };

    write(stream1, array, "array");

    std::array<uint32_t, 128> array2;
    HANDLE_STREAM_RESULT(read(stream2, array2, nullptr));

    ASSERT_EQ(array, array2);
}
