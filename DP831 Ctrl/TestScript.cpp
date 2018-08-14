#include <stdio.h>
#include <iostream>
#include <string>
#include <atlstr.h>
#include <sstream>
#include <iomanip>
#include "DP831.h"
#include "MotorComm.h"
#include "SerialPort.h"

using namespace std;

int main()
{
	DPSrc Src;	//Declare source using the class DPSrc
	Src.Connect(); //Connect to instrument
	Src.Send(":SYST:REM;"); // Send command to switch system from local to remote
	Src.Send("SYST:VERS?;"); //Query SCPI version - output is year 
	Src.Read();

	/*Portname must contain these backslashes, and remember to
	replace the following com port*/
	const char *portName = "\\\\.\\COM3";
	SerialPort *arduino;

	arduino = new SerialPort(portName);
	const char* input = "1";
	if (arduino->isConnected())
		cout << "Connection eablished" << endl;
	else
		cout << "ERROR, check port name";

	Motor Motor1, Motor2;

	char resp;
	cout << "USER DEFINED PARAMS: Supply limits, voltage range, voltage degree scale factor, degree increment, measurement time" << endl;
	cout << "Will the same parameters be used for both mirrors? Y/N" << endl;
	cin >> resp;
	if (resp == 'Y' || resp == 'y') 
	{
		getUserInput(Motor1); //Get user defined params
		Motor2 = Motor1; // Copy params to second motor
	}
	else 
	{
		getUserInput(Motor1); //Get user defined params for motor 1
		getUserInput(Motor2); //Get user defined params for motor 2
	}

	Motor1.strVLim = "8"; //because of ch1 lim
	setStartVal(Src, Motor1, "1", "0"); //Initialize all starting values and limits for motor 1 - note: this does not turn on any of the outputs 
	setStartVal(Src, Motor2, "2", "3"); //Initialize all starting values and limits for motor 2

	int ChConf;
	//Based on user defined range find the case for motor 2 
	//We do this here instead of later as we do not want to turn the channels off during measurement
	if (Motor2.vMin < 0 && Motor2.vMax <= 0)
	{
		ChConf = 1;
		Src.Send(":OUTP CH3,ON;");//Turn on output from CH3 - should be a negative volt min
	}
	else if (Motor2.vMin < 0 && Motor2.vMax > 0)
	{
		ChConf = 2;
		Src.Send(":OUTP CH3,ON;");//Turn on output from CH3 - should be a negative volt min
		Src.Send(":OUTP CH2,ON;"); //Turn on output from CH2 - should be positive volt
	}
	else
	{
		ChConf = 3;
		Src.Send(":OUTP CH2,ON;");//Turn on output from CH2 - should be a positive volt min
	}

	//double totalRunTime; //total time to execute measurement
	//totalRunTime = (Motor1.measTime * Motor1.steps) + (Motor2.measTime * Motor2.steps); //incomplete time calc
	//cout << "Scan will take roughly:" << totalRunTime << "seconds to execute";


	//Loop to handle raster scan
	for (int i = 0; i < Motor1.steps; i++) //Iterate through x-axis positions
	{	
		arduino->writeSerialPort("1", MAX_DATA_LENGTH); //Turns on pin 8 - Line marker
		cout << "X Direction - Step:" << i + 1 << endl; // Just to keep track
		if (i == 0) // Turn on positive supply during initial step
		{
			Src.Send(":OUTP CH1,ON"); //Turn on output from CH1 - should be 0 volts or positive
		}
		else 
		{
			Src.Send(":SOUR1:VOLT UP;"); // Increment for each change in i
		}

		for (int j = 0; j < Motor2.steps+1; j++) //MOTOR 2 LOOP
		{
			cout << setw(27) << "Y Direction - Step:" << j + 1 << endl; //Just to keep track
			//start of line - trigger marker 1 (pin 8) 

			if (j != 0) // increment
			{
				switch (ChConf)
				{
				case 1: // Negative volt to negative or zero volt 
					if (Motor2.vMin + j-1 * Motor2.incr < Motor2.vMax)
					{
						arduino->writeSerialPort("2", MAX_DATA_LENGTH); //trigger pin 9 - frame marker
						Src.Send(":SOUR3:VOLT DOWN;"); //increase voltage out from ch3
					}
					else //step down voltage to initial before next line of scanning starts
					{
						for (int k = 0; k < j - 1; k++)
						{
							Src.Send(":SOUR3:VOLT UP;"); //decrease voltage out from ch3 to start val
						}
					}
					break;
				case 2: // Negative volt to positive volt
					if (Motor2.vMin + j * Motor2.incr <= 0) //while negative voltage being used
					{
						arduino->writeSerialPort("2", MAX_DATA_LENGTH); //trigger pin 9 - frame marker
						Src.Send(":SOUR3:VOLT DOWN;"); //increment ch3
					}
					else if (j == Motor2.stepCross) // at point where voltage transitions from negative to positive (ch3 to ch2)
					{
						arduino->writeSerialPort("2", MAX_DATA_LENGTH); //trigger pin 9 - frame marker
						Src.Send(":INST CH2;");
					}
					else if (j > Motor2.stepCross && j < Motor2.steps)
					{
						arduino->writeSerialPort("2", MAX_DATA_LENGTH); //trigger pin 9 - frame marker
						Src.Send(":SOURC2:VOLT UP;");
					}
					else
					{						
						for (int k = 0; k < j - Motor2.stepCross - 1; k++)
						{
							Src.Send(":SOUR2:VOLT DOWN;"); 
						}

						for (int k = 0; k < Motor2.stepCross - 1; k++)
						{
							Src.Send(":SOUR3:VOLT UP;"); //this actually brings the voltage down...
						}
					}
					break;
				case 3:	// Positive/zero volt to positive volt
					if (Motor2.vMin + j-1 * Motor2.incr < Motor2.vMax)
					{
						arduino->writeSerialPort("2", MAX_DATA_LENGTH); //trigger pin 9 - frame marker
						Src.Send(":SOUR2:VOLT UP;");
					}
					else //step down voltage to initial before next line of scanning starts
					{
						for (int k = 0; k < j - 1; k++)
						{
							Src.Send(":SOUR2:VOLT DOWN;");
						}
					}
					break;
				}
			}
			Sleep(Motor2.measTime); //Wait time until voltage increment for motor 2 - change in y
			//end of frame - trigger marker 2 (pin 9)
		}
		Sleep(Motor1.measTime); // Wait time until voltage increment for motor 1 - change in x 
		arduino->writeSerialPort("3", MAX_DATA_LENGTH); //trigger pin 10 - end of line marker
	}
	Src.Send(":OUTP CH1,OFF; :OUTP CH2,OFF; :OUTP CH3,OFF;"); //Turn off output from all CH 
}



