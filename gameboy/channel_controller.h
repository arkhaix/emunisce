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
#ifndef CHANNELDISABLER_H
#define CHANNELDISABLER_H

#include "platform_types.h"

namespace emunisce {

class ChannelController {
public:
	ChannelController(u8& nr52, int channelBit);

	void EnableChannel();
	void DisableChannel();
	bool IsChannelEnabled();

private:
	u8& m_nr52;
	int m_channelBit;
};

}  // namespace emunisce

#endif
