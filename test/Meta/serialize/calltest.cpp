#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/serialize/hierarchy/serializableunit.h"

#include "Meta/serialize/container/noparent.h"

#include "testManager.h"
#include "testunit.h"

using namespace Engine::Serialize;
using namespace std::chrono_literals;

TEST(Serialize_Call, Call)
{

    TestManager mgr1("container1");
    TestManager mgr2("container2");

    NoParent<TestUnit> unit1;
    NoParent<TestUnit> unit2;

    HANDLE_MGR_RECEIVER(mgr1.addTopLevelItemImpl(receiver, &unit1, 10));
    HANDLE_MGR_RECEIVER(mgr2.addTopLevelItemImpl(receiver, &unit2, 10));

    Buffer buffer;
    HANDLE_MGR_RESULT(mgr1, mgr1.setMasterBuffer(buffer));
    mgr1.sendMessages();
    HANDLE_MGR_RECEIVER(mgr2.setSlaveBuffer(receiver, buffer));

    ASSERT_EQ(unit1.mCallCount, 0);
    ASSERT_EQ(unit2.mCallCount, 0);

    TestReceiver<Engine::Serialize::MessageResult, int> f1;
    unit1.call(1, f1);

    ASSERT_TRUE(f1.is_ready());
    ASSERT_MESSAGEFUTURE_EQ(f1, 2);

    ASSERT_EQ(unit1.mCallCount, 1);
    ASSERT_EQ(unit2.mCallCount, 0);

    mgr1.sendMessages();
    mgr2.receiveMessages(1, 1000ms);

    ASSERT_EQ(unit1.mCallCount, 1);
    ASSERT_EQ(unit2.mCallCount, 1);

    TestReceiver<Engine::Serialize::MessageResult, int> f2;
    unit2.call(2, f2);

    ASSERT_FALSE(f2.is_ready());

    ASSERT_EQ(unit1.mCallCount, 1);
    ASSERT_EQ(unit2.mCallCount, 1);

    mgr2.sendMessages();
    mgr1.receiveMessages(1, 1000ms);

    ASSERT_FALSE(f2.is_ready());

    ASSERT_EQ(unit1.mCallCount, 2);
    ASSERT_EQ(unit2.mCallCount, 1);

    mgr1.sendMessages();
    mgr2.receiveMessages(1, 1000ms);

    ASSERT_TRUE(f2.is_ready());
    ASSERT_MESSAGEFUTURE_EQ(f2, 3);

    ASSERT_EQ(unit1.mCallCount, 2);
    ASSERT_EQ(unit2.mCallCount, 2);

    TestReceiver<Engine::Serialize::MessageResult> f3;
    unit1.call_void(4, f3);

    ASSERT_TRUE(f3.is_ready());
    ASSERT_TRUE(f3.mHasValue);

    ASSERT_EQ(unit1.mCallCount, 3);
    ASSERT_EQ(unit2.mCallCount, 2);

    mgr1.sendMessages();
    mgr2.receiveMessages(1, 1000ms);

    ASSERT_EQ(unit1.mCallCount, 3);
    ASSERT_EQ(unit2.mCallCount, 3);
}

TEST(Serialize_Call, Call_Hierarchical)
{

    TestManager mgr1("container1");
    TestManager mgr2("container2");
    TestManager mgr3("container3");

    NoParent<TestUnit> unit1;
    NoParent<TestUnit> unit2;
    NoParent<TestUnit> unit3;

    HANDLE_MGR_RECEIVER(mgr1.addTopLevelItemImpl(receiver, &unit1, 10));
    HANDLE_MGR_RECEIVER(mgr2.addTopLevelItemImpl(receiver, &unit2, 10));
    HANDLE_MGR_RECEIVER(mgr3.addTopLevelItemImpl(receiver, &unit3, 10));

    Buffer buffer;
    HANDLE_MGR_RESULT(mgr1, mgr1.setMasterBuffer(buffer));
    mgr1.sendMessages();
    HANDLE_MGR_RECEIVER(mgr2.setSlaveBuffer(receiver, buffer));

    Buffer buffer2;
    HANDLE_MGR_RESULT(mgr2, mgr2.setMasterBuffer(buffer2));
    mgr2.sendMessages();
    HANDLE_MGR_RECEIVER(mgr3.setSlaveBuffer(receiver, buffer2));

    ASSERT_EQ(unit1.mCallCount, 0);
    ASSERT_EQ(unit2.mCallCount, 0);
    ASSERT_EQ(unit3.mCallCount, 0);

    TestReceiver<Engine::Serialize::MessageResult, int> f1;
    unit1.call(1, f1);

    ASSERT_TRUE(f1.is_ready());
    ASSERT_MESSAGEFUTURE_EQ(f1, 2);

    ASSERT_EQ(unit1.mCallCount, 1);
    ASSERT_EQ(unit2.mCallCount, 0);
    ASSERT_EQ(unit3.mCallCount, 0);

    mgr1.sendMessages();
    mgr2.receiveMessages(1, 0ms);

    ASSERT_EQ(unit1.mCallCount, 1);
    ASSERT_EQ(unit2.mCallCount, 1);
    ASSERT_EQ(unit3.mCallCount, 0);

    mgr2.sendMessages();
    mgr3.receiveMessages(1, 0ms);

    ASSERT_EQ(unit1.mCallCount, 1);
    ASSERT_EQ(unit2.mCallCount, 1);
    ASSERT_EQ(unit3.mCallCount, 1);

    TestReceiver<Engine::Serialize::MessageResult, int> f2;
    unit3.call(2, f2);

    ASSERT_FALSE(f2.is_ready());

    ASSERT_EQ(unit1.mCallCount, 1);
    ASSERT_EQ(unit2.mCallCount, 1);
    ASSERT_EQ(unit3.mCallCount, 1);

    mgr3.sendMessages();
    mgr2.receiveMessages(1, 0ms);

    ASSERT_FALSE(f2.is_ready());

    ASSERT_EQ(unit1.mCallCount, 1);
    ASSERT_EQ(unit2.mCallCount, 1);
    ASSERT_EQ(unit3.mCallCount, 1);

    mgr2.sendMessages();
    mgr1.receiveMessages(1, 0ms);

    ASSERT_FALSE(f2.is_ready());

    ASSERT_EQ(unit1.mCallCount, 2);
    ASSERT_EQ(unit2.mCallCount, 1);
    ASSERT_EQ(unit3.mCallCount, 1);

    mgr1.sendMessages();
    mgr2.receiveMessages(1, 0ms);

    ASSERT_FALSE(f2.is_ready());

    ASSERT_EQ(unit1.mCallCount, 2);
    ASSERT_EQ(unit2.mCallCount, 2);
    ASSERT_EQ(unit3.mCallCount, 1);

    mgr2.sendMessages();
    mgr3.receiveMessages(1, 0ms);

    ASSERT_TRUE(f2.is_ready());
    ASSERT_MESSAGEFUTURE_EQ(f2, 3);

    ASSERT_EQ(unit1.mCallCount, 2);
    ASSERT_EQ(unit2.mCallCount, 2);
    ASSERT_EQ(unit3.mCallCount, 2);
}
