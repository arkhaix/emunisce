#include "gtest/gtest.h"

TEST(Things, ThingsBreak)
{
	ASSERT_EQ(1, 0);
}

TEST(Things, ThingsWork)
{
	ASSERT_EQ(1, 1);
}

GTEST_API_ int main(int argc, char **argv) 
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
