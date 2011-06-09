#ifndef MBC5_H
#define MBC5_H

#include "Mbc1.h"

class Mbc5 : public Mbc1
{
public:

	Mbc5();

	virtual void Write8(u16 address, u8 value);
};

#endif
