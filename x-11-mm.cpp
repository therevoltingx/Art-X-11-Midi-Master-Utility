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
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>
#include <ctime>

#ifdef __WINDOWS_MM__
#include <windows.h>
#endif

using namespace std;
#include "parser.h"
#include "rtmidi/RtMidi.h"
#include "rtmidi/RtError.h"
#include "getopt_pp/getopt_pp_standalone.h"
using namespace GetOpt;

#define APPNAME "x-11-mm"
#define X11_MM

void print_description();
void set_options(int, char **);
void cleanup();
void main_loop();
void ListInputs(RtMidiIn *);
void ListOutputs(RtMidiOut *);
void ListDevices();
void MidiInCB( double deltatime, vector<unsigned char> *, void *);
void SendControlValue( int Control, int Value);
void MapChannel(int channel, bool isSysEx);
void SendMidiClock(int Value);
void test_function();

bool DEBUG = false;
string ConfName;
int MidiInIndex = 0;
int MidiOutIndex = 0;

vector<SYSEX_INFO> SysEx_v(250);		//TODO: FIX THIS THERE SHOULD BE NO LIMIT, SHOULD EXPAND/CONTRACT AUTOMATICALLY
RtMidiIn *MidiIn = NULL;
RtMidiOut *MidiOut = NULL;


void MapChannel(int channel, bool isSysEx)
{
#ifdef X11_MM
channel++;	//My X11 MIDI Master has on offset of 1
#endif
//check if this channel is mapped
vector<SYSEX_INFO>::iterator iter1 = SysEx_v.begin();
int index = 0;
while (iter1 != SysEx_v.end())
	{
	struct SYSEX_INFO MySysEx = *iter1;
	if ( MySysEx.MIDI_CHANNEL == channel )
		{
		//if we got sysex chan change command, but is not mapped.  again bug fix 
		if ( isSysEx )	if ( !MySysEx.IsSysEx )	return;		//:-b
		if ( !MySysEx.TOGGLE )	//regular stuff...
			{
			//MAPPED!
			//check if CONTROL_VAL is a midi clock message....
			if ( MySysEx.CONTROL_VAL == MIDI_START )
				{
				if (DEBUG) cout << "DEBUG: SENDING MIDI CLOCK START MESSAGE" << endl;
				SendMidiClock(MIDI_START);
				}
			else if ( MySysEx.CONTROL_VAL == MIDI_STOP ) 
				{
				if (DEBUG) cout << "DEBUG: SENDING MIDI CLOCK STOP MESSAGE" << endl;
				SendMidiClock(MIDI_STOP);
				}
			else if ( MySysEx.CONTROL_VAL == MIDI_CONTINUE ) 
				{
				if (DEBUG) cout << "DEBUG: SENDING MIDI CLOCK CONTINUE MESSAGE" << endl;
				SendMidiClock(MIDI_CONTINUE);
				}
			else
				{
				SendControlValue( MySysEx.CONTROL_ID, MySysEx.CONTROL_VAL );
				}
			//break;
			cout << "MAPPED: " << MySysEx.MIDI_CHANNEL << " to [" << MySysEx.CONTROL_ID \
				<< ", " << MySysEx.CONTROL_VAL << "]" << endl;
			}
		if ( MySysEx.TOGGLE )
			{
			if ( MySysEx.STATE )
				{
				if (DEBUG) cout << "DEBUG: STATE OF " << channel << " is : " << MySysEx.STATE << endl;
				if (DEBUG) cout << "DEBUG: MAPPING: " << channel << " to: " << MySysEx.CONTROL_VAL << endl;
				SendControlValue( MySysEx.CONTROL_ID2, MySysEx.CONTROL_VAL );
				SysEx_v.erase( iter1 );
				MySysEx.STATE = false;
				cout << "MAPPED: " << MySysEx.MIDI_CHANNEL << " to [" << MySysEx.CONTROL_ID \
				<< ", " << MySysEx.CONTROL_VAL << "]" << endl;
				SysEx_v.push_back( MySysEx ); 	
				break;
				}
			else
				{
				if (DEBUG) cout << "DEBUG: STATE OF " << channel << " is : " << MySysEx.STATE << endl;
				if (DEBUG) cout << "DEBUG: MAPPING: " << channel << " to: " << MySysEx.CONTROL_VAL2 << endl;
				SendControlValue( MySysEx.CONTROL_ID, MySysEx.CONTROL_VAL2 );
				SysEx_v.erase( iter1 );
				MySysEx.STATE = true;
				cout << "MAPPED: " << MySysEx.MIDI_CHANNEL << " to [" << MySysEx.CONTROL_ID \
					<< ", " << MySysEx.CONTROL_VAL2  << "]" << endl;
				SysEx_v.push_back( MySysEx ); 	
				break;
				}
			
			
			}
		
		}
	iter1++;
	}
	if (DEBUG) 
		{
	cout << "GOT CHAN CHNG: " << channel << endl;
	cout << "----------------------------------------------------------------" << endl;
	}
}

