#include "Meta/metalib.h"

#include <gtest/gtest.h>

#include "formattertestbase.h"

#include "../testManager.h"
#include "../testunit.h"

#include "Meta/serialize/streams/serializestream.h"

#include "Meta/serialize/container/noparent.h"

#include "Meta/serialize/streams/serializestreamdata.h"

#include "Meta/serialize/operations.h"

#include <iostream>

void FormatterBaseTest(Format format, std::string_view expected)
{
    NoParent<TestUnit> unit1;
    NoParent<TestUnit> unit2;

    unit1.list1 = { 1, 2, 3 };
    unit1.list2 = { 4, 5 };
    unit1.set1 = { 6, 7, 8, 9 };
    unit1.set2 = { 10 };

    unit1.map1 = { { { 1, 3.0f }, 3 },
        { { 2, 4.0f }, 6 } };

    unit1.complexList1.emplace_back(2, 6.0f, "set");
    unit1.complexList1.emplace_back();

    const char *s = "Hello World!";
    unit1.bytes = { s, strlen(s) };

    std::unique_ptr<std::stringbuf> pBuffer = std::make_unique<std::stringbuf>();

    std::stringbuf *buffer = pBuffer.get();

    FormattedSerializeStream stream { format(), SerializeStream { std::move(pBuffer) } };

    write(stream, unit1, "unit1");

    std::string file = buffer->str();

    std::cout << file << std::endl;

    if (!stream.isBinary())
        EXPECT_EQ(expected.data(), file);

    std::cout << "Buffer-Size: " << file.size() << std::endl;

    HANDLE_STREAM_RESULT(read(stream, unit2, nullptr));

    ASSERT_EQ(unit1, unit2);
}