#include "channelDisabler.h"

ChannelDisabler::ChannelDisabler(u8& nr52, int channelBit)
	: m_nr52(nr52)
{
	m_channelMask = ~(1<<channelBit);
}

void ChannelDisabler::DisableChannel()
{
	m_nr52 &= m_channelMask;
}