void MidiInCB( double deltatime, vector<unsigned char> *message, void *Data)
{
unsigned int nBytes = message->size();
if (DEBUG)
	{	
	cout << "EVENT SIZE: " << nBytes << endl;
	for ( unsigned int i=0; i < nBytes; i++ )
	cout << "Byte " << i << " = " << (int) message->at(i) << ", ";
	if ( nBytes > 0 )
		cout << endl << "stamp = " << deltatime << endl;	
	}

if (nBytes == 0) return;
int byte0 = (int) message->at(0);
int byte1 = (int) message->at(1);
if ( byte0 == 192 )	//192 = CHN CHNG 
	{
	MapChannel( (int) message->at(1), false );
	}
#ifdef X11_MM
else if ( byte0 == 240 ) // SYSEX?
	{
	if (DEBUG) cout << "GOT SYSEX: " << (int) message->at(4) << endl;
	MapChannel( (int) message->at(4) - 2, true);	//errr, to fix offset bug
	}
#endif
}

void SendMidiClock(int Value)
{
vector<unsigned char> output;
output.push_back( Value );
try
	{
	if (!MidiOut) { cerr << "MIDI DEVICE POINTER IS NULL!!" << endl; }
	MidiOut->sendMessage( &output );
	}
catch (RtError &error)
	{
	error.printMessage();
	cleanup();
	exit(1);
	}
}

void SendControlValue(int Control, int Value)
{
vector<unsigned char> output;
output.push_back( 176 );	//CONTROL CHANGE
output.push_back( Control );	//CONTROL ID
output.push_back( Value ); //SET VALUE
try
	{
	if (!MidiOut) { cerr << "MIDI DEVICE POINTER IS NULL!!" << endl; }
	MidiOut->sendMessage( &output );
	}
catch (RtError &error)
	{
	error.printMessage();
	cleanup();
	exit(1);
	}
}

int main( int argc, char** argv )
{
print_description();
set_options(argc, argv);

if (!ParseChanMap(ConfName, &SysEx_v)) 
	{
	exit(1);
	}
try 	{
	MidiIn = new RtMidiIn();
	MidiOut = new RtMidiOut();
	cout << "Using MIDI IN #" << MidiInIndex << "/" << MidiIn->getPortName(MidiInIndex) << endl;
	cout << "Using MIDI OUT #" << MidiOutIndex << "/" << MidiOut->getPortName(MidiOutIndex) << endl;
	MidiIn->openPort(MidiInIndex);
//	MidiIn->openVirtualPort("SysEx2Ctrl INPUT");
	MidiOut->openPort(MidiOutIndex);
	MidiIn->setCallback( &MidiInCB );
//	#ifdef X11_MM
//	MidiIn->ignoreTypes( false, true, true );	//don't ignore sysex messages
//#	#else
	MidiIn->ignoreTypes( true, true, true );	//ignore sysex messages
//	#endif
	}
catch (RtError &error)
	{
	error.printMessage();
	cerr << "Exiting..." << endl;
	cleanup();
	exit(1);
	}
cout << "Listening for events..." << endl;
main_loop();
cleanup();
return 0;
}

