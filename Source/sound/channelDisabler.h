#ifndef CHANNELDISABLER_H
#define CHANNELDISABLER_H

#include "../common/types.h"


class ChannelDisabler
{
public:

	ChannelDisabler(u8& nr52, int channelBit);

	void DisableChannel();

private:

	u8& m_nr52;
	int m_channelMask;
};

#endif
