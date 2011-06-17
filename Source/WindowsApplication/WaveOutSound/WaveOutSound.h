/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

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
#ifndef WAVEOUTSOUND_H
#define WAVEOUTSOUND_H


namespace Emunisce
{

class Phoenix;
class IEmulatedMachine;

class WaveOutSound
{
public:

	WaveOutSound();
	~WaveOutSound();

	void Initialize(Phoenix* phoenix);
	void Shutdown();

	void SetMachine(IEmulatedMachine* machine);

	void SetMute(bool mute);

private:

	class WaveOutSound_Private* m_private;
};

}	//namespace Emunisce

#endif
