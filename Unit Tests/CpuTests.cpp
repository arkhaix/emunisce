/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

This file is part of PhoenixGB.

PhoenixGB is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

PhoenixGB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PhoenixGB.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "gtest/gtest.h"

#include "../Cpu/Cpu.h"

class CPUTest : public ::testing::Test
{
protected:

	Cpu cpu;
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