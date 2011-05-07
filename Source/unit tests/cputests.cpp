#include "gtest/gtest.h"

#include "../cpu/cpu.h"

class CPUTest : public ::testing::Test
{
protected:

	CPU cpu;
};

TEST_F(CPUTest, RegisterMapping)
{
	cpu.af = 0x0102;
	cpu.bc = 0x0304;
	cpu.de = 0x0506;
	cpu.hl = 0x0708;

	EXPECT_EQ(0x01, cpu.a);
	EXPECT_EQ(0x02, cpu.f);
	EXPECT_EQ(0x03, cpu.b);
	EXPECT_EQ(0x04, cpu.c);
	EXPECT_EQ(0x05, cpu.d);
	EXPECT_EQ(0x06, cpu.e);
	EXPECT_EQ(0x07, cpu.h);
	EXPECT_EQ(0x08, cpu.l);
}

TEST_F(CPUTest, InitialValues)
{
	EXPECT_EQ(0x0100, cpu.pc);
	EXPECT_EQ(0xfffe, cpu.sp);
	EXPECT_EQ(0x01b0, cpu.af);
	EXPECT_EQ(0x0013, cpu.bc);
	EXPECT_EQ(0x00d8, cpu.de);
	EXPECT_EQ(0x014d, cpu.hl);
}