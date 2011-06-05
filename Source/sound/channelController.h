#ifndef CHANNELDISABLER_H
#define CHANNELDISABLER_H

#include "../common/types.h"


class ChannelController
{
public:

	ChannelController(u8& nr52, int channelBit);

	void EnableChannel();
	void DisableChannel();
	bool IsChannelEnabled();

private:

	u8& m_nr52;
	int m_channelBit;
};

#endif
