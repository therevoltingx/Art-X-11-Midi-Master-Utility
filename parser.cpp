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

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <string>
using namespace std;
#include "parser.h"

bool ParseChanMap(string FileName, vector<SYSEX_INFO> *SysEx_v)
{
ifstream File;
string Line;
File.open(FileName.c_str());
if (!File)
	{
	cerr << "Error Opening Configuration File: " << FileName << endl;
	return false;
	}
int prev_channel = -1;	//for toggling feature
while (getline(File, Line))
	{
	string::size_type i = Line.find_first_not_of ( " \t\n\v" );
	if ( i != string::npos && Line[i] == '#' ) continue;
	if ( Line.length() == 0 ) continue;
	bool isToggle = false;
	bool isSysEx = false;
	string bytes_tmp;
	int channel;
	int index;
		//extract the channel from the line
		{
		int start, end = 0;
		string channel_tmp;
		start = Line.find_first_of('[');
		end = Line.find_first_of(']');
		channel_tmp = Line.substr( start + 1, end - start - 1 );
		//check if it begins with 0x and flag as sysex message 
		int offset = channel_tmp.find("0x");
		if (offset  == string::npos )
			{
			channel = atoi(channel_tmp.c_str());
			}
		else
			{
			//sysex message detected 
			channel = -1;
			bytes_tmp = channel_tmp.substr( offset + 2, channel_tmp.length() );
			isSysEx = true;
			cout << "PARSER: DETECTED SYSEX " << bytes_tmp << endl;
			channel = atoi( bytes_tmp.c_str() );
			}
			
		//cout << channel_tmp << endl;
		}
	Line = Line.substr( Line.find_first_of('=') + 1, Line.length() );	//discard first section
	//cout << "LINE: " << Line << endl;
	int ctrl, val = 0;
	string CLOCK_MESSAGE;
		{
		string List;
		int list_start = Line.find_first_of('[', index) + 1;
		int list_end = Line.find_first_of(']', index);
		int list_length = (list_end - list_start);
		List = Line.substr(list_start, list_length);
		//cout << "LIST: " << List << endl;
		string ctrl_tmp = List.substr(1, List.find_first_of(','));
		ctrl = atoi(ctrl_tmp.c_str());
		//cout << "CTRL = " << ctrl_tmp << endl;
		int sub_index = List.find_last_of(',') + 1;
		//string val_tmp = List.substr(sub_index, List.find_last_of(']') - sub_index);
		string val_tmp = List.substr(sub_index, List.find_last_of(']') - sub_index);
		if (val_tmp.find("START") || val_tmp.find("STOP") || val_tmp.find("CONTINUE") )
			CLOCK_MESSAGE = val_tmp;
		val = atoi(val_tmp.c_str());
		//cout << "VAL = " << val_tmp << endl;
		}
	//check if it matches previous channel for toggling feature
	if ( (prev_channel == -1 || prev_channel != channel))
		{
		prev_channel = channel;
		SYSEX_INFO MySysEx;
		MySysEx.MIDI_CHANNEL = channel;
		MySysEx.CONTROL_ID = ctrl;
		if (CLOCK_MESSAGE.find ("START") != string::npos)
			MySysEx.CONTROL_VAL = MIDI_START;
		else if (CLOCK_MESSAGE.find("STOP") != string::npos)
			MySysEx.CONTROL_VAL = MIDI_STOP;
		else if (CLOCK_MESSAGE.find("CONTINUE") != string::npos)
			MySysEx.CONTROL_VAL = MIDI_CONTINUE;
		else
			MySysEx.CONTROL_VAL = val;
		MySysEx.CONTROL_ID2 = -1;
		MySysEx.CONTROL_VAL2 = -1;
		MySysEx.TOGGLE = false;
		MySysEx.IsSysEx = isSysEx;
		isToggle = false;
		cout << "PARSER: CHANNEL " << channel;
		cout << " Maps to [" << MySysEx.CONTROL_ID << ", " << MySysEx.CONTROL_VAL << "]" << endl;
		SysEx_v->push_back( MySysEx );
		}
	else if (channel == prev_channel)
		{
		isToggle = true;
		prev_channel = -1;
		//set previous vector values 
		vector<SYSEX_INFO>::iterator last = SysEx_v->end() - 1;
		struct SYSEX_INFO PrevSysEx = *last;
		struct SYSEX_INFO NewSysEx;
		NewSysEx.MIDI_CHANNEL = PrevSysEx.MIDI_CHANNEL;
		NewSysEx.TOGGLE = true;	
		NewSysEx.CONTROL_ID = PrevSysEx.CONTROL_ID;
		NewSysEx.CONTROL_ID2 = ctrl;
		NewSysEx.CONTROL_VAL = PrevSysEx.CONTROL_VAL;
		NewSysEx.CONTROL_VAL2 = val;
		NewSysEx.STATE = false;	//OFF BY DEFAULT
		NewSysEx.IsSysEx = isSysEx;
		SysEx_v->pop_back();
		SysEx_v->push_back( NewSysEx );
		cout << "PARSER: ENABLED TOGGLE FOR CHANNEL ";
		cout << PrevSysEx.MIDI_CHANNEL << endl;
		}
	
/*
else if (isSysEx)
		{
		prev_channel = channel;
		struct SYSEX_INFO NewSysEx;
		NewSysEx.IsSysEx = true;
		NewSysEx.TOGGLE = false;
		NewSysEx.CONTROL_ID = ctrl;
		NewSysEx.CONTROL_ID2 = -1;
		NewSysEx.CONTROL_VAL = val;
		NewSysEx.CONTROL_VAL2 = -1;
		NewSysEx.MIDI_CHANNEL = channel;
		NewSysEx.STATE = false;
		SysEx_v->push_back( NewSysEx );
		cout << "PARSER: MAP [" << bytes_tmp << "] = [" << ctrl << ", " << val << "]" << endl;
		}
*/
	}
File.close();
return true;
}

