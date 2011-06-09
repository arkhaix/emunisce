#ifndef MBC3_H
#define MBC3_H

#include "Mbc1.h"

class Mbc3 : public Mbc1
{
public:

	Mbc3();

	virtual void Write8(u16 address, u8 value);

protected:


};

#endif
