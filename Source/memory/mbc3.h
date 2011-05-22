#ifndef MBC3_H
#define MBC3_H

#include "mbc1.h"

class MBC3 : public MBC1
{
public:

	MBC3();

	virtual void Write8(u16 address, u8 value);

protected:


};

#endif