/*
bool ParseConfFile(string FileName, vector<SYSEX_INFO> *SysEx_v)
{
ifstream File;
string Line;
File.open(FileName.c_str());
if (!File)
	{
	cerr << "Error Opening Configuration File: " << FileName << endl;
	return false;
	}
while (getline(File, Line))	//clarity over efficiency, fairly trivial task.
	{
	string::size_type i = Line.find_first_not_of ( " \t\n\v" );
	if ( i != string::npos && Line[i] == '#' ) continue;
	if ( Line.length() == 0 ) continue;
	SYSEX_INFO MySysEx;
	string bytes;
	int index = 0;
		//extract the bytes from the line
		{
		index = Line.find_first_of('=');
		bytes = Line.substr( 0, index);
		}
	int channel, ctrl, val = 0;
		{
		string List;
		int list_start = Line.find_first_of('[', index) + 1;
		int list_end = Line.find_first_of(']', index);
		int list_length = (list_end - list_start);
		List = Line.substr(list_start, list_length);
		string chan_tmp = List.substr(0, List.find_first_of(','));
		channel = atoi(chan_tmp.c_str());
		string ctrl_tmp = List.substr(List.find_first_of(',') + 1, List.find_last_of(',') - 2);	//is this right?
		ctrl = atoi(ctrl_tmp.c_str());
		int sub_index = List.find_last_of(',');
		string val_tmp = List.substr(sub_index + 1, List.length() - sub_index);
		val = atoi(val_tmp.c_str());
		}
	MySysEx.MIDI_CHANNEL = channel;
	MySysEx.CONTROL_ID = ctrl;
	MySysEx.CONTROL_VAL = val;
	cout << "PARSER: ";
	for (int x = 0; x < bytes.size(); x++)
		{
		MySysEx.BYTES.push_back(bytes.at(x));
		cout << bytes.at(x);
		}
	SysEx_v->push_back( MySysEx );		
	cout << " Maps to [" << MySysEx.MIDI_CHANNEL << ", " << 
		MySysEx.CONTROL_ID << ", " << MySysEx.CONTROL_VAL << "]" << endl;
	}
File.close();
return true;
}
*/
