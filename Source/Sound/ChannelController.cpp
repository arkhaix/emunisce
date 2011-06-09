#include "ChannelController.h"

ChannelController::ChannelController(u8& nr52, int channelBit)
	: m_nr52(nr52)
	, m_channelBit(channelBit)
{
}

void ChannelController::EnableChannel()
{
	m_nr52 |= (1<<m_channelBit);
}

void ChannelController::DisableChannel()
{
	m_nr52 &= ~(1<<m_channelBit);
}

bool ChannelController::IsChannelEnabled()
{
	if(m_nr52 & (1<<m_channelBit))
		return true;

	return false;
}

