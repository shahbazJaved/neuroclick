// Description:
// NeuroClick is a program that emulates mouse clicks with eye blinks 
// why using the NeuroSky MindWave headset. http://neurosky.com/
//
// Main author: Franck Dernoncourt <http://francky.me>
// NeuroClick home page: http://francky.me/software.php#Neuroclick2012
// Version: 0.2 (20120721)
//
// Eye blink detection accuracy:
// Ca. 90% accurate, due to the NeuroSky MindWave API which misses around 10% 
// of eye blinks. Such a good accuracy is perhaps due to the fact that I place
// the sensor very close to my left eye, virtually on my eyebrow. Picture: 
// http://img694.imageshack.us/img694/1027/20120705005354.jpg. 
//
// What if I don't want to click but I need to blink ? 
// * Most of the time the mouse cursor is placed on a neutral position so 
//   it does not matter if you click;
// * For Neurosky to maximize eyeblink detection you have to blink strongly.
//
// Compatibility: 
// Tested on Windows 7 x64 with multiscreen. It should work on any Windows.
// Note that <windows.h> is used to emulate mouse clicks, this code will not
// compile on operating systems other than Windows.
//
//
// Copyright (C) 2012:               
//               - Franck Dernoncourt <http://francky.me>
//               - Neurosky team <http://neurosky.com>
// This program is licensed under the Creative Commons
// Attribution-Noncommercial-No Derivative Works 3.0 Unported license
// <http://creativecommons.org/licenses/by-nc-nd/3.0/legalcode>
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Ideas to investigate:
// * Being able to drag and drop would be great. However I am definitely not sure whether the eye blinking detection's accuracy and frequency would allow a smooth drag-and-drop.
// * How bad is it to blink as many times as we use mouse clicks?
// * Distinguishing left-click from right-click is currently impossible due to the fact that there is only one sensor, which mostly detects the left blinking (seems the left eye is the closest to the sensor).
// * Is there any way to avoid that the program eats 100% CPU of a core? (apart from using Sleep())
// * I find the headset quite incomfortable (it hurts my forehead) even though I find it not excessively intrusive. Please add your voice to the poll http://www.linkedin.com/groups/How-comfortable-is-NeuroSky-MindWave-3572341.S.130985046?view=&gid=3572341&type=member&item=130985046&trk=NUS_DISC_Q-ttle !
//
// Any feedback/comments/ideas are welcome! --> franck.dernoncourt@gmail.com



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "thinkgear.h"

/**
* Prompts and waits for the user to press ENTER.
*/
void
	wait() {
		printf( "\n" );
		printf( "Press the ENTER key...\n" );
		fflush( stdout );
		getc( stdin );
}

