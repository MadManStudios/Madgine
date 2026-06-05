#include <gtest/gtest.h>

#include "Platform/platformlib.h"

#include "Generic/containers/bits/array.h"
#include "Generic/containers/bits/variant.h"
#include "Generic/containers/bits/ptr.h"

#include "../logContainer.h"

TEST(Bits, Basic)
{
    constexpr bool b1 = std::same_as<Engine::Containers::MinimalHoldingUIntType_t<1>, bool>;
    ASSERT_TRUE(b1);
    constexpr bool b2 = std::same_as<Engine::Containers::MinimalHoldingUIntType_t<5>, uint8_t>;
    ASSERT_TRUE(b2);
    constexpr bool b3 = std::same_as<Engine::Containers::MinimalHoldingUIntType_t<63>, uint64_t>;
    ASSERT_TRUE(b3);
}

TEST(Bits, BitArray)
{

    Engine::Containers::BitArray<10, 11> data;

    size_t i = 1;
    for (auto field : data) {
        field = i;
        i += 1;
    }

    logContainer(data);

    i = 1;
    for (auto field : data) {
        ASSERT_EQ(field, i);
        i += 1;
    }
}

TEST(Bits, BitVariant)
{
    struct Test {
        Test(size_t *c)
            : mCounter(c)
        {
            ++(*mCounter);
        }

        ~Test()
        {
            ++(*mCounter);
        }

        size_t *mCounter;
    };
    Engine::Containers::BitVariant<Engine::Containers::BitPtr<int>, Engine::Containers::BitArray<2, 15>, Engine::Containers::BitUniquePtr<Test>> v;

    ASSERT_TRUE(v.is<Engine::Containers::BitPtr<int>>());
    ASSERT_EQ(v.as<Engine::Containers::BitPtr<int>>(), nullptr);

    Engine::Containers::BitArray<2, 15> ba;
    ba[0] = 23;
    ba[1] = 53;

    v = ba;

    Engine::Containers::BitArray<2, 15> ba2 = v.as<Engine::Containers::BitArray<2, 15>>();
    logContainer(ba2);
    ASSERT_EQ(ba, ba2);

    size_t counter = 0;
    v = std::make_unique<Test>(&counter);
    ASSERT_EQ(counter, 1);
    v = (int *)nullptr;
    ASSERT_EQ(counter, 2);
}