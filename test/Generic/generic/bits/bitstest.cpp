#include <gtest/gtest.h>

#include "Interfaces/interfaceslib.h"

#include "Generic/bits/array.h"
#include "Generic/bits/variant.h"
#include "Generic/bits/ptr.h"

#include "../logContainer.h"

TEST(Bits, Basic)
{
    constexpr bool b1 = std::same_as<Engine::MinimalHoldingUIntType_t<1>, bool>;
    ASSERT_TRUE(b1);
    constexpr bool b2 = std::same_as<Engine::MinimalHoldingUIntType_t<5>, uint8_t>;
    ASSERT_TRUE(b2);
    constexpr bool b3 = std::same_as<Engine::MinimalHoldingUIntType_t<63>, uint64_t>;
    ASSERT_TRUE(b3);
}

TEST(Bits, BitArray)
{

    Engine::BitArray<10, 11> data;

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
    Engine::BitVariant<Engine::BitPtr<int>, Engine::BitArray<2, 15>, Engine::BitUniquePtr<Test>> v;

    ASSERT_TRUE(v.is<Engine::BitPtr<int>>());
    ASSERT_EQ(v.as<Engine::BitPtr<int>>(), nullptr);

    Engine::BitArray<2, 15> ba;
    ba[0] = 23;
    ba[1] = 53;

    v = ba;

    Engine::BitArray<2, 15> ba2 = v.as<Engine::BitArray<2, 15>>();
    logContainer(ba2);
    ASSERT_EQ(ba, ba2);

    size_t counter = 0;
    v = std::make_unique<Test>(&counter);
    ASSERT_EQ(counter, 1);
    v = (int *)nullptr;
    ASSERT_EQ(counter, 2);
}