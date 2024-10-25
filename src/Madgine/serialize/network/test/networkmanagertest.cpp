#include <gtest/gtest.h>

#include "Madgine/serialize/network/networklib.h"

#include "Madgine/serialize/network/networkmanager.h"

#include "Meta/serialize/formats.h"

#include <future>

#include "../../../test/Meta/serialize/testManager.h"

using namespace Engine::Network;
using namespace Engine::Serialize;

TEST(NetworkManager, Connect)
{
    NetworkManager server("testNetworkServer");

    bool done = false;

    ASSERT_EQ(server.startServer(1234), NetworkManagerResult::SUCCESS) << "SocketAPI: " << server.getSocketAPIError();

#if !EMSCRIPTEN
    auto future = std::async(std::launch::async, [&]() {
        return server.acceptConnection(Formats::safebinary, 4s);
    });
    NetworkManager client("testNetworkClient");

    TestReceiver<NetworkManagerResult> receiver;
    Engine::Execution::detach(client.connect("127.0.0.1", 1234, Formats::safebinary, 4s) | Engine::Execution::then_receiver(receiver));
    EXPECT_EQ(future.get(), NetworkManagerResult::SUCCESS);
    server.sendMessages();
    client.receiveMessages(-1, 1s);
    ASSERT_TRUE(receiver.mFinished);    
    ASSERT_TRUE(receiver.mHasValue);
#endif
}
