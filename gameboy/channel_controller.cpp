/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

This file is part of Emunisce.

Emunisce is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Emunisce is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Emunisce.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "channel_controller.h"
using namespace emunisce;

ChannelController::ChannelController(u8& nr52, int channelBit) : m_nr52(nr52), m_channelBit(channelBit) {
}

void ChannelController::EnableChannel() {
	m_nr52 |= (1 << m_channelBit);
}

void ChannelController::DisableChannel() {
	m_nr52 &= ~(1 << m_channelBit);
}

bool ChannelController::IsChannelEnabled() {
	if (m_nr52 & (1 << m_channelBit)) {
		return true;
	}

	return false;
}
