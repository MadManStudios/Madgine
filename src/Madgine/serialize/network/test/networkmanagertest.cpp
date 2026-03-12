#include "Madgine/serialize/network/networklib.h"

#include "Madgine/serialize/network/networkmanager.h"

#include <future>
#include <gtest/gtest.h>

#include "Meta/serialize/formats.h"

#include "../../../test/Meta/serialize/testManager.h"

using namespace Engine::Network;
using namespace Engine::Serialize;

TEST(NetworkManager, Connect)
{
    NetworkManager server("testNetworkServer");

    ASSERT_EQ(server.startServer(1234), NetworkManagerResult::SUCCESS) << "SocketAPI: " << server.getSocketAPIError();

#if !EMSCRIPTEN
    auto future = std::async(std::launch::async, [&]() {
        return server.acceptConnection(Formats::safebinary, 4s);
    });
    NetworkManager client("testNetworkClient");

    
    Engine::Execution::Future<NetworkManagerResult> fut = client.connect("127.0.0.1", 1234, Formats::safebinary, 4s);
    EXPECT_EQ(future.get(), NetworkManagerResult::SUCCESS);
    server.sendMessages();
    client.receiveMessages(-1, 1s);
    ASSERT_TRUE(fut.is_value());
#endif
}
