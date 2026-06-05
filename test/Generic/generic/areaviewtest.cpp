#include <gtest/gtest.h>

#include "Platform/platformlib.h"

#include "Generic/areaview.h"

#include "logContainer.h"

TEST(AreaView, Basic)
{
    size_t area[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    Engine::AreaView<size_t, 2> view1 { area, { 4, 4 } };

    ASSERT_EQ(view1[2][2], 11);

    logContainer(view1);

    Engine::AreaView<size_t, 2> view2 = view1.subArea({ 0, 1 }, { 3, 2 });

    ASSERT_EQ(view2[1][2], 11);

    logContainer(view2);

    view2.swapAxis(0, 1);

    EXPECT_EQ(view2[2][1], 11);

    logContainer(view2);

    view2.flip(1);

    EXPECT_EQ(view2[0][1], 11);

    logContainer(view2);
}
