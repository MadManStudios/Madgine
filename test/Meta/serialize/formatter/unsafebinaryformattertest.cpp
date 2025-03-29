#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/serialize/formats.h"

#include "formattertestbase.h"

using namespace Engine::Serialize;
using namespace std::chrono_literals;

TEST(Serialize_Formatter, UnsafeBinary)
{
    FormatterBaseTest(Formats::unsafebinary);
}
