/*
    Copyright © 2008	Miguel Morales
    This file is part of x-11-mm.

    x-11-mm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    x-11-mm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with x-11-mm.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __X_11_MM_H__
#define __X_11_MM_H__

#include <vector>

#define MIDI_START 250
#define MIDI_CONTINUE 251
#define MIDI_STOP 252

class SYSEX_INFO	//misleading name, started off with a different idea
	{
	public:
	std::vector<unsigned char> BYTES;
	int MIDI_CHANNEL;
	int CONTROL_ID;
	int CONTROL_ID2;
	int CONTROL_VAL;
	int CONTROL_VAL2;
	bool TOGGLE;
	bool STATE;	//ON/OFF 	true=on false=off
	bool IsSysEx;
	};
#endif

