#ifndef MBC5_H
#define MBC5_H

#include "mbc1.h"

class MBC5 : public MBC1
{
public:

	MBC5();

	virtual void Write8(u16 address, u8 value);
};

#endif
