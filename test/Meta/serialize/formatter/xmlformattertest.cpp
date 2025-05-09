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

TEST(Serialize_Formatter, XML)
{
    FormatterBaseTest(Formats::xml, R"(<unit1>
    <list1>
        <Item>1</Item>
        <Item>2</Item>
        <Item>3</Item>
    </list1>
    <list2>
        <Item>4</Item>
        <Item>5</Item>
    </list2>
    <set1>
        <Item>6</Item>
        <Item>7</Item>
        <Item>8</Item>
        <Item>9</Item>
    </set1>
    <set2>
        <Item>10</Item>
    </set2>
    <map1>
        <Item>
            <Key>
                <First>1</First>
                <Second>3</Second>
            </Key>
            <Value>3</Value>
        </Item>
        <Item>
            <Key>
                <First>2</First>
                <Second>4</Second>
            </Key>
            <Value>6</Value>
        </Item>
    </map1>
    <complexList1>
        <Item first=2 second=6 third="set" fourth=true>
        </Item>
        <Item first=2 second=4 third="default" fourth=true>
        </Item>
    </complexList1>
    <complexList2>
    </complexList2>
    <complexList3>
    </complexList3>
    <bytes>SGVsbG8gV29ybGQh</bytes>
</unit1>
)");
}

TEST(Serialize_Formatter, XML_InvalidParse)
{
    std::unique_ptr<std::stringbuf> file = std::make_unique<std::stringbuf>(
        R"(<unit1>
    <list1>
        <Item>1</Item>
        <Item></Item> )" /* <--- Error here */ R"(
        <Item>3</Item>
    </list1>
    <list2>
        <Item>4</Item>
        <Item>5</Item>
    </list2>
    <set1>
        <Item>6</Item>
        <Item>7</Item>
        <Item>8</Item>
        <Item>9</Item>
    </set1>
    <set2>
        <Item>10</Item>
    </set2>
    <map1>
        <Item>
            <Key>
                <First>1</First>
                <Second>3</Second>
            </Key>
            <Value>3</Value>
        </Item>
        <Item>
            <Key>
                <First>2</First>
                <Second>4</Second>
            </Key>
            <Value>6</Value>
        </Item>
    </map1>
    <complexList1>
        <Item first=2 second=6 third="set">
        </Item>
        <Item first=2 second=4 third="default">
        </Item>
    </complexList1>
    <complexList2>
    </complexList2>
    <complexList3>
    </complexList3>
</unit1>")");

    NoParent<TestUnit> unit;

    FormattedSerializeStream in { Formats::xml(), SerializeStream { std::move(file) } };

    StreamResult result = read(in, unit, nullptr);
    ASSERT_EQ(result.mState, StreamState::PARSE_ERROR);
    ASSERT_TRUE(result.mError->mMsg == "ERROR: (4, 14): Expected: <int>" || result.mError->mMsg == "ERROR: (4, 14): Expected: <i>");
}

TEST(Serialize_Formatter, XML_ExtendedOrder)
{
    std::unique_ptr<std::stringbuf> file = std::make_unique<std::stringbuf>(
        R"(<unit1>
    <list1>
        <Item>1</Item>
        <Item>2</Item>
        <Item>3</Item>
    </list1>
    <list2>
        <Item>4</Item>
        <Item>5</Item>
    </list2>
    <set1>
        <Item>6</Item>
        <Item>7</Item>
        <Item>8</Item>
        <Item>9</Item>
    </set1>
    <set2>
        <Item>10</Item>
    </set2>
    <map1>
        <Item>
            <Key>
                <First>1</First>
                <Second>3</Second>
            </Key>
            <Value>3</Value>
        </Item>
        <Item>
            <Key>
                <First>2</First>
                <Second>4</Second>
            </Key>
            <Value>6</Value>
        </Item>
    </map1>
    <complexList1>
        <Item second=6 first=2 third="set" fourth=true>
        </Item>
        <Item first=2 second=4 third="default" fourth=true>
        </Item>
    </complexList1>
    <complexList2>
    </complexList2>
    <complexList3>
    </complexList3>
    <bytes>SGVsbG8gV29ybGQh</bytes>
</unit1>)");

    NoParent<TestUnit> unit;

    FormattedSerializeStream in { Formats::xml(), SerializeStream { std::move(file) } };

    HANDLE_STREAM_RESULT(read(in, unit, nullptr));
    ASSERT_EQ(unit.complexList1.front().i, 2);
    ASSERT_EQ(unit.complexList1.front().f, 6.0f);
    ASSERT_EQ(unit.complexList1.front().s, "set");
    ASSERT_EQ(unit.complexList1.front().b, true);
}
