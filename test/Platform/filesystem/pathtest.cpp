#include "Platform/platformlib.h"

#include "Platform/filesystem/path.h"

#include <gtest/gtest.h>

TEST(Path, Basic)
{
    Engine::Platform::Filesystem::Path p1 { "foo/test" };
    Engine::Platform::Filesystem::Path p2 { "foo\\test2/mixed\\" };

    ASSERT_EQ(p2.str(), "foo/test2/mixed");
    ASSERT_EQ(p1.str(), "foo/test");
    ASSERT_EQ(p1.parentPath().str(), "foo");

    Engine::Platform::Filesystem::Path p3 { "foo/test.txt" };

    ASSERT_EQ(p3.stem(), "test");
    ASSERT_EQ(p3.extension(), ".txt");
}
