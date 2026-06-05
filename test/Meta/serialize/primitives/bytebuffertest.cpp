#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/base64/base64.h"

#include "Generic/bytebuffer.h"

using namespace Engine::Serialize;
using namespace std::chrono_literals;

TEST(Serialize_Primitives, Base64)
{
    const char *s = "Hello World!";
    Engine::Memory::ByteBuffer buffer = { s, strlen(s) };
    std::string encoded = Engine::Base64::encode(buffer);
    ASSERT_EQ(encoded, "SGVsbG8gV29ybGQh");
    Engine::Memory::ByteBuffer decoded;
    ASSERT_TRUE(Engine::Base64::decode(decoded, encoded));
    ASSERT_EQ(buffer, decoded);
}