/**
* Program which prints ThinkGear EEG_POWERS values to stdout.
*/
int
	main( void ) {

		// http://andre.stechert.org/urwhatu/2006/01/error_c2143_syn.html
		// It's no surprise that Microsoft's compiler wants the variables at the beginnig of the function: Standard C wants them there.
		// At least, this is what C89 defines. C99 allows variable declarations also in the middle of a function.
		char *comPortName = NULL;
		char *portNumber = (char*)malloc(sizeof(char) * (2 + 1));
		char *comPortBase = NULL;
		int   dllVersion = 0;
		int   connectionId = 0;
		int   packetsRead = 0;
		int   errCode = 0;
		int comPortFound = 0;
		const int MAX_PORT = 16;
		size_t length = 0;
		int i = 0;
		int j = 0;
		int x = 0;
		int y = 0;
		INPUT input;
		POINT cursorPos;

		/* Print driver version number */
		dllVersion = TG_GetDriverVersion();
		printf( "ThinkGear DLL version: %d\n", dllVersion );

		/* Get a connection ID handle to ThinkGear */
		connectionId = TG_GetNewConnectionId();
		if( connectionId < 0 ) {
			fprintf( stderr, "ERROR: TG_GetNewConnectionId() returned %d.\n", 
				connectionId );
			wait();
			exit( EXIT_FAILURE );
		}
		fprintf( stderr, "ThinkGear Connection ID is: %d.\n\n", connectionId );

		/* Set/open stream (raw bytes) log file for connection */
		errCode = TG_SetStreamLog( connectionId, "streamLog.txt" );
		if( errCode < 0 ) {
			fprintf( stderr, "ERROR: TG_SetStreamLog() returned %d.\n", errCode );
			wait();
			exit( EXIT_FAILURE );
		}

		/* Set/open data (ThinkGear values) log file for connection */
		errCode = TG_SetDataLog( connectionId, "dataLog.txt" );
		if( errCode < 0 ) {
			fprintf( stderr, "ERROR: TG_SetDataLog() returned %d.\n", errCode );
			wait();
			exit( EXIT_FAILURE );
		}

		/* Attempt to connect the connection ID handle to serial ports between COM0 and "COM MAX_PORT" */
		fprintf(stdout, "Scanning COM ports 0 to %d...\n", MAX_PORT);
		comPortBase = "\\\\.\\COM";
		length = strlen(comPortBase);
		comPortName = (char *)realloc (comPortName, (length + 5)*sizeof(char)); 

		for(i=0; i <= MAX_PORT && comPortFound == 0; i++)
		{

			// Generating the serial port number			
			portNumber = itoa(i, portNumber, 10);
			fprintf( stderr, portNumber);
			strcpy(comPortName,comPortBase);

			for(j=0; j<strlen(portNumber); j++)
			{
				comPortName[length+j] = portNumber[j];
			}

			comPortName[length+strlen(portNumber)] = '\0';
			// Maybe I could have used something like strcat(comPortBase, portNumber);

			fprintf( stdout, "ok");
			fprintf( stdout, comPortName);

			//comPortName = "\\\\.\\COM11";

			// Trying to connect on the generated serial port number
			fprintf( stdout, "trying to connect on");
			fprintf( stdout, comPortName);
			fprintf( stdout, "\n");
			errCode = TG_Connect( connectionId, 
				comPortName, 
				TG_BAUD_9600, 
				TG_STREAM_PACKETS );
			if( errCode < 0 ) {
				fprintf( stderr, "ERROR: TG_Connect() returned %d.\n", errCode );
				if(errCode == -1) printf("FAILED connection (-1 connectionId does not refer to a valid ThinkGear Connection ID handle.)\n");
				if(errCode == -2) printf("FAILED connection (-2 serialPortName could not be opened as a serial communication port for any reason.)\n");
				if(errCode == -3) printf("FAILED connection (-3 serialBaudrate is not a valid TG_BAUD_* value.)\n");
				if(errCode == -4) printf("FAILED connection (-4 serialDataFormat is not a valid TG_STREAM_* type.)\n");
			} 
			else
			{

				// Trying to read one packet to check the connection.
				printf("Connection available...\n");
				Sleep(10000); // sometimes we need to wait a little...
				errCode = TG_ReadPackets(connectionId, 1);
				if(errCode >= 0)
				{	
					printf("OK\n");
					comPortFound = 1;
				}
				else
				{
					if(errCode == -1) printf("FAILED reading (Invalid connection ID)\n");
					if(errCode == -2) printf("FAILED reading (0 bytes on the stream)\n");
					if(errCode == -3) printf("FAILED reading (I/O error occured)\n");
				}


			}
		} 	/* end: "Attempt to connect the connection ID handle to serial ports between COM0 and "COM16"" */


		// Hopefully the connection should have been established now.
		// Otherwise we stop here.
		if (comPortFound == 1) {
			printf( "connection established");
		} else {
			printf( "connection could not be established");
			wait();
			exit( EXIT_FAILURE ); 
		}

		//To get eyeblinks, you will need to first call the following function:
		TG_EnableBlinkDetection(connectionId, 1 );

		/* Read 10 ThinkGear Packets from the connection, 1 Packet at a time */
		packetsRead = 0;
		while (1){
			//while( packetsRead < 10000 ) {

			/* Attempt to read a Packet of data from the connection */
			errCode = TG_ReadPackets( connectionId, 1 );


			/* If TG_ReadPackets() was able to read a complete Packet of data... */
			if( errCode == 1 ) {
				packetsRead++;
				printf( ".");

				/* If attention value has been updated by TG_ReadPackets()... */
				if( TG_GetValueStatus(connectionId, TG_DATA_ATTENTION) != 0 ) {

					/* Get and print out the updated attention value */
					fprintf( stdout, "New attention value: %f\n",
						TG_GetValue(connectionId, TG_DATA_ATTENTION) );
					fflush( stdout );

				} /* end "If attention value has been updated..." */


				/* If eye blinking value has been updated by TG_ReadPackets()... */
				if( TG_GetValueStatus(connectionId, TG_DATA_BLINK_STRENGTH) != 0 ) {

					/* Get and print out the updated eye blinking value */
					fprintf( stdout, "New eye blinking value: %f\n",
						TG_GetValue(connectionId, TG_DATA_BLINK_STRENGTH) );
					fflush( stdout );

					// Get mouse position: 
					// * http://msdn.microsoft.com/en-us/library/ms648380(v=vs.85).aspx#_win32_Using_the_Keyboard_to_Move_the_Cursor 
					// * http://msdn.microsoft.com/en-us/library/ms648390%28VS.85%29.aspx
					// * http://stackoverflow.com/questions/2396576/how-to-get-the-cursor-position

					GetCursorPos(&cursorPos);
					x = (int) cursorPos.x;
					y = (int) cursorPos.y;
					//x = 10;
					//y = 20;

					// http://stackoverflow.com/questions/4540282/using-mouse-with-sendinput-in-c
					input.type = INPUT_MOUSE;
					input.mi.mouseData=0;
					input.mi.dx = x*(65536.0f/GetSystemMetrics(SM_CXSCREEN));//x being coord in pixels
					input.mi.dy = y*(65536.0f/GetSystemMetrics(SM_CYSCREEN));//y being coord in pixels
					input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
					SendInput(1,&input,sizeof(input));

				} /* end "If eye blinking value has been updated..." */



			} /* end "If a Packet of data was read..." */

			// Posted by Friedemann Wolpert on July 04, 2012 @ 11:45 PM
			// Regarding your 100% CPU Problem
			// http://support.neurosky.com/discussions/mindwave/916-can-the-neurosky-mindwave-be-used-to-emulate-mouse-clicks-for-somebody-using-computers-all-day-long#comment_17102761
			if( errCode == 0 ) {
				printf( "z");
				Sleep(200);   // use #include Windows.h  
			}


		} /* end "Read 10 Packets of data from connection..." */

		/* Clean up */
		TG_FreeConnection( connectionId );

		/* End program */
		wait();
		return( EXIT_SUCCESS );
}