int msleep(unsigned long milisec)  
{  
#ifdef __WINDOWS_MM__
Sleep(milisec);
#else
//http://cc.byexamples.com/20070525/nanosleep-is-better-than-sleep-and-usleep/
struct timespec req={0};
time_t sec=(int)(milisec/1000);
milisec=milisec-(sec*1000);
req.tv_sec=sec;
req.tv_nsec=milisec*1000000L;
while(nanosleep(&req,&req)==-1)
	continue;
#endif
return 1;
}

void main_loop()
{
while (1)
	{ 
	msleep(1);
	//if (DEBUG) cout << "Sleep Over" << endl;
	}
//test_function();
exit(0);
}

void print_description()
{
cout << APPNAME << endl;
cout << "Turn the ART X-11 MIDI MASTER CONTROL Messages into MIDI CCs" << endl;
cout << "Tweak effects with your feet, for cheap!" << endl;
cout << "Enjoi! ^_^" << endl;
cout << "Miguel Morales; Thanks to: RtMidi and GetOpt_p developers!!!" << endl;
cout << "therevoltingx@gmail.com" << endl;
}

void print_usage()
{
cout << "Usage: " << endl;
cout << APPNAME << endl;
cout << "Options" << endl;
cout << "-h Show Usage" << endl;
cout << "-d Enable Debug Mode" << endl;
cout << "-l List Midi Devices" << endl;
cout << "-i [number] Use specified MIDI input device." << endl;
cout << "-o [number] Use specified MIDI output device." << endl;
cout << "-c [path] Use specified mapping configuration file." << endl;
}

void ListDevices()
{
RtMidiIn *MyMidiIn = NULL;
RtMidiOut *MyMidiOut = NULL;
cout << "Available MIDI Devices: " << endl;
try 	{
	MyMidiIn = new RtMidiIn();
	MyMidiOut = new RtMidiOut();
	}
catch (RtError &error)
	{
	error.printMessage();
	exit(1);
	}
ListInputs(MyMidiIn);
cout << "------------------------------------" << endl;
ListOutputs(MyMidiOut);
delete MyMidiIn;
delete MyMidiOut;
}

void ListInputs(RtMidiIn *m)
{
unsigned int nPorts = m->getPortCount();
cout << "Available Inputs: " << nPorts << endl;
try 
	{
	for ( unsigned int x = 0; x < nPorts; x++ )
		{
		cout << x << ") " << m->getPortName(x) << endl;
		}
	}
catch ( RtError &error)
	{
	error.printMessage();
	}
}

void ListOutputs(RtMidiOut *m)
{
unsigned int nPorts = m->getPortCount();
cout << "Available Outputs: " << nPorts << endl;
try 
	{
	for ( unsigned int x = 0; x < nPorts; x++ )
		{
		cout << x << ") " << m->getPortName(x) << endl;
		}
	}
catch ( RtError &error)
	{
	error.printMessage();
	}
}

void cleanup()
{
if (MidiIn) 
	{
	MidiIn->closePort();
	delete MidiIn;
	}
if (MidiOut)
	{
	MidiOut->closePort();
	delete MidiOut;
	}
}

void set_options(int argc, char **argv)
{
GetOpt_pp opts(argc, argv);
opts >> OptionPresent('d', "debug", DEBUG);
opts >> Option('c', "conf", ConfName);
opts >> Option('i', "in", MidiInIndex);
opts >> Option('o', "out", MidiOutIndex);

if (DEBUG) cout << "DEBUG MODE ON" << endl;
if (!(opts >> OptionPresent('c', "conf")))
	ConfName = "map.txt";
if ((opts >> OptionPresent('l', "list")))
	{
	ListDevices();
	exit(0);
	}
if ((opts >> OptionPresent('h', "help")))
	{
	print_usage();
	exit(0);
	}

}

void test_function()
{
//dummy function to test functionality
MapChannel(3, false);	//on
MapChannel(3, false);	//off
MapChannel(3, false);	//on
MapChannel(3, false);	//off
MapChannel(4, false);	//on
MapChannel(3, false);	//on
MapChannel(4, false);	//off
MapChannel(3, false);	//off
MapChannel(4, false);	//on

MapChannel(5, false);
MapChannel(6, false);
MapChannel(7, false);
}
