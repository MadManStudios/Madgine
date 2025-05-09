#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/serialize/formats.h"

#include "formattertestbase.h"

#include "../testunit.h"
#include "../testManager.h"
#include "Meta/serialize/container/noparent.h"
#include "Meta/serialize/streams/formattedserializestream.h"
#include "Meta/serialize/operations.h"

using namespace Engine::Serialize;
using namespace std::chrono_literals;

TEST(Serialize_Formatter, JSON)
{
    FormatterBaseTest(Formats::json, R"("unit1" : {
    "list1" : [1,2,3],
    "list2" : [4,5],
    "set1" : [6,7,8,9],
    "set2" : [10],
    "map1" : [
        {
            "Key" : {
                "First" : 1,
                "Second" : 3
            },
            "Value" : 3
        },
        {
            "Key" : {
                "First" : 2,
                "Second" : 4
            },
            "Value" : 6
        }
    ],
    "complexList1" : [
        {
            "__extended" : {
                "first" : 2,
                "second" : 6,
                "third" : "set",
                "fourth" : true
            }
        },
        {
            "__extended" : {
                "first" : 2,
                "second" : 4,
                "third" : "default",
                "fourth" : true
            }
        }
    ],
    "complexList2" : [
    ],
    "complexList3" : [
    ],
    "bytes" : "SGVsbG8gV29ybGQh"
})");
}

TEST(Serialize_Formatter, JSON_InvalidParse)
{
    std::unique_ptr<std::stringbuf> file = std::make_unique<std::stringbuf>(
        R"("Item" : {
    "list1" : [1,2,3],
    "list2" : [4,5],
    "set1" : [6,,8,9],
    "set2" : [10],
    "map1" : [
        {
            "Key" : {
                "First" : 1,
                "Second" : 3
            },
            "Value" : 3
        },
        {
            "Key" : {
                "First" : 2,
                "Second" : 4
            },
            "Value" : 6
        }
    ],
    "complexList1" : [
        {
            "__extended" : {
                "first" : 2,
                "second" : 6,
                "third" : "set",
                "fourth" : true
            }
        },
        {
            "__extended" : {
                "first" : 2,
                "second" : 4,
                "third" : "default",
                "fourth" : true
            }
        }
    ],
    "complexList2" : [
    ],
    "complexList3" : [
    ],
    "bytes" : "SGVsbG8gV29ybGQh"
})");

    NoParent<TestUnit> unit;

    FormattedSerializeStream in { Formats::json(), SerializeStream { std::move(file) } };

    StreamResult result = read(in, unit, nullptr);
    ASSERT_EQ(result.mState, StreamState::PARSE_ERROR);
    ASSERT_TRUE(result.mError->mMsg == "ERROR: (4, 16): Expected: <int>" || result.mError->mMsg == "ERROR: (4, 16): Expected: <i>");
}

TEST(Serialize_Formatter, JSON_ExtendedOrder)
{
    std::unique_ptr<std::stringbuf> file = std::make_unique<std::stringbuf>(
        R"("Item" : {
    "list1" : [1,2,3],
    "list2" : [4,5],
    "set1" : [6,7,8,9],
    "set2" : [10],
    "map1" : [
        {
            "Key" : {
                "First" : 1,
                "Second" : 3
            },
            "Value" : 3
        },
        {
            "Key" : {
                "First" : 2,
                "Second" : 4
            },
            "Value" : 6
        }
    ],
    "complexList1" : [
        {
            "__extended" : {
                "second" : 6,
                "third" : "set",
                "first" : 2,
                "fourth" : true
            }
        },
        {
            "__extended" : {
                "second" : 4,
                "third" : "default",
                "fourth" : true,
                "first" : 2
            }
        }
    ],
    "complexList2" : [
    ],
    "complexList3" : [
    ],
    "bytes" : "SGVsbG8gV29ybGQh"
})");

    NoParent<TestUnit> unit;

    FormattedSerializeStream in { Formats::json(), SerializeStream { std::move(file) } };

    HANDLE_STREAM_RESULT(read(in, unit, nullptr));
    ASSERT_EQ(unit.complexList1.front().i, 2);
    ASSERT_EQ(unit.complexList1.front().f, 6.0f);
    ASSERT_EQ(unit.complexList1.front().s, "set");
    ASSERT_EQ(unit.complexList1.front().b, true);
}
